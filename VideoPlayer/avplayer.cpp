#include "avplayer.h"
#include <QThread>
#include <QDebug>

//同步阈值下限
#define AV_SYNC_THRESHOLD_MIN 0.04
//同步阈值上限
#define AV_SYNC_THRESHOLD_MAX 0.1
//但帧视频时长阈值上限，用于适配低帧时同步，
//帧率过低视频帧超前不适合翻倍延迟，应特殊处理
//处理，这里设置上限一秒10帧率
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
//同步操作摆烂阈值上限，此时同步已无意义
#define AV_NOSYNC_THRESHOLD 10.0

#define AV_SYNC_REJUDGESHOLD 0.01

void fillAStreamCallback(void *userdata, uint8_t *stream, int len){
    memset(stream,0,len);
    AVPlayer* thiz=(AVPlayer*)userdata;
    double audioPts=0.00;
    if(thiz->m_clockInitFlag==-1){
        thiz->initAVClock();
    }
    while(len>0){
        if(thiz->m_exit)return;
        if(thiz->m_audioBufIndex>=thiz->m_audioBufSize){
            int ret=thiz->m_decoder->getAFrame(thiz->m_audioFrame);
            if(ret){
                thiz->m_audioBufIndex=0;
                if((thiz->m_targetSampleFmt!=thiz->m_audioFrame->format||
                    thiz->m_targetFreq!=thiz->m_audioFrame->sample_rate||
                    thiz->m_targetChannelLayout!=thiz->m_audioFrame->channel_layout||
                    thiz->m_targetNbSamples!=thiz->m_audioFrame->nb_samples)&&!thiz->m_swrCtx){
                    thiz->m_swrCtx=swr_alloc_set_opts(nullptr,thiz->m_targetChannelLayout,thiz->m_targetSampleFmt,thiz->m_targetFreq,thiz->m_audioFrame->channel_layout,
                                                      (enum AVSampleFormat)thiz->m_audioFrame->format,thiz->m_audioFrame->sample_rate,0,nullptr);
                    if(!thiz->m_swrCtx||swr_init(thiz->m_swrCtx)<0){
                        qDebug()<<"swr_init_failed";
                        return;
                    }
                }
                if(thiz->m_swrCtx){
                    const uint8_t** in = (const uint8_t**)thiz->m_audioFrame->extended_data;
                    int out_count = (uint64_t)thiz->m_audioFrame->nb_samples * thiz->m_targetFreq / thiz->m_audioFrame->sample_rate + 256;
                    int out_size = av_samples_get_buffer_size(nullptr, thiz->m_targetChannels, out_count, thiz->m_targetSampleFmt, 0);
                    if (out_size < 0) {
                        qDebug() << "av_samples_get_buffer_size failed";
                        return;
                    }
                    av_fast_malloc(&thiz->m_audioBuf, &thiz->m_audioBufSize, out_size);
                    if (!thiz->m_audioBuf) {
                        qDebug() << "av_fast_malloc failed";
                        return;
                    }
                    int len2 = swr_convert(thiz->m_swrCtx, &thiz->m_audioBuf, out_count, in, thiz->m_audioFrame->nb_samples);
                    if (len2 < 0) {
                        qDebug() << "swr_convert failed";
                        return;
                    }
                    thiz->m_audioBufSize = av_samples_get_buffer_size(nullptr, thiz->m_targetChannels, len2, thiz->m_targetSampleFmt, 0);
                }
                else {
                    thiz->m_audioBufSize = av_samples_get_buffer_size(nullptr, thiz->m_targetChannels, thiz->m_audioFrame->nb_samples, thiz->m_targetSampleFmt, 0);
                    av_fast_malloc(&thiz->m_audioBuf, &thiz->m_audioBufSize, thiz->m_audioBufSize+256);
                    if (!thiz->m_audioBuf) {
                        qDebug() << "av_fast_malloc failed";
                        return;
                    }
                    memcpy(thiz->m_audioBuf, thiz->m_audioFrame->data[0], thiz->m_audioBufSize);
                }
                audioPts = thiz->m_audioFrame->pts * av_q2d(thiz->m_fmtCtx->streams[thiz->m_audioIndex]->time_base);
                //qDebug()<<thiz->m_audioPts;
                av_frame_unref(thiz->m_audioFrame);
            }
            else{
                if(thiz->m_decoder->isExit()){
                    emit thiz->AVTerminate();
                }
                return;
            }
        }
        int len1=thiz->m_audioBufSize-thiz->m_audioBufIndex;
        len1=(len1>len?len:len1);
        SDL_MixAudio(stream,thiz->m_audioBuf+thiz->m_audioBufIndex,len1,thiz->m_volume);
        len-=len1;
        thiz->m_audioBufIndex+=len1;
        stream+=len1;
    }
    thiz->m_audioClock.setClock(audioPts);
    uint32_t _pts=(uint32_t)audioPts;
    if(thiz->m_lastAudPts!=_pts){
        emit thiz->AVPtsChanged(_pts);
        thiz->m_lastAudPts=_pts;
    }
}

AVPlayer::AVPlayer():
    m_decoder(new Decoder()),
    m_targetChannels(0),m_targetFreq(0),
    m_targetChannelLayout(0),m_targetNbSamples(0),
    m_volume(30),m_clockInitFlag(-1),m_audioIndex(-1),
    m_videoIndex(-1),m_imageWidth(300),m_imageHeight(300),
    m_audioFrame(av_frame_alloc()),m_pause(0),m_exit(0),m_audioBuf(nullptr),m_videobuffer(nullptr),m_swrCtx(nullptr),m_swsCtx(nullptr){}

int AVPlayer::Play(const QString &url){
    clearPlayer();
    if(!m_decoder->decode(url)){
        qDebug()<<"decode failed";
        return 0;
    }
    //解码成功获取流时长
    m_duration=m_decoder->avDuration();
    emit AVDurationChanged(m_duration);
    m_pause=0;
    m_clockInitFlag=-1;
    if(!initSDL()){
        qDebug()<<"init sdl failed";
        return  0;
    }
    if(!initVideo()){
        qDebug()<<"init video failed";
        return 0;
    }
    return 1;
}

void AVPlayer::pause(bool isPause){
    if(SDL_GetAudioStatus()==SDL_AUDIO_STOPPED)
        return;
    if(isPause){
        if(SDL_GetAudioStatus()==SDL_AUDIO_PLAYING){
            SDL_PauseAudio(1);
            m_pauseTime=av_gettime_relative()/1000000.0;
            m_pause=1;
        }
    }else{
        if(SDL_GetAudioStatus()==SDL_AUDIO_PAUSED) {
            SDL_PauseAudio(0);
            m_frameTimer+=av_gettime_relative()/1000000.0-m_pauseTime;
            m_pause=0;
        }
    }
}

void AVPlayer::clearPlayer(){
    if(playState()!=AV_STOPPED){
        m_exit=1;
        if(playState()==AV_PLAYING)
            SDL_PauseAudio(1);
        m_decoder->exit();
        SDL_CloseAudio();
        if(m_swrCtx)swr_free(&m_swrCtx);
        if(m_swsCtx)sws_freeContext(m_swsCtx);
        m_swrCtx=nullptr;
        m_swsCtx=nullptr;
    }
}

AVPlayer::PlayState AVPlayer::playState(){
    AVPlayer::PlayState state;
    switch (SDL_GetAudioStatus()) {
    case SDL_AUDIO_PLAYING:
        state=AVPlayer::AV_PLAYING;
        break;
    case SDL_AUDIO_PAUSED:
        state=AVPlayer::AV_PAUSED;
        break;
    case SDL_AUDIO_STOPPED:
        state=AVPlayer::AV_STOPPED;
        break;
    default:
        break;
    }
    return state;
}

int AVPlayer::initSDL(){
    if(SDL_Init(SDL_INIT_AUDIO)!=0){
        qDebug()<<"SDL_INIT failed";
        return 0;
    }
    m_exit=0;
    m_audioBufSize=0;
    m_audioBufIndex=0;
    m_lastAudPts=-1;
    m_audioCodecPar=m_decoder->audioCodecPar();
    //用于描述音频流规格的结构体
    //freq：采样率 format:音频格式 channels: 声道数
    //samples:每次读写的样本数
    //callback:回调函数 用于提供音频数据
    SDL_AudioSpec cur_spec;
    cur_spec.channels=m_audioCodecPar->channels;
    cur_spec.freq=m_audioCodecPar->sample_rate;
    cur_spec.silence=0;
    cur_spec.callback=fillAStreamCallback;
    cur_spec.userdata=this;
    cur_spec.samples=m_audioCodecPar->frame_size;
    if(SDL_OpenAudio(&cur_spec,nullptr)<0){
        qDebug()<<"SDL_OpenAudio failed";
        return 0;
    }
    m_audioIndex=m_decoder->audIndex();
    m_targetSampleFmt=AV_SAMPLE_FMT_S16;
    m_targetChannels=m_audioCodecPar->channels;
    m_targetFreq=m_audioCodecPar->sample_rate;
    m_targetChannelLayout=av_get_default_channel_layout(m_targetChannels);
    m_targetNbSamples=m_audioCodecPar->frame_size;
    m_fmtCtx=m_decoder->formatContext();
    SDL_PauseAudio(0);//0表示开始播放音频
    return 1;
}

int AVPlayer::initVideo(){
    m_frameTimer=0.00;
    m_videoCodecPar=m_decoder->videoCodecPar();
    m_videoIndex=m_decoder->vidIndex();
    m_imageWidth=m_videoCodecPar->width;
    m_imageHeight=m_videoCodecPar->height;
    m_dstPixFmt=AV_PIX_FMT_YUV422P;
    m_swsFlags=SWS_BICUBIC;//表示采用双三次插值方法进行图像缩放
    //分配存储转换后帧数据的buffer内存
    int bufsize=av_image_get_buffer_size(m_dstPixFmt,m_imageWidth,m_imageHeight,1);
    m_videobuffer=(uint8_t*)av_realloc(m_videobuffer,bufsize*sizeof(uint8_t));
    av_image_fill_arrays(m_pixels,m_pitch,m_videobuffer,m_dstPixFmt,
                         m_imageWidth,m_imageHeight,1);
    //视频帧播放回调递插入线程池任务队列
    if(!ThreadPool::addTask(std::bind(&AVPlayer::videoCallBack,this,std::placeholders::_1),std::make_shared<int>(4))){
        qDebug()<<"videocallback add task failed";
        return 0;
    }
    return 1;
}

void AVPlayer::videoCallBack(std::shared_ptr<void> par){
    double time=0.00;
    double duration=0.00;
    double delay=0.00;
    if(m_clockInitFlag==-1){
        initAVClock();
    }
    do{
        if(m_exit)break;
        if(m_pause){
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        if(m_decoder->getRemainingVFrame()){
            MyFrame* lastFrame=m_decoder->peekLastVFrame();
            MyFrame* curFrame=m_decoder->peekVFrame();
            if(curFrame->serial!=m_decoder->vidPketSerial()){
                m_decoder->setNextVFrame();
                continue;
            }
            if(curFrame->serial!=lastFrame->serial){
                m_frameTimer=av_gettime_relative()/1000000.0;
            }
            duration=vpDuration(lastFrame,curFrame);
            delay=computeTargetDelay(duration);
            time=av_gettime_relative()/1000000.0;
            //显示时长未到达
            if(time<m_frameTimer+delay){
                QThread::msleep((uint32_t)(FFMIN(AV_SYNC_REJUDGESHOLD,m_frameTimer+delay-time)*1000));
                continue;
            }
            m_frameTimer+=delay;
            if(time-m_frameTimer>AV_SYNC_THRESHOLD_MAX)
                m_frameTimer=time;
            //队列中未显示帧一帧以上执行逻辑丢帧判断
            if(m_decoder->getRemainingVFrame()>1){
                MyFrame* nextFrame=m_decoder->peekNextVFrame();
                duration=nextFrame->pts-curFrame->pts;
                //若主时钟超前到大于当前帧理论显示应持续的时间了，则当前帧立即丢弃
                if(time>m_frameTimer+duration){
                    m_decoder->setNextVFrame();
                    qDebug()<<"abandon vframe";
                    continue;
                }
            }
            displayImage(&curFrame->frame);
            m_decoder->setNextVFrame();
        }else{
            QThread::msleep(10);
        }
    }while(true);
    qDebug()<<"videoCallBack exit"<<endl;
}

void AVPlayer::initAVClock(){
    if(m_clockInitFlag==-1){
        m_audioClock.setClock(0.0);
        m_videoClock.setClock(0.0);
        m_clockInitFlag=1;
    }
}

void AVPlayer::displayImage(AVFrame *frame){
    if (frame) {
        //判断若是否需要格式转换
        if ((m_videoCodecPar->width != m_imageWidth ||
              m_videoCodecPar->height != m_imageHeight ||
              m_videoCodecPar->format != m_dstPixFmt)&&!m_swsCtx) {
            m_swsCtx = sws_getCachedContext(m_swsCtx,frame->width,frame->height,
                (enum AVPixelFormat)frame->format, m_imageWidth,m_imageHeight,m_dstPixFmt,
                                                              m_swsFlags, nullptr, nullptr, nullptr);
        }
        if (m_swsCtx) {
            sws_scale(m_swsCtx, frame->data, frame->linesize, 0,
                                frame->height, m_pixels, m_pitch);
            emit frameChanged(QSharedPointer<YUV422Frame>::create(m_pixels[0],m_imageWidth,m_imageHeight));
        }
        else {
            emit frameChanged(QSharedPointer<YUV422Frame>::create((uint8_t*)frame->data[0],m_imageWidth,m_imageHeight));
        }

        //记录视频时钟
        m_videoClock.setClock(frame->pts * av_q2d(m_fmtCtx->streams[m_videoIndex]->time_base));
    }
}

double AVPlayer::vpDuration(MyFrame *lastFrame, MyFrame *frame){
    if(lastFrame->serial==frame->serial){
        double duration=frame->pts-lastFrame->pts;
        if(isnan(duration)||duration>AV_NOSYNC_THRESHOLD)
            return lastFrame->duration;
        else
            return duration;
    }
    return 0.00;
}

double AVPlayer::computeTargetDelay(double delay){
    //视频当前显示帧与当前播放音频帧时间戳差值
    double diff=m_videoClock.getClock()-m_audioClock.getClock();
    //计算同步阈值
    double sync=FFMAX(AV_SYNC_THRESHOLD_MIN,FFMIN(AV_SYNC_THRESHOLD_MAX,delay));

    //不同步时间超过阈值直接放弃同步
    if(!isnan(diff)&&fabs(diff)<AV_NOSYNC_THRESHOLD){
        if(diff<=-sync)delay=FFMAX(0,diff+delay);
        else if(diff>=sync&&delay>AV_SYNC_FRAMEDUP_THRESHOLD)
            delay=diff+delay;
        else if(diff>=sync)
            delay=delay*2;
    }
    return  delay;
}

AVPlayer::~AVPlayer(){
    av_frame_free(&m_audioFrame);
    clearPlayer();
    delete  m_decoder;
    if(m_swrCtx)
        swr_free(&m_swrCtx);
    if(m_swsCtx)
        sws_freeContext(m_swsCtx);
    if(m_audioBuf)
        av_free(m_audioBuf);
    if(m_videobuffer)
        av_free(m_videobuffer);
}

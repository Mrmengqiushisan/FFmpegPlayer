#include "decoder.h"
#include<QImage>
#include<QVector>
#include<mutex>
#include<condition_variable>
#include<QThread>
#include"c11threadpool.h"

using AVTool::Decoder;


Decoder::Decoder():
    m_fmtCtx(nullptr),
    m_maxFrameQueueSize(16),
    m_maxPacketQueueSize(30),
    m_audioIndex(-1),
    m_videoIndex(-1),
    m_exit(0),
    m_duration(0){
    if(!init()){
        qDebug()<<"init failed";
    }
}

Decoder::~Decoder(){
    exit();
}

AVTool::MediaInfo *Decoder::detectMediaInfo(const QString &url)
{
    int ret{0};
    int duration{0};
    //初始化FFmpeg上下文
    AVFormatContext* fmtCtx=avformat_alloc_context();
    if(!fmtCtx)return  nullptr;
    //创建字典对象 这可以添加键值对内容 去告诉fmtctx在打开音视频时应该需要做那些事情
    AVDictionary* formatOpts=nullptr;
    //这个键值对告诉fmtctx的探测大小为32字节
    av_dict_set(&formatOpts,"probesize","32",0);
    //打开url 可以解析得到一些内容
    ret=avformat_open_input(&fmtCtx,url.toUtf8().constData(),nullptr,nullptr);
    if(ret<0){
        av_strerror(ret,m_errBuf,ERRBUF_SIZE);
        qDebug() << "avformat_open_input error:" << m_errBuf;
        avformat_free_context(fmtCtx);
        av_dict_free(&formatOpts);
        return  nullptr;
    }
    //获取流对象
    ret=avformat_find_stream_info(fmtCtx,nullptr);
    if(ret<0){
        av_strerror(ret,m_errBuf,ERRBUF_SIZE);
        qDebug() << "avformat_find_stream_info error:" << m_errBuf;
        avformat_free_context(fmtCtx);
        av_dict_free(&formatOpts);
        return  nullptr;
    }
    //记录流时长
    AVRational q{1,AV_TIME_BASE};
    duration=(uint32_t)(fmtCtx->duration*av_q2d(q));
    av_dict_free(&formatOpts);
    int videoIndex=av_find_best_stream(fmtCtx,AVMEDIA_TYPE_VIDEO,-1,-1,nullptr,0);
    if (videoIndex < 0) {
        qDebug() << "no video stream!";
        return Q_NULLPTR;
    }
    //视频解码初始化
    //获取编解码器参数
    AVCodecParameters* videoCodecPar=fmtCtx->streams[videoIndex]->codecpar;
    if (!videoCodecPar) {
        qDebug() << "videocodecpar is nullptr!";
        return Q_NULLPTR;
    }
    //构造编解码器上下文
    AVCodecContext* codecCtx=avcodec_alloc_context3(nullptr);
    //将参数设置到编解码器中
    ret=avcodec_parameters_to_context(codecCtx,videoCodecPar);
    if (ret < 0) {
        av_strerror(ret, m_errBuf, sizeof(m_errBuf));
        qDebug() << "error info_avcodec_parameters_to_context:" << m_errBuf;
        return Q_NULLPTR;
    }
    //找到解码器
    const AVCodec* videoCodec=avcodec_find_decoder(codecCtx->codec_id);
    if (!videoCodec) {
        qDebug() << "avcodec_find_decoder failed!";
        return Q_NULLPTR;
    }
    codecCtx->codec_id=videoCodec->id;
    //根据编解码器上下文打开解码器
    ret=avcodec_open2(codecCtx,videoCodec,nullptr);
    if (ret < 0) {
        av_strerror(ret, m_errBuf, sizeof(m_errBuf));
        qDebug() << "error info_avcodec_open2:" << m_errBuf;
        return Q_NULLPTR;
    }
    //开始解码
    AVPacket* pkt=av_packet_alloc();
    AVFrame* frame=av_frame_alloc();
    bool falg=false;
    while(1){
        //首先获取包数据
        ret=av_read_frame(fmtCtx,pkt);
        if(ret!=0){
            return Q_NULLPTR;
        }
        if(pkt->stream_index==videoIndex){
            //将压缩包数据发送给解码器
            ret=avcodec_send_packet(codecCtx,pkt);
            av_packet_unref(pkt);
            if(ret<0||ret==AVERROR(EAGAIN)||ret==AVERROR_EOF){
                av_strerror(ret, m_errBuf, sizeof(m_errBuf));
                qDebug() << "avcodec_send_packet error:" << m_errBuf;
                continue;
            }
            while(1){
                ret=avcodec_receive_frame(codecCtx,frame);
                if(ret==0){
                    falg=true;
                    break;
                }else if(ret==AVERROR(EAGAIN)){
                    break;
                }else{
                    return  Q_NULLPTR;
                }
            }
            if(falg)break;
        }else{
            av_packet_unref(pkt);
        }
    }
    //走到这说明已经拿到了frame
    int imageWidth=videoCodecPar->width;
    int imageHeight=videoCodecPar->height;
    enum  AVPixelFormat dstPixFmt=AV_PIX_FMT_RGB24;
    int swsFlags=SWS_BICUBIC;
    uint8_t* pixels[4];
    int pitch[4];
    //分配存储转换后帧数据的buffer内存
    int bufsize=av_image_get_buffer_size(dstPixFmt,imageWidth,imageHeight,1);
    uint8_t* buffer=(uint8_t*)av_malloc(bufsize*sizeof(uint8_t));
    av_image_fill_arrays(pixels,pitch,buffer,dstPixFmt,imageWidth,imageHeight,1);
    //用于设置图像大小和像素转换的上下文
    SwsContext* swsCtx=sws_getCachedContext(nullptr,frame->width,frame->height,(enum AVPixelFormat)frame->format,
                                            imageWidth,imageHeight,dstPixFmt,swsFlags,nullptr,nullptr,nullptr);
    if(swsCtx){
        //该函数用于做实际的转换
        sws_scale(swsCtx,frame->data,frame->linesize,0,frame->height,pixels,pitch);
    }
    av_frame_unref(frame);
    AVTool::MediaInfo* info=new AVTool::MediaInfo();
    info->duration=duration;
    info->tipImg=QImage(pixels[0],imageWidth,imageHeight,pitch[0],QImage::Format_RGB888);
    av_packet_free(&pkt);
    av_frame_free(&frame);
    avformat_close_input(&fmtCtx);
    avcodec_free_context(&codecCtx);
    return info;
}

void Decoder::seekTo(int32_t target, int32_t seekRel){
    if(m_isSeek==1)return;
    if(target<0)target=0;
    m_seekTarget=target;
    m_seekRel=seekRel;
    m_isSeek=1;
}

int Decoder::getAFrame(AVFrame *frame){
    //qDebug()<<"get audio frame";
    std::unique_lock<std::mutex> curlock(m_audioFrameQueue.mutex);
    while(!m_audioFrameQueue.size){
        bool ret=m_audioFrameQueue.cond.wait_for(curlock,std::chrono::milliseconds(50),[&](){
            return m_audioFrameQueue.size&!m_exit;
        });
        if(!ret)return 0;
    }
    if(m_audioFrameQueue.frameVec[m_audioFrameQueue.readIndex].serial!=m_audioPacketQueue.serial){
        av_frame_unref(&m_audioFrameQueue.frameVec[m_audioFrameQueue.readIndex].frame);
        m_audioFrameQueue.readIndex=(m_audioFrameQueue.readIndex+1)%m_maxFrameQueueSize;
        m_audioFrameQueue.size--;
    }
    av_frame_move_ref(frame,&m_audioFrameQueue.frameVec[m_audioFrameQueue.readIndex].frame);
    m_audioFrameQueue.readIndex=(m_audioFrameQueue.readIndex+1)%m_maxFrameQueueSize;
    m_audioFrameQueue.size--;
    return 1;
}

int Decoder::getRemainingVFrame(){
    if(!m_videoFrameQueue.size)return 0;
    return m_videoFrameQueue.size-m_videoFrameQueue.shown;
}

Decoder::MYFRAME *Decoder::peekLastVFrame(){
    MYFRAME* frame=&m_videoFrameQueue.frameVec[m_videoFrameQueue.readIndex];
    return frame;
}

Decoder::MYFRAME *Decoder::peekVFrame(){
    while(!m_videoFrameQueue.size){
        std::unique_lock<std::mutex> lock(m_videoFrameQueue.mutex);
        bool ret=m_videoFrameQueue.cond.wait_for(lock,std::chrono::milliseconds(100),[&](){
            return (!m_exit)&m_videoFrameQueue.size;
        });
        if(!ret)return nullptr;
    }
    int index=(m_videoFrameQueue.readIndex+m_videoFrameQueue.shown)%m_maxFrameQueueSize;
    MYFRAME* frame=&m_videoFrameQueue.frameVec[index];
    return frame;
}

Decoder::MYFRAME *Decoder::peekNextVFrame(){
    while(m_videoFrameQueue.size<2){
        std::unique_lock<std::mutex> lock(m_videoFrameQueue.mutex);
        bool ret=m_videoFrameQueue.cond.wait_for(lock,std::chrono::milliseconds(100),[&](){
            return (!m_exit)&(m_videoFrameQueue.size>=2);
        });
        if(!ret)return nullptr;
    }
    int index=(m_videoFrameQueue.readIndex+m_videoFrameQueue.shown+1)%m_maxFrameQueueSize;
    MYFRAME* frame=&m_videoFrameQueue.frameVec[index];
    return frame;
}

void Decoder::setNextVFrame(){
    std::unique_lock<std::mutex> lock(m_videoFrameQueue.mutex);
    if(!m_videoFrameQueue.size)return;
    if(!m_videoFrameQueue.shown){
        m_videoFrameQueue.shown=1;
        return;
    }
    av_frame_unref(&m_videoFrameQueue.frameVec[m_videoFrameQueue.readIndex].frame);
    m_videoFrameQueue.readIndex=(m_videoFrameQueue.readIndex+1)%m_maxFrameQueueSize;
    m_videoFrameQueue.size--;
}

int Decoder::decode(const QString &url){
    int ret=0;
    m_fmtCtx=avformat_alloc_context();//解封装初始化
    AVDictionary* formatOpts=nullptr;
    av_dict_set(&formatOpts,"probesize","32",0);
    ret=avformat_open_input(&m_fmtCtx,url.toUtf8().constData(),nullptr,&formatOpts);
    if(ret<0){
        av_strerror(ret,m_errBuf,ERRBUF_SIZE);
        qDebug()<<"avformat_open_input :"<<m_errBuf;
        av_dict_free(&formatOpts);
        avformat_free_context(m_fmtCtx);
        return 0;
    }
    //获取流信息
   ret=avformat_find_stream_info(m_fmtCtx,nullptr);
   if(ret<0){
       av_strerror(ret,m_errBuf,ERRBUF_SIZE);
       qDebug()<<"avformat_find_stream_info:"<<m_errBuf;
       avformat_free_context(m_fmtCtx);
       av_dict_free(&formatOpts);
       return 0;
   }
   //记录流时长
   AVRational q{1,AV_TIME_BASE};
   m_duration=(uint32_t)(m_fmtCtx->duration*av_q2d(q));
   av_dict_free(&formatOpts);
   //获取音视频index
   m_audioIndex=av_find_best_stream(m_fmtCtx,AVMEDIA_TYPE_AUDIO,-1,-1,nullptr,0);
   if(m_audioIndex<0){
       qDebug()<<"no audio stream";
       return 0;
   }
   m_videoIndex=av_find_best_stream(m_fmtCtx,AVMEDIA_TYPE_VIDEO,-1,-1,nullptr,0);
   if(m_videoIndex<0){
       qDebug()<<"no video stream";
       return 0;
   }
   //音频编解码器content初始化
   AVCodecParameters* audioPar= m_fmtCtx->streams[m_audioIndex]->codecpar;
   if(!audioPar){
       qDebug()<<"audio par is nullptr";
       avformat_free_context(m_fmtCtx);
       return 0;
   }
   m_audioPktDecoder.codecCtx=avcodec_alloc_context3(nullptr);
   ret=avcodec_parameters_to_context(m_audioPktDecoder.codecCtx,audioPar);
   if(ret<0){
       av_strerror(ret,m_errBuf,ERRBUF_SIZE);
       qDebug()<<"avcodec_parameters_to_content:"<<m_errBuf;
       return 0;
   }
   const AVCodec* audioCode=avcodec_find_decoder(m_audioPktDecoder.codecCtx->codec_id);
   if(!audioCode){
       qDebug()<<"audiocode is nullptr";
       return 0;
   }
   m_audioPktDecoder.codecCtx->codec_id=audioCode->id;
   ret=avcodec_open2(m_audioPktDecoder.codecCtx,audioCode,nullptr);
   if(ret<0){
       av_strerror(ret,m_errBuf,ERRBUF_SIZE);
       qDebug()<<"avcodec_open2:"<<m_errBuf;
       return 0;
   }
   //视频编解码器content初始化
   AVCodecParameters* videoPar=m_fmtCtx->streams[m_videoIndex]->codecpar;

   m_videoPktDecoder.codecCtx=avcodec_alloc_context3(nullptr);
   ret=avcodec_parameters_to_context(m_videoPktDecoder.codecCtx,videoPar);
   if(ret<0){
       av_strerror(ret,m_errBuf,ERRBUF_SIZE);
       qDebug()<<"avcodec_parameters_to_content:"<<m_errBuf;
       return 0;
   }
   const AVCodec* videoCodec=avcodec_find_decoder(m_videoPktDecoder.codecCtx->codec_id);
   if(!videoCodec){
       qDebug()<<"videoCodec is nullptr";
       return 0;
   }
   m_videoPktDecoder.codecCtx->codec_id=videoCodec->id;
   ret=avcodec_open2(m_videoPktDecoder.codecCtx,videoCodec,nullptr);
   if(ret<0){
       av_strerror(ret,m_errBuf,ERRBUF_SIZE);
       qDebug()<<"avcodec_open2:"<<m_errBuf;
       return 0;
   }
   //记录视频帧率
   m_vidFrameRate=av_guess_frame_rate(m_fmtCtx,m_fmtCtx->streams[m_videoIndex],nullptr);
   setInitval();
   ThreadPool::addTask(std::bind(&Decoder::demux,this,std::placeholders::_1),std::make_shared<int>(1));
   ThreadPool::addTask(std::bind(&Decoder::audioDecode,this,std::placeholders::_1),std::make_shared<int>(2));
   ThreadPool::addTask(std::bind(&Decoder::videoDecode,this,std::placeholders::_1),std::make_shared<int>(3));
   return 1;
}

void Decoder::exit(){
    m_exit=1;
    QThread::msleep(200);
    clearQueueCache();
    if(m_fmtCtx){
        avformat_close_input(&m_fmtCtx);
        m_fmtCtx=nullptr;
    }
    if(m_audioPktDecoder.codecCtx){
        avcodec_free_context(&m_audioPktDecoder.codecCtx);
        m_audioPktDecoder.codecCtx=nullptr;
    }
    if(m_videoPktDecoder.codecCtx){
        avcodec_free_context(&m_videoPktDecoder.codecCtx);
        m_videoPktDecoder.codecCtx=nullptr;
    }
}

bool Decoder::init(){
    if(!ThreadPool::init()){
        qDebug()<<"threadpool init failed";
        return false;
    }
    m_audioPacketQueue.pktVec.resize(m_maxPacketQueueSize);
    m_videoPacketQueue.pktVec.resize(m_maxPacketQueueSize);
    m_audioFrameQueue.frameVec.resize(m_maxFrameQueueSize);
    m_videoFrameQueue.frameVec.resize(m_maxFrameQueueSize);
    m_audioPktDecoder.codecCtx=nullptr;
    m_videoPktDecoder.codecCtx=nullptr;
    return true;
}

void Decoder::setInitval(){
    m_audioPacketQueue.size=0;
    m_audioPacketQueue.serial=0;
    m_audioPacketQueue.pushIndex=0;
    m_audioPacketQueue.readIndex=0;

    m_videoPacketQueue.size=0;
    m_videoPacketQueue.serial=0;
    m_videoPacketQueue.pushIndex=0;
    m_videoPacketQueue.readIndex=0;

    m_audioFrameQueue.size=0;
    m_audioFrameQueue.shown=0;
    m_audioFrameQueue.pushIndex=0;
    m_audioFrameQueue.readIndex=0;

    m_videoFrameQueue.size=0;
    m_videoFrameQueue.shown=0;
    m_videoFrameQueue.pushIndex=0;
    m_videoFrameQueue.readIndex=0;

    m_exit=0;

    m_isSeek=0;

    m_vidSeek=0;
    m_audSeek=0;

    m_seekRel=0;
    m_seekTarget=0;

    memset(m_errBuf,0,ERRBUF_SIZE);

    m_audioPktDecoder.m_serial=0;
    m_videoPktDecoder.m_serial=0;
}

void Decoder::packetQueueFlush(Decoder::PacketQueue *queue){
    qDebug()<<"flush audio(video) queue";
    std::unique_lock<std::mutex> curlock(queue->mutex);
    while(queue->size){
        av_packet_unref(&queue->pktVec[queue->readIndex].pkt);
        queue->readIndex=(queue->readIndex+1)%m_maxPacketQueueSize;
        queue->size--;
    }
    queue->serial++;
}
void Decoder::clearQueueCache(){
    std::lock_guard<std::mutex> lockAP(m_audioPacketQueue.mutex);
    std::lock_guard<std::mutex> lockVP(m_videoPacketQueue.mutex);
    while(m_audioPacketQueue.size){
        av_packet_unref(&m_audioPacketQueue.pktVec[m_audioPacketQueue.readIndex].pkt);
        m_audioPacketQueue.readIndex=(m_audioPacketQueue.readIndex+1)%m_maxPacketQueueSize;
        m_audioPacketQueue.size--;
    }
    while(m_videoPacketQueue.size){
        av_packet_unref(&m_videoPacketQueue.pktVec[m_videoPacketQueue.readIndex].pkt);
        m_videoPacketQueue.readIndex=(m_videoPacketQueue.readIndex+1)%m_maxPacketQueueSize;
        m_videoPacketQueue.size--;
    }
    std::lock_guard<std::mutex> lockAF(m_audioFrameQueue.mutex);
    std::lock_guard<std::mutex> lockVF(m_videoFrameQueue.mutex);
    while(m_audioFrameQueue.size){
        av_frame_unref(&m_audioFrameQueue.frameVec[m_audioFrameQueue.readIndex].frame);
        m_audioFrameQueue.readIndex=(m_audioFrameQueue.readIndex+1)%m_maxFrameQueueSize;
        m_audioFrameQueue.size--;
    }
    while(m_videoFrameQueue.size){
        av_frame_unref(&m_videoFrameQueue.frameVec[m_videoFrameQueue.readIndex].frame);
        m_videoFrameQueue.readIndex=(m_videoFrameQueue.readIndex+1)%m_maxFrameQueueSize;
        m_videoFrameQueue.size--;
    }
}
void Decoder::demux(std::shared_ptr<void> par){
    Q_UNUSED(par)
    int ret{-1};
    AVPacket* packet=av_packet_alloc();
    while(true){
        if(m_exit)break;
        if(m_audioPacketQueue.size>=m_maxPacketQueueSize||m_videoPacketQueue.size>=m_maxPacketQueueSize){
            //qDebug()<<"Packet Queue is Fulled";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        if(m_isSeek){
            int64_t seekTarget=m_seekTarget*AV_TIME_BASE;//将秒转为微妙
            //AVSEEK_FLAG_BACKWARD是指如果找不到该时间戳下对应的帧 就直接选择离该时间戳最近的帧
            ret=av_seek_frame(m_fmtCtx,-1,seekTarget,AVSEEK_FLAG_BACKWARD);
            if(ret<0){
                //av_packet_free(&packet);
                av_strerror(ret,m_errBuf,ERRBUF_SIZE);
                qDebug()<<"av_seek_frame: "<<m_errBuf;
                //break;
            }else{
                packetQueueFlush(&m_audioPacketQueue);
                packetQueueFlush(&m_videoPacketQueue);
                m_audSeek=1;
                m_vidSeek=1;
            }
            m_isSeek=0;
        }
        ret=av_read_frame(m_fmtCtx,packet);
        if(ret!=0){
            av_packet_free(&packet);
            av_strerror(ret,m_errBuf,ERRBUF_SIZE);
            qDebug()<<"av_read_frame: "<<m_errBuf;
            break;
        }
        if(packet->stream_index==m_audioIndex){
            pushPacket(&m_audioPacketQueue,packet);
        }
        else if(packet->stream_index==m_videoIndex){
            pushPacket(&m_videoPacketQueue,packet);
        }else{
            av_packet_unref(packet);
        }
    }
    av_packet_free(&packet);
    if(!m_exit){
        while(m_audioFrameQueue.size){
            QThread::msleep(50);
        }
        exit();
    }
    qDebug()<<"demux thread exit";
}

void Decoder::audioDecode(std::shared_ptr<void> par){
    Q_UNUSED(par)
    qDebug()<<"audio decode";
    int ret{-1};
    AVPacket* packet=av_packet_alloc();
    AVFrame* frame=av_frame_alloc();
    while(1){
        if(m_exit)break;
        if(m_audioFrameQueue.size>=m_maxFrameQueueSize){
            //qDebug()<<"audio frame size is full";
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        ret=getPacket(&m_audioPacketQueue,packet,&m_audioPktDecoder);
        if(ret){
            ret=avcodec_send_packet(m_audioPktDecoder.codecCtx,packet);
            av_packet_unref(packet);
            if(ret<0){
                av_strerror(ret,m_errBuf,ERRBUF_SIZE);
                qDebug()<<"avcodec_send_packet: "<<m_errBuf;
                continue;
            }
            while(1){
                ret=avcodec_receive_frame(m_audioPktDecoder.codecCtx,frame);
                if(ret!=0)break;
                if(m_audSeek){
                    int pts=(int)frame->pts*av_q2d(m_fmtCtx->streams[m_audioIndex]->time_base);
                    if(pts<m_seekTarget){
                        av_frame_unref(frame);
                        continue;
                    }else m_audSeek=0;
                }
                pushAFrame(frame);
            }
        }else{
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
    av_packet_free(&packet);
    av_frame_free(&frame);
    qDebug()<<"audioDecode exit";
}

void Decoder::videoDecode(std::shared_ptr<void> par){
    Q_UNUSED(par)
    qDebug()<<"video decode";
    int ret=0;
    AVPacket* packet=av_packet_alloc();
    AVFrame* frame=av_frame_alloc();
    while(1){
        if(m_exit)break;
        if(m_videoFrameQueue.size>=m_maxFrameQueueSize){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        ret=getPacket(&m_videoPacketQueue,packet,&m_videoPktDecoder);
        if(ret){
            ret=avcodec_send_packet(m_videoPktDecoder.codecCtx,packet);
            av_packet_unref(packet);
            if(ret<0||ret==AVERROR(EAGAIN)||ret==AVERROR_EOF){
                av_strerror(ret,m_errBuf,ERRBUF_SIZE);
                qDebug()<<"avcodec_send_packet: "<<m_errBuf;
                continue;
            }
            while(1){
                ret=avcodec_receive_frame(m_videoPktDecoder.codecCtx,frame);
                if(ret!=0){
                    //qDebug()<<"video decode failed";
                    break;
                }
                if(m_vidSeek){
                    int pts=(int)frame->pts*av_q2d(m_fmtCtx->streams[m_videoIndex]->time_base);
                    if(pts<m_seekTarget){
                        av_frame_unref(frame);
                        continue;
                    }else m_vidSeek=0;
                }
                pushVFrame(frame);
            }
        }else{
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
    av_packet_free(&packet);
    av_frame_free(&frame);
    qDebug()<<"video decode exit";
}

int Decoder::getPacket(Decoder::PacketQueue *queue, AVPacket *pkt, Decoder::PKtDecoder *decoder){
    //qDebug()<<"get packet audio(video)";
    std::unique_lock<std::mutex> curlock(queue->mutex);
    while(!queue->size){
        bool ret=queue->cond.wait_for(curlock,std::chrono::milliseconds(20),[&](){
            return queue->size&!m_exit;
        });
        if(!ret)return 0;
    }
    if(queue->serial!=decoder->m_serial){
        //序列不相等说明发生跳转直接丢弃
        //并清空解码器缓存
        avcodec_flush_buffers(decoder->codecCtx);
        decoder->m_serial=queue->pktVec[queue->readIndex].serial;
        return 0;
    }
    av_packet_move_ref(pkt,&queue->pktVec[queue->readIndex].pkt);
    decoder->m_serial=queue->pktVec[queue->readIndex].serial;
    queue->readIndex=(queue->readIndex+1)%m_maxPacketQueueSize;
    queue->size--;
    return 1;
}

void Decoder::pushPacket(Decoder::PacketQueue *queue, AVPacket *pkt){
    //qDebug()<<"push audio(video) packet";
    std::unique_lock<std::mutex> curlock(queue->mutex);
    while(queue->size>=m_maxPacketQueueSize){
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    av_packet_move_ref(&queue->pktVec[queue->pushIndex].pkt,pkt);
    queue->pktVec[queue->pushIndex].serial=queue->serial;
    if(queue->serial!=0){
        //qDebug()<<"sreial "<<queue->serial;
    }
    queue->pushIndex=(queue->pushIndex+1)%m_maxPacketQueueSize;
    queue->size++;
}

void Decoder::pushAFrame(AVFrame *frame){
    //qDebug()<<"push audio frame";
    std::unique_lock<std::mutex> curlock(m_audioFrameQueue.mutex);
    while(m_audioFrameQueue.size>=m_maxFrameQueueSize){
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        continue;
    }
    av_frame_move_ref(&m_audioFrameQueue.frameVec[m_audioFrameQueue.pushIndex].frame,frame);
    m_audioFrameQueue.frameVec[m_audioFrameQueue.pushIndex].serial=m_audioPktDecoder.m_serial;
    m_audioFrameQueue.pushIndex=(m_audioFrameQueue.pushIndex+1)%m_maxFrameQueueSize;
    m_audioFrameQueue.size++;
}

void Decoder::pushVFrame(AVFrame *frame){
    //qDebug()<<"push video frame";
    std::unique_lock<std::mutex> curlock(m_videoPacketQueue.mutex);
    while(m_videoFrameQueue.size>=m_maxFrameQueueSize){
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        continue;
    }
    //这里必须先设置当前帧的时间序列，时长以及该帧的时间戳
    //然后执行引用转移
    m_videoFrameQueue.frameVec[m_videoFrameQueue.pushIndex].serial=m_videoPktDecoder.m_serial;
    m_videoFrameQueue.frameVec[m_videoFrameQueue.pushIndex].duration=\
            (m_vidFrameRate.den&&m_vidFrameRate.num)?av_q2d(AVRational{m_vidFrameRate.den,m_vidFrameRate.num}):0.00;
    m_videoFrameQueue.frameVec[m_videoFrameQueue.pushIndex].pts=\
            frame->pts*av_q2d(m_fmtCtx->streams[m_videoIndex]->time_base);
    av_frame_move_ref(&m_videoFrameQueue.frameVec[m_videoFrameQueue.pushIndex].frame,frame);
    m_videoFrameQueue.pushIndex=(m_videoFrameQueue.pushIndex+1)%m_maxFrameQueueSize;
    m_videoFrameQueue.size++;
}

#ifndef AVPLAYER_H
#define AVPLAYER_H

#include <QObject>
#include "decoder.h"
#include <QSharedPointer>
#include <QSize>
#include "clock.h"
#include "c11threadpool.h"
#include "vframe.h"
#include "sonic.h"

extern "C"{
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>
#include <SDL.h>
#include <libavutil/imgutils.h>
}

using namespace AVTool;

class YUV422Frame;

typedef Decoder::MYFRAME MyFrame;

class AVPlayer : public QObject
{
    Q_OBJECT
    friend void fillAStreamCallback(void* userdata,uint8_t* stream,int len);
public:
    enum PlayState{
        AV_STOPPED,
        AV_PLAYING,
        AV_PAUSED
    };
    AVPlayer();
    ~AVPlayer();
    int Play(const QString& url);

    void pause(bool isPause);
    
    AVTool::MediaInfo* detectMediaInfo(const QString& url);
    
    void clearPlayer();

    AVPlayer::PlayState playState();

    inline void setVFrameSize(const QSize& size){
        m_imageWidth=size.width();
        m_imageHeight=size.height();
    }

    inline void setVolume(int volumePer){
        m_volume=(volumePer*SDL_MIX_MAXVOLUME/100)%(SDL_MIX_MAXVOLUME+1);
    }

    inline void seekBy(int32_t time_s){
        seekTo((int32_t)m_audioClock.getClock()+time_s);
    }
    inline void seekTo(int32_t time_s){
        if(time_s<0)
            time_s=0;
        m_decoder->seekTo(time_s,time_s-(int32_t)m_audioClock.getClock());
    }

    inline  uint32_t avDuration(){
        return m_duration;
    }
    inline void setPlaySpeed(float speed){
        m_playSpeed=speed;
    }
signals:
    void AVDurationChanged(unsigned int duration);
    void frameChanged(QSharedPointer<YUV422Frame>frame);
    void AVTerminate();
    void AVPtsChanged(unsigned int pts);
private:
    int  initSDL();
    int  initVideo();
    void videoCallBack(std::shared_ptr<void>par);
    void initAVClock();
    void displayImage(AVFrame* frame);
    double vpDuration(MyFrame* lastFrame,MyFrame* frame );
    double computeTargetDelay(double delay);
private:
    //解码器实例
    Decoder* m_decoder;
    AVFormatContext* m_fmtCtx;
    //------------------- 音频解码参数 ---------------------//
    AVCodecParameters* m_audioCodecPar;
    //用于音频重采样的上下文 用于将不同采样率通道数和格式的音频数据进行转换和重采样
    SwrContext* m_swrCtx;
    //存放音频数据
    uint8_t* m_audioBuf;
    uint32_t m_audioBufSize;
    uint32_t m_audioBufIndex;
    uint32_t m_duration;
    uint32_t m_lastAudPts;
    //表示音频采样格式的枚举类型 以便在音频编解码，重采样和处理过程中准确的描述和操作音频数据的格式
    enum AVSampleFormat m_targetSampleFmt;
    AVClock m_audioClock;//音频播放时钟
    //存放音频数据
    int m_targetChannels;
    int m_targetFreq;
    int m_targetChannelLayout;
    int m_targetNbSamples;
    int m_volume;

    //------------------ 视频解码参数----------------------//
    AVClock m_videoClock;//视频播放时钟
    //同步时钟初始化标志 音视频异步线程 谁先读到该标志就由谁初始化时钟
    int m_clockInitFlag;
    int m_audioIndex;
    int m_videoIndex;
    int m_imageWidth;
    int m_imageHeight;
    AVFrame* m_audioFrame;
    AVCodecParameters* m_videoCodecPar;
    enum AVPixelFormat m_dstPixFmt;
    //用于视频帧的图像转换和缩放 他提供了一种机制 可以将一个视频从一个格式和尺寸转换为另一个格式和尺寸
    int m_swsFlags;
    SwsContext* m_swsCtx;
    uint8_t* m_videobuffer;
    uint8_t* m_pixels[4];
    int      m_pitch[4];
    //-----------------统一参数------------------------//
    //记录音视频帧最新播放帧的时间戳 用于同步
    int     m_pause;    //暂停标志
    int     m_exit;     //终止标志
    float   m_playSpeed;//播放速度
    double  m_frameTimer;
    double  m_delay;    //延时时间
    double  m_pauseTime;//记录暂停前的时间
    sonicStream m_sonicStream;
};

#endif // AVPLAYER_H

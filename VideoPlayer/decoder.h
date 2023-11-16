#ifndef DECODER_H
#define DECODER_H

#include<mutex>
#include<condition_variable>
#include<QVector>
#include<QDebug>
#define ERRBUF_SIZE 256
extern "C"{
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
}

QT_BEGIN_NAMESPACE
namespace AVTool {

class Decoder{
public:
    typedef struct MyFrame{
        int     serial;     //当前的解码序列
        double  duration;   //总时长
        double  pts;        //当前时间
        AVFrame frame;      //用于存放音/视频数据
    }MYFRAME;
    typedef struct MyPacket{
        int      serial;    //时间序列
        AVPacket pkt;       //存放音视频压缩数据
    }MYPACKET;
    typedef struct PacketQueue{
        int readIndex;
        int pushIndex;
        int size;
        int serial;
        std::mutex mutex;
        std::condition_variable cond;
        QVector<MYPACKET>       pktVec;
    }PACKETQUEUE;
    typedef struct FrameQueue{
        int readIndex;
        int pushIndex;
        int size;
        int shown;
        std::mutex mutex;
        std::condition_variable cond;
        QVector<MYFRAME>        frameVec;
    }FRAMEQUEUE;
    typedef struct PKtDecoder{
        int             m_serial;
        AVCodecContext* codecCtx;
    }PKTDECODER;
    Decoder();
    ~Decoder();
    void seekTo(int32_t target,int32_t seekRel);
    int  getAFrame(AVFrame* frame);
    int  getRemainingVFrame();
    //查看上一帧(该帧为当前显示的画面帧)
    MYFRAME* peekLastVFrame();
    //查看将要显示的帧
    MYFRAME* peekVFrame();
    //查看将要显示的下一帧
    MYFRAME* peekNextVFrame();
    //将读索引后移一位
    void     setNextVFrame();

    inline int vidPketSerial() const{
        return m_videoPacketQueue.serial;
    }
    inline int audPketSerial()const{
        return m_audioPacketQueue.serial;
    }
    inline int vidIndex()const{
        return m_videoIndex;
    }
    inline int audIndex()const{
        return m_audioIndex;
    }
    inline AVFormatContext* formatContext()const{
        return m_fmtCtx;
    }
    inline AVCodecParameters* audioCodecPar()const{
        return m_fmtCtx->streams[m_audioIndex]->codecpar;
    }
    inline AVCodecParameters* videoCodecPar()const{
        return m_fmtCtx->streams[m_videoIndex]->codecpar;
    }
    inline uint32_t avDuration(){
        return m_duration;
    }
    inline int isExit(){
        return m_exit;
    }
    int  decode(const QString& url);
    void exit();
private:
    //音视频压缩包处理队列
    PacketQueue m_audioPacketQueue;
    PacketQueue m_videoPacketQueue;
    //音视频帧数据处理队列
    FrameQueue  m_audioFrameQueue;
    FrameQueue  m_videoFrameQueue;
    //音视频编解码器
    PKtDecoder  m_audioPktDecoder;
    PKtDecoder  m_videoPktDecoder;
    //该结构体是FFMPEG 主要的结构体 用于记录格式化IO数据以及上下文数据
    AVFormatContext* m_fmtCtx;
    //记录帧数据以及压缩包队列处理的最大长度
    const int m_maxFrameQueueSize;
    const int m_maxPacketQueueSize;
    //AVRational结构体中有两个量 分别用于表示分数的分子和分母 用此结构体表示视频帧率
    AVRational m_vidFrameRate;

    int m_audioIndex;
    int m_videoIndex;

    int m_exit;
    //是否执行跳转
    int m_isSeek;
    //跳转后等待目标帧标志
    int m_vidSeek;
    int m_audSeek;
    //跳转相对时间
    int64_t m_seekRel;
    //跳转绝对时间
    int64_t m_seekTarget;
    //流总时长单位为s
    uint32_t m_duration;
    //用于记录错误日志的数组
    char m_errBuf[ERRBUF_SIZE];
private:
    bool init();
    void setInitval();
    void packetQueueFlush(PacketQueue* queue);
    void clearQueueCache();
    void demux(std::shared_ptr<void> par);
    void audioDecode(std::shared_ptr<void>par);
    void videoDecode(std::shared_ptr<void>par);
    int  getPacket(PacketQueue* queue,AVPacket* pkt,PKtDecoder* decoder);
    void pushPacket(PacketQueue* queue,AVPacket* pkt);
    void pushAFrame(AVFrame* frame);
    void pushVFrame(AVFrame* frame);
};
}
QT_END_NAMESPACE


#endif // DECODER_H

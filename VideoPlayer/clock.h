#ifndef CLOCK_H
#define CLOCK_H

extern "C"{
#include <libavutil/time.h>
}
class AVClock{
public:
    AVClock():m_pts(0.0),m_drift(0.0){}
    inline void reset(){
        m_pts=0.0;
        m_drift=0.0;
    }
    inline void setClock(double pts){
        setClockAt(pts);
    }
    inline double getClock(){
        return m_drift+av_gettime_relative()/1000000.0;
    }

    ~AVClock(){}
private:
    inline void setClockAt(double pts){
        m_drift=pts-av_gettime_relative()/1000000.0;
        m_pts=pts;
    }
    double  m_pts;
    double  m_drift;
};


#endif // CLOCK_H

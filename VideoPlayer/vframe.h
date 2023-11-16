#ifndef VFRAME_H
#define VFRAME_H

#include<QObject>
#include<stdlib.h>

class YUV422Frame{
public:
    YUV422Frame(uint8_t* buffer,uint32_t pixelw,uint32_t pixelh):m_buffer(nullptr){
        create(buffer,pixelw,pixelh);
    }
    ~YUV422Frame(){
        if(m_buffer)free(m_buffer);
    }
    inline uint8_t* getBufY()const{
        return m_buffer;
    }
    inline uint8_t* getBufU()const{
        return m_buffer+m_pixelH*m_pixelW;
    }
    inline uint8_t* getBufV()const{
        return m_buffer+m_pixelH*m_pixelW*3/2;
    }
    inline uint32_t getPixelW()const{
        return  m_pixelW;
    }
    inline uint32_t getPixelH()const{
        return  m_pixelH;
    }
private:
    void create(uint8_t* buffer,uint32_t pixelW,uint32_t pixelH){
        m_pixelH=pixelH;
        m_pixelW=pixelW;
        int size=m_pixelH*m_pixelW;
        if(!m_buffer)m_buffer=(uint8_t*)malloc(size*2);
        memcpy(m_buffer,buffer,size);
        memcpy(m_buffer+size,buffer+size,size/2);
        memcpy(m_buffer+size*3/2,buffer+size*3/2,size/2);
    }
    uint8_t* m_buffer;
    uint32_t m_pixelW;
    uint32_t m_pixelH;
};

#endif // VFRAME_H

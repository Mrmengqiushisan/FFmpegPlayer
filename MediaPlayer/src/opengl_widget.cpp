#include "opengl_widget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QDebug>
#include "vframe.h"
#include <QApplication>
#include <QScreen>

#define VERTEXIN 0
#define TEXTUREIN 1

OpenGLWidget::OpenGLWidget(QWidget *parent):
    QOpenGLWidget(parent),
    m_isDoubleClick(0),
    m_scaleRate(ScaleRate::RATE_ORIGIN)
{
    setMouseTracking(true);
    connect(&m_timer,&QTimer::timeout,[this](){
        if(!this->m_isDoubleClick)
            emit this->mouseClicked();
        this->m_isDoubleClick=0;
        this->m_timer.stop();
    });
    m_timer.setInterval(300);
}

OpenGLWidget::~OpenGLWidget()
{
    makeCurrent();
    vbo.destroy();
    textureY->destroy();
    textureU->destroy();
    textureV->destroy();
    doneCurrent();
}

void OpenGLWidget::resetFrame()
{
    m_frame->setToPureBlack();
    update();
}

void OpenGLWidget::setScaleRate(OpenGLWidget::ScaleRate scaleRate)
{
    m_scaleRate=scaleRate;
}

void OpenGLWidget::onShowYUV(QSharedPointer<YUV420Frame> frame)
{
    if(!frame)
        qDebug()<<"null frame!\n";
    if(m_frame)
        m_frame.reset();
    m_frame=frame;
    update();
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    //内存对齐设置为1，防止宽度无法被4整除导致画面渲染扭曲
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    static const GLfloat vertices[]{
        //顶点坐标
        -1.0f,-1.0f,
        -1.0f,+1.0f,
        +1.0f,+1.0f,
        +1.0f,-1.0f,
        //纹理坐标
        0.0f,1.0f,
        0.0f,0.0f,
        1.0f,0.0f,
        1.0f,1.0f,
    };

    vbo.create();
    vbo.bind();
    vbo.allocate(vertices,sizeof(vertices));

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    //attribute 用于在顶点着色器中声明传入的顶点属性变量 varying 用于在顶点着色器和片段着色器之间传递数据
    const char *vsrc =
        "attribute vec4 vertexIn; \
        attribute vec2 textureIn; \
        varying vec2 textureOut;  \
        void main(void)           \
        {                         \
        gl_Position = vertexIn; \
        textureOut = textureIn; \
        }";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc = "varying vec2 textureOut; \
    uniform sampler2D tex_y; \
    uniform sampler2D tex_u; \
    uniform sampler2D tex_v; \
    void main(void) \
    { \
        vec3 yuv; \
        vec3 rgb; \
        yuv.x = texture2D(tex_y, textureOut).r; \
        yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
        yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
        rgb = mat3( 1,       1,         1, \
                    0,       -0.39465,  2.03211, \
                    1.13983, -0.58060,  0) * yuv; \
        gl_FragColor = vec4(rgb, 1); \
    }";
    fshader->compileSourceCode(fsrc);

    program = new QOpenGLShaderProgram(this);

    program->addShader(vshader);
    program->addShader(fshader);

    //program->addShaderFromSourceCode(QOpenGLShader::Vertex,vshadersrc);
    //program->addShaderFromSourceCode(QOpenGLShader::Fragment,fshadersrc);

    //绑定输入的定点坐标和纹理坐标属性
    program->bindAttributeLocation("vertexIn",VERTEXIN);
    program->bindAttributeLocation("textureIn",TEXTUREIN);
    program->link();
    program->bind();

    //启用并且设置定点位置和纹理坐标
    program->enableAttributeArray(VERTEXIN);
    program->enableAttributeArray(TEXTUREIN);
    program->setAttributeBuffer(VERTEXIN,GL_FLOAT, 0, 2, 2*sizeof(GLfloat));
    program->setAttributeBuffer(TEXTUREIN,GL_FLOAT,8*sizeof(GLfloat), 2 , 2*sizeof(GLfloat));

    //定位shader中 uniform变量
    posUniformY = program->uniformLocation("tex_y");
    posUniformU = program->uniformLocation("tex_u");
    posUniformV = program->uniformLocation("tex_v");

    //创建纹理并且获取id
    textureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureY->create();
    textureU->create();
    textureV->create();
    m_idY = textureY->textureId();
    m_idU = textureU->textureId();
    m_idV = textureV->textureId();

    glClearColor(0.0,0.0,0.0,0.0);

}

void OpenGLWidget::resizeGL(int w, int h)
{
    m_curWidth=width();
    m_curHeight=height();
}

void OpenGLWidget::paintGL()
{
    if(!m_frame)
        return;
    uint32_t videoW=m_frame->getPixelW();
    uint32_t videoH=m_frame->getPixelH();

    if(m_scaleRate!=ScaleRate::RATE_FULLSCREEN) {
        int rateW,rateH;
        switch (m_scaleRate) {
            case ScaleRate::RATE_ORIGIN:
                rateW=videoW;
                rateH=videoH;
                break;
            case ScaleRate::RATE_4_3:
                rateW=4;
                rateH=3;
                break;
            case ScaleRate::RATE_16_9:
                rateW=16;
                rateH=9;
                break;
        }

        int targetWidth, targetHeight;

        if (m_curWidth * rateH > m_curHeight * rateW) {
            // 窗口的宽度过宽，以高度为基准进行缩放
            targetHeight = m_curHeight;
            targetWidth = (m_curHeight * rateW) / rateH;
        } else {
            // 窗口的高度过高，以宽度为基准进行缩放
            targetWidth = m_curWidth;
            targetHeight = (m_curWidth * rateH) / rateW;
        }

        // 设置视口，保持等比例缩放
        int xOffset = (m_curWidth - targetWidth) / 2;
        int yOffset = (m_curHeight - targetHeight) / 2;

        //qDebug()<<"X:"<<xOffset<<" Y"<<yOffset<<"W:"<<targetWidth<<" H:"<<targetHeight<<"w:"<<w<<" h:"<<h;

        glViewport(xOffset, yOffset, targetWidth, targetHeight);
    }

    //激活纹理单元GL_TEXTURE0,系统里面的
    glActiveTexture(GL_TEXTURE0);

    //绑定y分量纹理对象id到激活的纹理单元
    glBindTexture(GL_TEXTURE_2D,m_idY);

    //使用内存中的数据创建真正的y分量纹理数据
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,videoW,videoH,0,GL_RED,GL_UNSIGNED_BYTE,m_frame->getBufY());//m_yPtr 大小是(videoW*videoH)

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //激活纹理单元GL_TEXTURE1
    glActiveTexture(GL_TEXTURE1);
    //绑定u分量纹理对象id到激活的纹理单元
    glBindTexture(GL_TEXTURE_2D,m_idU);
    //使用内存中的数据创建真正的u分量纹理数据
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,videoW>>1 , videoH,0,GL_RED,GL_UNSIGNED_BYTE,m_frame->getBufU());//m_uPtr 大小是(videoW*videoH / 2)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //激活纹理单元GL_TEXTURE2
    glActiveTexture(GL_TEXTURE2);
    //绑定v分量纹理对象id到激活的纹理单元
    glBindTexture(GL_TEXTURE_2D,m_idV);
    //使用内存中的数据创建真正的v分量纹理数据
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, videoW>>1 , videoH, 0, GL_RED, GL_UNSIGNED_BYTE, m_frame->getBufV());//m_vPtr 大小是(videoW*videoH / 2)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //指定y纹理要使用新值
    glUniform1i(posUniformY, 0);
    //指定u纹理要使用新值
    glUniform1i(posUniformU, 1);
    //指定v纹理要使用新值
    glUniform1i(posUniformV, 2);
    //使用顶点数组方式绘制图形
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_timer.isActive())
        m_timer.start();
}

void OpenGLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_isDoubleClick=1;
    emit mouseDoubleClicked();
}



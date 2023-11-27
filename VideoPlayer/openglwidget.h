#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QObject>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QTimer>
#include <memory>
#include <QSharedPointer>

class YUV422Frame;

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

class OpenGLWidget : public QOpenGLWidget,protected QOpenGLFunctions{
    Q_OBJECT
public:
    //显示比例
    enum class ScaleRate:int{
        RATE_4_3,   //4:3
        RATE_16_9,  //16:9
        RATE_FULLSCREEN,    //铺满
        RATE_ORIGIN //原始比例
    };
    explicit OpenGLWidget(QWidget *parent=nullptr);
    ~OpenGLWidget();
    void resetFrame();
    void setScaleRate(OpenGLWidget::ScaleRate scaleRate);
signals:
    void mouseClicked();
    void mouseDoubleClicked();
public slots:
    void onShowYUV(QSharedPointer<YUV422Frame>frame);
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    virtual void mouseReleaseEvent(QMouseEvent* event)override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event)override;
private:
    QOpenGLShaderProgram* program;
    QOpenGLBuffer  vbo;

    //opengl中y、u、v分量位置
    GLuint posUniformY,posUniformU,posUniformV;
    //纹理
    QOpenGLTexture *textureY = nullptr,*textureU = nullptr,*textureV = nullptr;
    //纹理ID，创建错误返回0
    GLuint m_idY,m_idU,m_idV;
    //原始数据
    QSharedPointer<YUV422Frame> m_frame;
    int m_isDoubleClick;
    int m_curWidth;
    int m_curHeight;
    QTimer m_timer;
    ScaleRate m_scaleRate;
};

#endif // OPENGLWIDGET_H

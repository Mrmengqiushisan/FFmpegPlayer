#ifndef OPENGL_WIDGET_H
#define OPENGL_WIDGET_H
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QTimer>
#include <memory>
#include <QPushButton>

class YUV420Frame;

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

class OpenGLWidget : public QOpenGLWidget,protected QOpenGLFunctions
{
    Q_OBJECT

public:
    //显示比例
    enum class ScaleRate:int
    {
        RATE_4_3,   //4:3
        RATE_16_9,  //16:9
        RATE_FULLSCREEN,    //铺满
        RATE_ORIGIN     //原始比例
    };

    explicit OpenGLWidget(QWidget *parent = 0);
    ~OpenGLWidget();

    void resetFrame();

    void setScaleRate(OpenGLWidget::ScaleRate scaleRate);

signals:
    void mouseClicked();
    void mouseDoubleClicked();

public slots:
    void onShowYUV(QSharedPointer<YUV420Frame> frame);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) override;

    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

private:
    QOpenGLShaderProgram *program;
    QOpenGLBuffer vbo;

    //opengl中y、u、v分量位置
    GLuint posUniformY,posUniformU,posUniformV;

    //纹理
    QOpenGLTexture *textureY = nullptr,*textureU = nullptr,*textureV = nullptr;

    //纹理ID，创建错误返回0
    GLuint m_idY,m_idU,m_idV;

    //待显示图片数据
    QSharedPointer<YUV420Frame> m_frame;

    int m_isDoubleClick;
    int m_curWidth;
    int m_curHeight;

    ScaleRate m_scaleRate;

    QTimer m_timer;
};


#endif


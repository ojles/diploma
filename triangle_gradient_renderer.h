#ifndef CALCULATIONRESULTRENDERER_H
#define CALCULATIONRESULTRENDERER_H

#include <QQuickFramebufferObject>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/qopenglshaderprogram.h>
#include <QtGui/qopenglfunctions.h>

class TriangleGradientRenderer : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions {
public:
    TriangleGradientRenderer(QVector<QVector2D>* vertices,
                             QVector<QVector4D>* colors,
                             QVector<unsigned int>* indices,
                             bool* showTriangulation,
                             bool* dataChanged);

    ~TriangleGradientRenderer();

    void render() override;

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

private:
    QString loadShaderFromFile(QString fileName);

    QVector<QVector2D>* vertices;
    QVector<QVector4D>* colors;
    QVector<unsigned int>* indices;
    bool* showTriangulation;
    bool* dataChanged;

    QOpenGLShaderProgram program;
    QOpenGLVertexArrayObject* vao;
    QOpenGLBuffer* vbov;
    QOpenGLBuffer* vboc;
    QOpenGLBuffer* ibo;

    int vertexAttribute;
    int colorAttribute;
};

#endif

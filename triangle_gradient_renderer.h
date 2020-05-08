#ifndef CALCULATIONRESULTRENDERER_H
#define CALCULATIONRESULTRENDERER_H

#include <QQuickFramebufferObject>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/qopenglshaderprogram.h>
#include <QtGui/qopenglfunctions.h>

class TriangleGradientRenderer : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions
{
public:
    TriangleGradientRenderer(QVector<QVector2D>* vertices, QVector<QVector4D>* colors);

    void render() override;

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

private:
    QString loadShaderFromFile(QString fileName);

    QVector<QVector2D>* vertices;
    QVector<QVector4D>* colors;
    QOpenGLShaderProgram program;
    int vertexAttribute;
    int colorAttribute;
};

#endif // CALCULATIONRESULTRENDERER_H

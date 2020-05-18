#include "triangle_gradient_renderer.h"

TriangleGradientRenderer::TriangleGradientRenderer(QVector<QVector2D>* vertices, QVector<QVector4D>* colors) {
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    this->vertices = vertices;
    this->colors = colors;

    program.addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":shader/Vertex.shader");
    program.addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":shader/Fragment.shader");
    program.link();

    vertexAttribute = program.attributeLocation("vertex");
    colorAttribute = program.attributeLocation("custom_color");
}

void TriangleGradientRenderer::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    program.bind();
    program.enableAttributeArray(vertexAttribute);
    program.enableAttributeArray(colorAttribute);
    program.setAttributeArray(vertexAttribute, vertices->constData());
    program.setAttributeArray(colorAttribute, colors->constData());
    glDrawArrays(GL_TRIANGLES, 0, vertices->size());
    program.disableAttributeArray(vertexAttribute);
    program.disableAttributeArray(colorAttribute);
    program.release();

    update();
}

QOpenGLFramebufferObject* TriangleGradientRenderer::createFramebufferObject(const QSize &size) {
    QOpenGLFramebufferObjectFormat format;
    return new QOpenGLFramebufferObject(size, format);
}

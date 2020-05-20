#include "triangle_gradient_renderer.h"

TriangleGradientRenderer::TriangleGradientRenderer(
        QVector<QVector2D>* vertices,
        QVector<QVector4D>* colors,
        QVector<unsigned int>* indices,
        bool* showTriangulation,
        bool* dataChanged)
    : vertices(vertices),
      colors(colors),
      indices(indices),
      showTriangulation(showTriangulation),
      dataChanged(dataChanged),
      vao(new QOpenGLVertexArrayObject()),
      vbov(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer)),
      vboc(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer)),
      ibo(new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer)) {

    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_PRIMITIVE_RESTART_INDEX);

    program.addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":shader/Vertex.shader");
    program.addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":shader/Fragment.shader");
    program.link();

    vertexAttribute = program.attributeLocation("vertex");
    colorAttribute = program.attributeLocation("custom_color");

    vao->create();
    vao->bind();

    vbov->create();
    vbov->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vbov->bind();
    program.enableAttributeArray(vertexAttribute);
    program.setAttributeBuffer(vertexAttribute, GL_FLOAT, 0, 2);

    vboc->create();
    vboc->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboc->bind();
    program.enableAttributeArray(colorAttribute);
    program.setAttributeBuffer(colorAttribute, GL_FLOAT, 0, 4);

    ibo->create();
    ibo->setUsagePattern(QOpenGLBuffer::StaticDraw);
    ibo->bind();

    vao->release();
    program.release();
}

void TriangleGradientRenderer::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // TODO: make async
    if (*dataChanged) {
        int verticesAmount = vertices->size();
        vertices->reserve(verticesAmount * 2);
        colors->reserve(verticesAmount * 2);
        for (int i = 0; i < verticesAmount; i++) {
            vertices->push_back(vertices->at(i));
            colors->push_back(QVector4D(1.0f, 0.0f, 0.0f, 0.4f));
        }

        int indicesAmount = indices->size();
        indices->reserve(indicesAmount * 2);
        for (int i = 0; i < indicesAmount; i++) {
            indices->push_back(indices->at(i) + verticesAmount);
        }

        vbov->bind();
        vbov->allocate(vertices->constData(), vertices->size() * sizeof(QVector2D));
        vboc->bind();
        vboc->allocate(colors->constData(), colors->size() * sizeof(QVector4D));
        ibo->bind();
        ibo->allocate(indices->constData(), indices->size() * sizeof(unsigned int));
        *dataChanged = false;
    }

    program.bind();
    vao->bind();
    glDrawElements(GL_TRIANGLES, indices->size() / 2, GL_UNSIGNED_INT, 0);
    if (*showTriangulation) {
        for (int i = indices->size() / 2; i < indices->size(); i += 3) {
            glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * i));
        }
    }
    vao->release();
    program.release();

    update();
}

TriangleGradientRenderer::~TriangleGradientRenderer() {
    delete vao;
    delete vbov;
    delete vboc;
    delete ibo;
}

QOpenGLFramebufferObject* TriangleGradientRenderer::createFramebufferObject(const QSize &size) {
    QOpenGLFramebufferObjectFormat format;
    return new QOpenGLFramebufferObject(size, format);
}

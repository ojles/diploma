attribute highp vec4 vertex;
attribute mediump vec4 custom_color;
varying mediump vec4 color;

void main(void)
{
    color = custom_color;
    gl_Position = vec4(vertex.x, -vertex.y, vertex.z, vertex.w);
}

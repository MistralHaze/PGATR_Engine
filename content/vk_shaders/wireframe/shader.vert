#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 _model;
    mat4 _view;
    mat4 _proj;
} ubo;

layout(location = 0) in vec3 inPosition;


void main() {
    gl_Position = ubo._proj * ubo._view * ubo._model * vec4(inPosition, 1.0);
}

#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 _model;
    mat4 _view;
    mat4 _proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 tcPos;
layout(location = 1) out vec3 tcColor;
layout(location = 2) out vec2 tcTexCoord;


void main() {
    gl_Position = ubo._proj * ubo._view * ubo._model * vec4(inPosition, 1.0);
    tcPos = (ubo._proj * ubo._view * ubo._model * vec4(inPosition, 1.0)).xyz;
    tcColor = inColor;
    tcTexCoord = inTexCoord;
}

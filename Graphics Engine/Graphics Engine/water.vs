#version 330 core
layout (location = 0) in vec2 position; // only need to pass vec2 since y is set to 0

out vec2 textureCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(void) {
    
    gl_Position = projection * view * model * vec4(position.x, 0.0, position.y, 1.0);
    
    textureCoords = vec2(position.x/2.0 + 0.5, position.y/2.0 + 0.5); // 不確定要幹嘛
    
}

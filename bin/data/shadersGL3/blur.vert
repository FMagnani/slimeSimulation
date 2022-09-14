#version 150

// these are for the programmable pipeline system and are passed in
// by default from OpenFrameworks
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in vec4 color;
in vec4 normal;
in vec2 texcoord;
// this is the end of the default functionality

out vec2 texCoordVarying;

void main()
{

    texCoordVarying = texcoord;

    // send the vertices to the fragment shader
    gl_Position = modelViewProjectionMatrix * position;
}

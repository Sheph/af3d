#version 330 core

uniform sampler2D texMain;
uniform sampler2D texSpecular;

uniform vec4 mainColor;
uniform vec4 specularColor;
uniform vec4 lightColor;
uniform vec3 lightDir;

in vec2 v_texCoord;
in vec3 v_viewDir;
in float v_lightType;
in vec3 v_lightDir;

out vec4 fragColor;

void main()
{
    if (v_lightType == 0.0) {
        // ambient
        fragColor = texture(texMain, v_texCoord) * mainColor * vec4(lightColor.rgb, 1.0);
    }
}

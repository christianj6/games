#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 normal     = normalize(fragNormal);
    vec3 tint       = colDiffuse.rgb * fragColor.rgb;

    vec3 diffuse = vec3(0.0);
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light;
            if (lights[i].type == LIGHT_DIRECTIONAL)
                light = -normalize(lights[i].target - lights[i].position);
            else
                light = normalize(lights[i].position - fragPosition);

            diffuse += lights[i].color.rgb * max(dot(normal, light), 0.0);
        }
    }

    vec3 ambientColor = (ambient.rgb / 10.0) * tint;
    vec3 color        = texelColor.rgb * (tint * diffuse + ambientColor);

    finalColor = clamp(vec4(color, texelColor.a * colDiffuse.a), 0.0, 1.0);
    finalColor = pow(finalColor, vec4(1.0 / 2.2));
}

#version 440

in vec2 TexCoord;
in vec4 fragPos;
in vec3 vNormals;
out vec4 FragColor;

uniform sampler2D colorTexture; // base texture (albedo/diffuse)
uniform sampler2D aoMap;// Ambient Occlusion (black = shadown)
uniform vec3 mainLight;	//light position
uniform vec3 fillLight; // 2nd light (fill)
uniform vec3 backLight; // 3rd light (back)
uniform vec3 lightColor; // main light color (warm)
uniform vec3 lightColorFill; // fill light (cooler, bluish)
uniform vec3 lightColorBack; // back light (neutral white)
uniform vec3 ka;		//environment light
uniform vec3 kd;		//difuse light
uniform vec3 viewPos;   // camera position (world space)
uniform float ks;       // specular factor (ex: 0.2)
uniform float shininess;// brightness exponent (ex: 32.0)

void main()
{
	
    vec3 baseColor = texture(colorTexture, TexCoord).rgb;
    float ao = texture(aoMap, TexCoord).r; // use only R channel

    vec3 N = normalize(vNormals);
    vec3 V = normalize(viewPos - vec3(fragPos));

    // --- Ambient lighting ---
    float kaIntensity=0.5;
    vec3 ambient = ka * kaIntensity;

    // --- Main light ---
    vec3 L_main = normalize(mainLight - vec3(fragPos));
    vec3 diffuseMain = kd * max(dot(L_main, N), 0.0f) * lightColor;
    vec3 R_main = reflect(-L_main, N);
    float specMain = pow(max(dot(V, R_main), 0.0), shininess);
    vec3 specularMain = ks * specMain * lightColor;

    // --- Fill light ---
    vec3 L_fill = normalize(fillLight - vec3(fragPos));
    vec3 diffuseFill = kd * max(dot(L_fill, N), 0.0f) * lightColorFill;
    vec3 R_fill = reflect(-L_fill, N);
    float specFill = pow(max(dot(V, R_fill), 0.0), shininess);
    vec3 specularFill = ks * specFill * lightColorFill;

    // --- Back light ---
    vec3 L_back = normalize(backLight - vec3(fragPos));
    vec3 diffuseBack = kd * max(dot(L_back, N), 0.0f) * lightColorBack;
    vec3 R_back = reflect(-L_back, N);
    float specBack = pow(max(dot(V, R_back), 0.0), shininess);
    vec3 specularBack = ks * specBack * lightColorBack;

    vec3 finalColor = (ambient + 
                       diffuseMain + specularMain +
                       diffuseFill + specularFill +
                       diffuseBack + specularBack) * baseColor * ao;

    FragColor = vec4(finalColor, 1.0);
}

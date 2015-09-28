//flat varying vec4 vColour;
varying vec4 vColour;
varying vec3 vNormal;
varying vec3 vPosEye;
varying vec3 vVertex;
uniform float uOpacity;
uniform bool uLighting;
uniform float uBrightness;
uniform float uContrast;
uniform float uSaturation;
uniform float uAmbient;
uniform float uDiffuse;
uniform float uSpecular;
uniform bool uTextured;
uniform bool uCalcNormal;
uniform sampler2D uTexture;
uniform vec3 uClipMin;
uniform vec3 uClipMax;

#define HUGEVAL 1e20

void main(void)
{
  //Clip planes in X/Y/Z (shift seems to be required on nvidia)
  if (any(lessThan(vVertex, uClipMin - vec3(0.01))) || any(greaterThan(vVertex, uClipMax + vec3(0.01)))) discard;

  vec4 fColour = vColour;
  if (uTextured) 
    fColour = texture2D(uTexture, gl_TexCoord[0].xy);

  if (!uLighting) 
  {
    gl_FragColor = fColour;
    return;
  }
  
  //TODO: uniforms for rest of light params!
  const float shininess = 100.0; //Size of highlight (higher is smaller)
  const vec3 light = vec3(1.0, 1.0, 1.0);  //Colour of light

  //Head light, lightPos=(0,0,0) - vPosEye
  vec3 lightDir = normalize(-vPosEye);

  //Calculate diffuse lighting
  vec3 N = normalize(vNormal);

  //Default normal...
  if (length(N) < 0.9 || uCalcNormal)
  {
    vec3 fdx = vec3(dFdx(vPosEye.x),dFdx(vPosEye.y),dFdx(vPosEye.z));    
    vec3 fdy = vec3(dFdy(vPosEye.x),dFdy(vPosEye.y),dFdy(vPosEye.z));
    N = normalize(cross(fdx,fdy)); 
  }

  float diffuse = abs(dot(N, lightDir));

   //Compute the specular term
   vec3 specular = vec3(0.0,0.0,0.0);
   if (diffuse > 0.0 && uSpecular > 0.0)
   {
      vec3 specolour = vec3(1.0, 1.0, 1.0);   //Color of light
      //Normalize the half-vector
      //vec3 halfVector = normalize(vPosEye + lightDir);
      vec3 halfVector = normalize(vec3(0.0, 0.0, 1.0) + lightDir);
      //Compute cosine (dot product) with the normal (abs for two-sided)
      float NdotHV = abs(dot(N, halfVector));
      specular = specolour * pow(NdotHV, shininess);
   }

  vec3 lightWeighting = light * (uAmbient + diffuse * uDiffuse + specular * uSpecular);
  float alpha = fColour.a;
  if (uOpacity > 0.0) alpha *= uOpacity;
  vec4 colour = vec4(fColour.rgb * lightWeighting, alpha);

  //Brightness adjust
  colour += uBrightness;
  //Saturation & Contrast adjust
  const vec4 LumCoeff = vec4(0.2125, 0.7154, 0.0721, 0.0);
  vec4 AvgLumin = vec4(0.5, 0.5, 0.5, 0.0);
  vec4 intensity = vec4(dot(colour, LumCoeff));
  colour = mix(intensity, colour, uSaturation);
  colour = mix(AvgLumin, colour, uContrast);
  colour.a = alpha;

  if (alpha < 0.01) discard;

  gl_FragColor = colour;
}


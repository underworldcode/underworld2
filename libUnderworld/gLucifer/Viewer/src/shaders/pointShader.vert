//#version 120 //Required to use gl_PointCoord
uniform float uPointScale;   // scale to calculate size in pixels
uniform int uPointDist;   // Scale by distance

attribute float aSize;
attribute float aPointType;

varying float vSmooth;
varying float vPointType;
varying vec3 vPosEye;
varying float vPointSize;
varying vec3 vVertex;

void main(void)
{
   vSmooth = aSize;
   float pSize = abs(aSize);

   // calculate window-space point size
   vec3 posEye = vec3(gl_ModelViewMatrix * gl_Vertex);
   float dist = 1.0;
   if (uPointDist > 0)
      dist = length(posEye);
   //Limit scaling, overly large points are very slow to render
   //gl_PointSize = max(1.0, min(40.0, uPointScale * pSize / dist));
   gl_PointSize = uPointScale * pSize / dist;
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
   gl_FrontColor = gl_Color;
   vPosEye = posEye;
   vPointType = aPointType;
   vPointSize = gl_PointSize;
   vVertex = gl_Vertex.xyz;
}


/*
 * Copyright (c) 2014, Monash University. All rights reserved.
 * Author: Owen Kaluza - owen.kaluza ( at ) monash.edu
 *
 * Licensed under the GNU Lesser General Public License
 * https://www.gnu.org/licenses/lgpl.html
 * (volume shader from sharevol https://github.com/OKaluza/sharevol)
 */
#version 120
//precision highp float;

//Defined dynamically before compile...
const int maxSamples = 2048;

uniform sampler3D uVolume;
uniform sampler2D uTransferFunction;

uniform vec3 uBBMin;
uniform vec3 uBBMax;
uniform vec3 uResolution;

uniform bool uEnableColour;

uniform float uBrightness;
uniform float uContrast;
uniform float uSaturation;
uniform float uPower;

uniform mat4 uPMatrix;
uniform mat4 uInvPMatrix;
uniform mat4 uMVMatrix;
uniform mat4 uNMatrix;
uniform vec4 uViewport;
uniform int uSamples;
uniform float uDensityFactor;
uniform float uIsoValue;
uniform vec4 uIsoColour;
uniform float uIsoSmooth;
uniform int uIsoWalls;
uniform int uFilter;
uniform vec2 uRange;

//#define tex3D(pos) interpolate_tricubic_fast(pos)
//#define tex3D(pos) texture3Dfrom2D(pos).x

float tex3D(vec3 pos) 
{
  //if (uFilter > 0)
  //  return interpolate_tricubic_fast(pos);
  return texture3D(uVolume, pos).x; //from2D(pos).x;
}

// It seems WebGL has no transpose
mat4 transpose(in mat4 m)
{
  return mat4(
              vec4(m[0].x, m[1].x, m[2].x, m[3].x),
              vec4(m[0].y, m[1].y, m[2].y, m[3].y),
              vec4(m[0].z, m[1].z, m[2].z, m[3].z),
              vec4(m[0].w, m[1].w, m[2].w, m[3].w)
             );
}

//Light moves with camera
const vec3 lightPos = vec3(0.5, 0.5, 5.0);
const float ambient = 0.2;
const float diffuse = 0.8;
const vec3 diffColour = vec3(1.0, 1.0, 1.0);  //Colour of diffuse light
const vec3 ambColour = vec3(0.2, 0.2, 0.2);   //Colour of ambient light

void lighting(in vec3 pos, in vec3 normal, inout vec3 colour)
{
  vec4 vertPos = uMVMatrix * vec4(pos, 1.0);
  vec3 lightDir = normalize(lightPos - vertPos.xyz);
  vec3 lightWeighting = ambColour + diffColour * diffuse * clamp(abs(dot(normal, lightDir)), 0.1, 1.0);

  colour *= lightWeighting;
}

vec3 isoNormal(in vec3 pos, in vec3 shift, in float density)
{
  vec3 shiftpos = vec3(pos.x + shift.x, pos.y + shift.y, pos.z + shift.z);
  vec3 shiftx = vec3(shiftpos.x, pos.y, pos.z);
  vec3 shifty = vec3(pos.x, shiftpos.y, pos.z);
  vec3 shiftz = vec3(pos.x, pos.y, shiftpos.z);

  //Detect bounding box hit (walls)
  if (uIsoWalls > 0)
  {
    if (pos.x <= uBBMin.x) return vec3(-1.0, 0.0, 0.0);
    if (pos.x >= uBBMax.x) return vec3(1.0, 0.0, 0.0);
    if (pos.y <= uBBMin.y) return vec3(0.0, -1.0, 0.0);
    if (pos.y >= uBBMax.y) return vec3(0.0, 1.0, 0.0);
    if (pos.z <= uBBMin.z) return vec3(0.0, 0.0, -1.0);
    if (pos.z >= uBBMax.z) return vec3(0.0, 0.0, 1.0);
  }

  //Calculate normal
  return vec3(density) - vec3(tex3D(shiftx), tex3D(shifty), tex3D(shiftz));
}

vec2 rayIntersectBox(vec3 rayDirection, vec3 rayOrigin)
{
  //Intersect ray with bounding box
  vec3 rayInvDirection = 1.0 / rayDirection;
  vec3 bbMinDiff = (uBBMin - rayOrigin) * rayInvDirection;
  vec3 bbMaxDiff = (uBBMax - rayOrigin) * rayInvDirection;
  vec3 imax = max(bbMaxDiff, bbMinDiff);
  vec3 imin = min(bbMaxDiff, bbMinDiff);
  float back = min(imax.x, min(imax.y, imax.z));
  float front = max(max(imin.x, 0.0), max(imin.y, imin.z));
  return vec2(back, front);
}

void main()
{
    //Compute eye space coord from window space to get the ray direction
    mat4 invMVMatrix = transpose(uMVMatrix);
    //ObjectSpace *[MV] = EyeSpace *[P] = Clip /w = Normalised device coords ->VP-> Window
    //Window ->[VP^]-> NDC ->[/w]-> Clip ->[P^]-> EyeSpace ->[MV^]-> ObjectSpace
    vec4 ndcPos;
    ndcPos.xy = ((2.0 * gl_FragCoord.xy) - (2.0 * uViewport.xy)) / (uViewport.zw) - 1;
    ndcPos.z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) /
               (gl_DepthRange.far - gl_DepthRange.near);
    ndcPos.w = 1.0;
    vec4 clipPos = ndcPos / gl_FragCoord.w;
    //vec4 eyeSpacePos = uInvPMatrix * clipPos;
    vec3 rayDirection = normalize((invMVMatrix * uInvPMatrix * clipPos).xyz);

    //Ray origin from the camera position
    vec4 camPos = -vec4(uMVMatrix[3]);  //4th column of modelview
    vec3 rayOrigin = (invMVMatrix * camPos).xyz;

    //Calc step
    float stepSize = 1.732 / float(uSamples); //diagonal of [0,1] normalised coord cube = sqrt(3)

    //Intersect ray with bounding box
    vec2 intersection = rayIntersectBox(rayDirection, rayOrigin);
    //Subtract small increment to avoid errors on front boundary
    intersection.y -= 0.000001;
    //Discard points outside the box (no intersection)
    if (intersection.x <= intersection.y) discard;

    vec3 rayStart = rayOrigin + rayDirection * intersection.y;
    vec3 rayStop = rayOrigin + rayDirection * intersection.x;

    vec3 step = normalize(rayStop-rayStart) * stepSize;
    vec3 pos = rayStart;

    float T = 1.0;
    vec3 colour = vec3(0.0);
    bool inside = false;
    vec3 shift = uIsoSmooth / uResolution;
    //Number of samples to take along this ray before we pass out back of volume...
    float travel = distance(rayStop, rayStart) / stepSize;
    int samples = int(ceil(travel));
    float range = uRange.y - uRange.x;
    if (range <= 0.0) range = 1.0;
  
    //Raymarch, front to back
    for (int i=0; i < maxSamples; ++i)
    {
      //Render samples until we pass out back of cube or fully opaque
#ifndef IE11
      if (i == samples || T < 0.01) break;
#else
      //This is slower but allows IE 11 to render, break on non-uniform condition causes it to fail
      if (i == uSamples) break;
      if (all(greaterThanEqual(pos, uBBMin)) && all(lessThanEqual(pos, uBBMax)))
#endif
      {
        //Get density 
        float density = tex3D(pos);

#define ISOSURFACE
#ifdef ISOSURFACE
        //Passed through isosurface?
        if (uIsoValue > 0.0 && ((!inside && density >= uIsoValue) || (inside && density < uIsoValue)))
        {
          inside = !inside;
          //Find closer to exact position by iteration
          //http://sizecoding.blogspot.com.au/2008/08/isosurfaces-in-glsl.html
          float exact;
          float a = intersection.y + (float(i)*stepSize);
          float b = a - stepSize;
          for (int j = 0; j < 5; j++)
          {
            exact = (b + a) * 0.5;
            pos = rayDirection * exact + rayOrigin;
            density = tex3D(pos);
            if (density - uIsoValue < 0.0)
              b = exact;
            else
              a = exact;
          }

          //Skip edges unless flagged to draw
          if (uIsoWalls > 0 || all(greaterThanEqual(pos, uBBMin)) && all(lessThanEqual(pos, uBBMax)))
          {
            vec4 value = vec4(uIsoColour.rgb, 1.0);

            //normal = normalize(normal);
            //if (length(normal) < 1.0) normal = vec3(0.0, 1.0, 0.0);
            vec3 normal = normalize(mat3(uNMatrix) * isoNormal(pos, shift, density));

            vec3 light = value.rgb;
            lighting(pos, normal, light);
            //Front-to-back blend equation
            colour += T * uIsoColour.a * light;
            T *= (1.0 - uIsoColour.a);
          }
        }
#endif

        if (uDensityFactor > 0.0)
        {
          //Normalise the density over provided range
          density = (density - uRange.x) / range;

          density = pow(density, uPower); //Apply power

          vec4 value;
          if (uEnableColour)
            value = texture2D(uTransferFunction, vec2(density, 0.5));
          else
            value = vec4(density);

          value *= uDensityFactor * stepSize;

          //Color
          colour += T * value.rgb;
          //Alpha
          T *= 1.0 - value.a;
        }
      }
	  
      //Next sample...
      pos += step;
    }

    //Apply brightness, saturation & contrast
    colour += uBrightness;
    const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
    vec3 AvgLumin = vec3(0.5, 0.5, 0.5);
    vec3 intensity = vec3(dot(colour, LumCoeff));
    colour = mix(intensity, colour, uSaturation);
    colour = mix(AvgLumin, colour, uContrast);

    if (T > 0.95) discard;
    gl_FragColor = vec4(colour, 1.0 - T);

#ifdef WRITE_DEPTH
    /* Write the depth !Not supported in WebGL without extension */
    vec4 clip_space_pos = uPMatrix * uMVMatrix * vec4(rayStart, 1.0);
    float ndc_depth = clip_space_pos.z / clip_space_pos.w;
    float depth = (((gl_DepthRange.far - gl_DepthRange.near) * ndc_depth) + 
                     gl_DepthRange.near + gl_DepthRange.far) / 2.0;
    gl_FragDepth = depth;
#endif
}

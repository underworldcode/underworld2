/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
** Copyright (c) 2010, Monash University
** All rights reserved.
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**
**       * Redistributions of source code must retain the above copyright notice,
**          this list of conditions and the following disclaimer.
**       * Redistributions in binary form must reproduce the above copyright
**         notice, this list of conditions and the following disclaimer in the
**         documentation and/or other materials provided with the distribution.
**       * Neither the name of the Monash University nor the names of its contributors
**         may be used to endorse or promote products derived from this software
**         without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
** THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
** BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
** OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**
** Contact:
*%  Owen Kaluza - Owen.Kaluza(at)monash.edu
*%
*% Development Team :
*%  http://www.underworldproject.org/aboutus.html
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "View.h"

View::View(std::string title, bool stereo_flag, float xf, float yf, float nearc, float farc)
{
   // default view params
   near_clip = nearc;       //Near clip plane
   far_clip = farc;       //Far clip plane
   eye_sep_ratio = 0.03f;  //Eye separation ratio to focal length
   fov = 45.0f; //60.0     //Field of view - important to adjust for stereo viewing
   focal_length = focal_length_adj = 0.0; //Stereo zero parallex distance adjustment
   scene_shift = 0.0;      //Stereo projection shift
   rotated = sort = false;

   model_size = 0.0;       //Scalar magnitude of model dimensions
   width = 0;              //Viewport width
   height = 0;             //Viewport height

   x = xf;
   y = yf;
   w = h = 1.0f;

   orientation = RIGHT_HANDED;

   stereo = stereo_flag; 
   autozoom = false;
   redraw = true;
   filtered = true;
   scaled = true;
   initialised = false;

   for (int i=0; i<3; i++)
   {
      rotate_centre[i]     = 0.0;      // Centre of rotation
      focal_point[i]       = FLT_MIN;  // Focal point
      default_focus[i]     = FLT_MIN;  // Default focal point
      model_trans[i]       = 0.0;
      model_trans_lag[i]   = 0.0;
      scale[i]             = 1.0;
      min[i]               = 0.0;
      max[i]               = 0.0;
   }
   rotation.identity();
   rotation_lag.identity();
   use_inertia = false;

   //View properties can only be set if exist, so set all defaults
   properties["title"] = title;
   properties["zoomstep"] = -1;
   properties["margin"] = 32;
   properties["rulers"] = false;
   properties["border"] = 1;
   properties["fillborder"] = false;
   properties["bordercolour"] = Colour_ToJson(LUC_GREY);
   properties["axis"] = true;
   properties["axislength"] = 0.1;
   properties["timestep"] = false;
   properties["antialias"] = true; //Should be global
   properties["shift"] = 0;

   is3d = true;
}

void View::setProperties(std::string props)
{
   jsonParseProperties(props, properties);
}

void View::addObject(DrawingObject* obj)
{      
   objects.push_back(obj);
   debug_print("Object %d '%s' added to viewport '%s'\n", obj->id, obj->name.c_str(), properties["title"].ToString().c_str());
}

bool View::hasObject(DrawingObject* obj)
{      
   for (unsigned int i=0; i < objects.size(); i++)
      if (objects[i] == obj) return true;
   return false;
}

//Setup camera/modelview to view a model with enclosing cuboid defined by min[X,Y,Z] and max[X,Y,Z]
bool View::init(bool force, float* newmin, float* newmax)
{
   if (!newmin) newmin = min;
   if (!newmax) newmax = max;

   for (int i=0; i<3; i++)
   {
      //Invalid bounds! Skip
      if (!ISFINITE(newmin[i]) || !ISFINITE(newmax[i])) return false;

      //If bounds changed, reset focal point to default
      if (min[i] != newmin[i] || max[i] != newmax[i]) focal_point[i] = default_focus[i];

      min[i] = newmin[i];
      max[i] = newmax[i];
      dims[i] = fabs(max[i] - min[i]);

      //Fallback focal point and rotation centre = model bounding box centre point
      if (focal_point[i] == FLT_MIN) focal_point[i] = min[i] + dims[i] / 2.0f;
      rotate_centre[i] = focal_point[i];
   }

   //Save magnitude of dimensions
   model_size = sqrt(dotProduct(dims,dims));
   if (model_size == 0 || !ISFINITE(model_size)) return false;

   //Adjust clipping planes
   if (near_clip == 0 || far_clip == 0)
   {
      //NOTE: Too much clip plane range can lead to depth buffer precision problems
      //Near clip should be as far away as possible as greater precision reserved for near
      float min_dist = model_size / 10.0;  //Estimate of min dist between viewer and geometry
      float aspectRatio = 1.33;
      if (width && height)
         aspectRatio = width / (float)height;
      near_clip = min_dist / sqrt(1 + pow(tan(0.5*M_PI*fov/180), 2) * (pow(aspectRatio, 2) + 1));
      //near_clip = model_size / 5.0;   
      far_clip = model_size * 20.0;
   }

   if (max[2] > min[2]) is3d = true; else is3d = false;
   debug_print("Model size %f dims: %f,%f,%f - %f,%f,%f (scale %f,%f,%f) 3d? %s\n", 
           model_size, min[0], min[1], min[2], max[0], max[1], max[2], scale[0], scale[1], scale[2], (is3d ? "yes" : "no"));

   //Auto-cam etc should only be procesed once...and only when viewport size has been set
   if ((force || !initialised) && width > 0 && height > 0)
   {
      //Only flag correctly initialised after focal point set (have model bounds)
      initialised = true;

      projection(EYE_CENTRE);

      //Default translation by model size
      if (model_trans[2] == 0)
         model_trans[2] = model_trans_lag[2] = -model_size;

      // Initial zoom to fit
      if (properties["zoomstep"].ToInt(-1) == 0) zoomToFit();

      debug_print("   Auto cam: (Viewport %d x %d) (Model: %f x %f x %f)\n", width, height, dims[0], dims[1], dims[2]);
      debug_print("   Looking At: %f,%f,%f\n", focal_point[0], focal_point[1], focal_point[2]);
      debug_print("   Rotate Origin: %f,%f,%f\n", rotate_centre[0], rotate_centre[1], rotate_centre[2]);
      debug_print("   Clip planes: near %f far %f. Focal length %f Eye separation ratio: %f\n", near_clip, far_clip, focal_length, eye_sep_ratio);
      debug_print("   Translate: %f,%f,%f\n", model_trans[0], model_trans[1], model_trans[2]);

      //Apply changes to view (and reset inertia)
      inertia(false);
      apply();
   }

   //inertia(false); //Reset inertia lag

   return true;
}

void View::getMinMaxDistance(float* mindist, float* maxdist)
{
   //Save min/max distance
   float vert[3], dist;
   glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
   *maxdist = -HUGE_VAL;
   *mindist = HUGE_VAL; 
   for (int i=0; i<2; i++)
   {
      vert[0] = i==0 ? min[0] : max[0];
      for (int j=0; j<2; j++)
      {
         vert[1] = j==0 ? min[1] : max[1];
         for (int k=0; k<2; k++)
         {
            vert[2] = k==0 ? min[2] : max[2];
            dist = eyeDistance(modelView, vert);
            if (dist < *mindist) *mindist = dist;
            if (dist > *maxdist) *maxdist = dist;
         }
      }
   }
   if (*maxdist == *mindist) *maxdist += 0.0000001;
   //printf("DISTANCE MIN %f MAX %f\n", *mindist, *maxdist);
}

std::string View::rotateString()
{
   //Convert rotation to command string...
   std::ostringstream ss;
   ss << "rotation " << rotation[0] << " " << rotation[1] << " " << rotation[2] << " " << rotation[3];
   return ss.str();
}

std::string View::translateString()
{
   //Convert translation to command string...
   std::ostringstream ss;
   ss << "translation " << model_trans[0] << " " << model_trans[1] << " " << model_trans[2];
   return ss.str();
}

void View::getCamera(float rotate[4], float translate[3], float focus[3])
{
   //Convert translation & rotation to array...
   //rotation.toEuler(rotate[0], rotate[1], rotate[2]);
   for (int i=0; i<4; i++) rotate[i] = rotation[i];
   memcpy(translate, model_trans, sizeof(float) * 3);
   memcpy(focus, focal_point, sizeof(float) * 3);
}

std::string View::adjustStereo(float aperture, float focal_len, float eye_sep)
{
   fov += aperture;
   if (fov < 30) fov = 30;
   if (fov > 70) fov = 70;
   focal_length_adj += focal_len;
   eye_sep_ratio += eye_sep;
   //if (eye_sep_ratio < 0) eye_sep_ratio = 0;
   debug_print("STEREO: Aperture %f Focal Length Adj %f Eye Separation %f\n", fov, focal_length_adj, eye_sep_ratio);
   std::ostringstream ss;
   if (aperture) ss << "aperture " << aperture;
   if (focal_len) ss << "focallength " << focal_len;
   if (eye_sep) ss << "eyeseparation " << eye_sep;
   return ss.str();
}

void View::focus(float x, float y, float z, float aperture, bool setdefault)
{
   focal_point[0] = rotate_centre[0] = x;
   focal_point[1] = rotate_centre[1] = y;
   focal_point[2] = rotate_centre[2] = z;
   if (aperture > 0)
     fov = aperture;
   //reset(); //reset view
   //Set as the default
   if (setdefault)
      memcpy(default_focus, focal_point, sizeof(float)*3);
}

void View::setTranslation(float x, float y, float z)
{
   model_trans[0] = x;
   model_trans[1] = y;
   model_trans[2] = z;
}

void View::translate(float x, float y, float z)
{
   model_trans[0] += x;
   model_trans[1] += y;
   model_trans[2] += z;
   inertia();
}

void View::setRotation(float x, float y, float z, float w)
{
   rotation = Quaternion(x, y, z, w);
}

void View::rotate(float degrees, Vec3d axis)
{
   Quaternion nrot;
   nrot.fromAxisAngle(axis, degrees);
   nrot.normalise();
   rotation = nrot * rotation;
   inertia();
}

void View::rotate(float degreesX, float degreesY, float degreesZ)
{
   //std::cerr << "Rotate : " << degreesX << "," << degreesY << "," << degreesZ << std::endl;
   rotate(degreesZ, Vec3d(0,0,1));
   rotate(degreesY, Vec3d(0,1,0));
   rotate(degreesX, Vec3d(1,0,0));
}

void View::setScale(float x, float y, float z)
{
   scale[0] *= x;
   scale[1] *= y;
   scale[2] *= z;
}

std::string View::zoom(float factor)
{
   float adj = factor * model_size;
   model_trans_lag[2] = model_trans[2] += adj;
   std::ostringstream ss;
   ss << "translate z " << adj;
   return ss.str();
}

std::string View::zoomClip(float factor)
{
   near_clip += factor * model_size;
   if (near_clip < model_size * 0.001) near_clip = model_size * 0.001; //Bounds check
   //debug_print("Near clip = %f\n", near_clip);
   std::ostringstream ss;
   ss << "nearclip " << near_clip;
   return ss.str();
}

void View::reset()
{
   for (int i=0; i<3; i++)
   {
      model_trans[i] = model_trans_lag[i] = 0;
      //Default focal point and rotation centre = model bounding box centre point
      //if (focal_point[i] == FLT_MIN)
      //   focal_point[i] = min[i] + dims[i] / 2.0f;
      rotate_centre[i] = focal_point[i];
   }
   rotation.identity();
   rotation_lag.identity();
   rotated = true;   //Flag rotation
}

void View::print()
{
   float xrot, yrot, zrot;
   rotation_lag.toEuler(xrot, yrot, zrot);
   printf("Viewport %d,%d %d x %d\n", xpos, ypos, width, height);
   printf("Clip planes: near %f far %f\n", near_clip, far_clip);
   printf("Model size %f dims: %f,%f,%f - %f,%f,%f (scale %f,%f,%f)\n", 
           model_size, min[0], min[1], min[2], max[0], max[1], max[2], scale[0], scale[1], scale[2]);
   printf("Focal Point %f,%f,%f\n", focal_point[0], focal_point[1], focal_point[2]);
#ifndef USE_OMEGALIB
   printf("<struct name=\"Camera\">\n");
   printf("   <param name=\"Type\">lucCamera</param>\n");
   printf("   <param name=\"focalPointX\">%f</param>\n", focal_point[0]);
   printf("   <param name=\"focalPointY\">%f</param>\n", focal_point[1]);
   printf("   <param name=\"focalPointZ\">%f</param>\n", focal_point[2]);
   printf("   <param name=\"translateX\">%f</param>\n", model_trans_lag[0]);
   printf("   <param name=\"translateY\">%f</param>\n", model_trans_lag[1]);
   printf("   <param name=\"translateZ\">%f</param>\n", model_trans_lag[2]);
   printf("   <param name=\"rotateX\">%.1f</param>\n", -xrot);
   printf("   <param name=\"rotateY\">%.1f</param>\n", -yrot);
   printf("   <param name=\"rotateZ\">%.1f</param>\n", -zrot);
   printf("   <param name=\"aperture\">%.1f</param>\n", fov);
   printf("   <param name=\"CoordinateSystem\">%s</param>\n", (orientation == RIGHT_HANDED ? "RightHanded" : "LeftHanded"));
   printf("</struct>\n");
   printf("------------------------------\n");
#endif
   printf("%s\n", translateString().c_str());
   printf("%s\n", rotateString().c_str());
   printf("------------------------------\n");
}

//Absolute viewport
void View::port(int x, int y, int width, int height)
{
   this->width = width;
   this->height = height;
   xpos = x;
   ypos = y;

   glViewport(x, y, width, height);
   glScissor(x, y, width, height);
   //debug_print("Viewport set at %d,%d %d x %d\n", x, y, width, height);
}

//Viewport calculated using saved position & dim values
void View::port(int win_width, int win_height)
{
   width = ceil(w * win_width);
   height = ceil(h * win_height);

   xpos = ceil(x * win_width);
   ypos = ceil((1.0 - y - h) * win_height); //Flip y-coord for gl

   //Ensure full window area filled
   if (abs(win_width - (xpos + width)) < 5)
      width = win_width - xpos;
   if (abs(win_height - (ypos + height)) < 5)
      height = win_height - ypos;

   glViewport(xpos, ypos, width, height);
   glScissor(xpos, ypos, width, height);
   //debug_print("-------Viewport (%f,%f %fx%f) set at %d,%d %d x %d\n", x,y,w,h, xpos, ypos, width, height);
}

bool View::hasPixel(int x, int y)
{
   if (x < xpos) return false;
   if (y < ypos) return false;
   if (x > xpos + width) return false;
   if (y > ypos + height) return false;
   return true;
}

void View::projection(int eye)
{
   if (!initialised) return;
   float aspectRatio = width / (float)height;
   
   // Perspective viewing frustum parameters 
   float left, right, top, bottom;
   float eye_separation, frustum_shift;

   //This is zero parallax distance, objects closer than this will appear in front of the screen,
   //default is to set to distance to model front edge...
   float focal_length = fabs(model_trans_lag[2]) - model_size * 0.5; //3/4 of model size - distance from eye to model centre
   focal_length += focal_length_adj;   //Apply user adjustment (default 0)
   if (focal_length < 0) focal_length = 0.1;

   //Calculate eye separation based on focal length
   eye_separation = focal_length * eye_sep_ratio;

   // Build the viewing frustum 
   // Top of frustum calculated from field of view (aperture) and near clipping plane, camera->aperture = fov in degrees 
   top = tan(0.5f * DEG2RAD * fov) * near_clip;
   bottom = -top;
   // Account for aspect ratio (width/height) to get right edge of frustum 
   right = aspectRatio * top;
   left = -right;

   //Shift frustum to the left/right to account for right/left eye viewpoint
   frustum_shift = eye * 0.5 * eye_separation * fabs(near_clip / focal_length);  //Mutiply by eye (-1 left, 0 none, 1 right)
   //Viewport eye shift in pixels => for raycasting shader
   eye_shift = eye * eye_sep_ratio * height * 0.6 / tan(DEG2RAD * fov);

   // In Stereo, View vector for each camera is parallel 
   // Need to adjust eye position and focal point to left/right to account for this... 
   //Shift model by half of eye-separation in opposite direction of eye (equivalent to camera shift in same direction as eye)
   scene_shift = eye * eye_separation * -0.5f; 

   if (eye) debug_print("STEREO %s: focalLen: %f eyeSep: %f frustum_shift: %f, scene_shift: %f eye_shift %f\n", (eye < 0 ? "LEFT (RED)  " : "RIGHT (BLUE)"),
                        focal_length, eye_separation, frustum_shift, scene_shift, eye_shift);
   //debug_print(" Ratio %f Left %f Right %f Top %f Bottom %f Near %f Far %f\n",
   //         aspectRatio, left, right, bottom, top, near_clip, far_clip);

   // Set up our projection transform
   glMatrixMode(GL_PROJECTION);  
   glLoadIdentity();
   glFrustum(left - frustum_shift, right - frustum_shift, bottom, top, near_clip, far_clip);

   // Return to model view
   glMatrixMode(GL_MODELVIEW);
   GL_Error_Check;
}

void View::apply(bool use_fp)
{
   if (!use_inertia) inertia(false);
   GL_Error_Check;
   // Setup view transforms
   glMatrixMode(GL_MODELVIEW);
#ifndef USE_OMEGALIB
   glLoadIdentity();
   GL_Error_Check;

   // Translate to cancel stereo parallax
   glTranslatef(scene_shift, 0.0, 0.0);
   GL_Error_Check;

   // Translate model away from eye by camera zoom/pan translation 
   //debug_print("APPLYING VIEW '%s': trans %f,%f,%f\n", title.c_str(), model_trans[0], model_trans[1], model_trans[2]);
   glTranslatef(model_trans_lag[0]*scale[0], model_trans_lag[1]*scale[0], model_trans_lag[2]*scale[0]);
   GL_Error_Check;
#endif

   // Adjust centre of rotation, default is same as focal point so this does nothing...
   float adjust[3] = {(focal_point[0]-rotate_centre[0])*scale[0], (focal_point[1]-rotate_centre[1])*scale[1], (focal_point[2]-rotate_centre[2])*scale[2]};
   if (use_fp) glTranslatef(-adjust[0], -adjust[1], -adjust[2]);
   GL_Error_Check;

   // rotate model 
   rotation_lag.apply();
   GL_Error_Check;

   // Adjust back for rotation centre
   if (use_fp) glTranslatef(adjust[0], adjust[1], adjust[2]);
   GL_Error_Check;

   // Translate to align eye with model centre - view focal point
   //glTranslatef(-rotate_centre[0], -rotate_centre[1], -rotate_centre[2]);
   if (use_fp) glTranslatef(-focal_point[0]*scale[0], -focal_point[1]*scale[1], orientation * -focal_point[2]*scale[2]);
   GL_Error_Check;

   // Switch coordinate system if applicable
   glScalef(1.0, 1.0, 1.0 * orientation); 
   GL_Error_Check;

   // Apply scaling factors
   if (scale[0] != 1.0 || scale[1] != 1.0 || scale[2] != 1.0)
   {
      glScalef(scale[0], scale[1], scale[2]);
      // Enable automatic rescaling of normal vectors when scaling is turned on
      //glEnable(GL_RESCALE_NORMAL);
      glEnable(GL_NORMALIZE);
   }
   GL_Error_Check;

   // Set default polygon front faces
   if (orientation == RIGHT_HANDED)
      glFrontFace(GL_CCW);
   else
      glFrontFace(GL_CW);
   GL_Error_Check;
}

bool View::scaleSwitch()
{
   static float save_scale[3];
   if (scaled)
   { 
      save_scale[0] = scale[0];
      save_scale[1] = scale[1];
      save_scale[2] = scale[2];
      scale[0] = 1.0;
      scale[1] = 1.0;
      scale[2] = 1.0;
      scaled = false;
   }
   else
   {
      scale[0] = save_scale[0];
      scale[1] = save_scale[1];
      scale[2] = save_scale[2];
      scaled = true;
   }
   return scaled;
}

int View::direction()
{
   //Returns 1 or -1 multiplier representing direction of viewer
   return model_trans[2] > 0 ? -1 : 1; 
}

void View::setCoordSystem(int orientation)
{
   this->orientation = orientation;
}

int View::switchCoordSystem()
{
   if (orientation == RIGHT_HANDED)
      setCoordSystem(LEFT_HANDED);
   else
      setCoordSystem(RIGHT_HANDED);
   rotated = true;   //Flag rotation
   return orientation;
}

void View::inertia(bool on)
{
   if (!on) 
   {
      memcpy(model_trans_lag, model_trans, sizeof(float) * 3);
      rotation_lag = Quaternion(rotation);
      return;
   }

   // Apply inertia lag to translations and rotations
   for (int c = 0; c < 3; ++c)
      model_trans_lag[c] += (model_trans[c] - model_trans_lag[c]) * 0.1;
   rotation_lag.x += (rotation.x - rotation_lag.x) * 0.05;
   rotation_lag.y += (rotation.y - rotation_lag.y) * 0.05;
   rotation_lag.z += (rotation.z - rotation_lag.z) * 0.05;
   rotation_lag.w += (rotation.w - rotation_lag.w) * 0.05;
   //After linear interpolation between quaternions, need to normalise
   rotation_lag.normalise();
}

#define ADJUST 0.444444
void View::zoomToFit(int margin)
{
   if (margin < 0) margin = properties["margin"].ToInt(32);

   // The bounding box of model 
   GLfloat rect3d[8][3] = {{min[0], min[1], min[2]},
                           {min[0], min[1], max[2]},
                           {min[0], max[1], min[2]},
                           {min[0], max[1], max[2]},
                           {max[0], min[1], min[2]},
                           {max[0], min[1], max[2]},
                           {max[0], max[1], min[2]},
                           {max[0], max[1], max[2]}};

   //3d rect vertices, object and window coords
   GLfloat modelView[16];
   GLfloat projection[16];
   int viewport[4];
   int count = 0;
   double error = 1, scale2d, adjust = ADJUST;
   glGetIntegerv(GL_VIEWPORT, viewport);
   glGetFloatv(GL_PROJECTION_MATRIX, projection);

   // Continue scaling adjustments until within tolerance
   while (count < 30 && fabs(error) > 0.005) 
   {    
      float min_x = 10000, min_y = 10000, max_x = -10000, max_y = -10000;
      GLfloat win3d[8][3];
      int i;

      // Set camera and get modelview matrix defined by viewpoint
      apply();
      glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
      for (i = 0; i < 8; i++) {
         gluProjectf(rect3d[i][0], rect3d[i][1], rect3d[i][2], 
                     modelView, projection, viewport, &win3d[i][0]);
         //Save max/min x and y - define bounding 2d rectangle
         if (win3d[i][0] < min_x) min_x = win3d[i][0]; 
         if (win3d[i][0] > max_x) max_x = win3d[i][0];
         if (win3d[i][1] < min_y) min_y = win3d[i][1];
         if (win3d[i][1] > max_y) max_y = win3d[i][1];
         //printf("  %d) rect %f,%f,%f win %f,%f,%f\n", i, rect3d[i][0], rect3d[i][1], rect3d[i][2], win3d[i][0], win3d[i][1], win3d[i][2]);
      }

     // Calculate min bounding rectangle centered in viewport (with margins)
     {
      // Offset to viewport edges as gluProject returns window coords
      min_x -= (viewport[0] + margin);
      min_y -= (viewport[1] + margin);
      max_x -= (viewport[0] + margin);
      max_y -= (viewport[1] + margin);

      double width = viewport[2] - margin*2;
      double height = viewport[3] - margin*2;
      double centrex = width/2.0;
      double centrey = height/2.0;

      double dminx = fabs(min_x - centrex);
      double dmaxx = fabs(max_x - centrex);
      double new_max_x = centrex + (dminx > dmaxx ? dminx : dmaxx);
      double new_min_x = width - new_max_x;

      double dminy = fabs(min_y - centrey);
      double dmaxy = fabs(max_y - centrey);
      double new_max_y = centrey + (dminy > dmaxy ? dminy : dmaxy);
      double new_min_y = height - new_max_y;

      double xscale = 1.0, yscale = 1.0; 
      float rwidth = (new_max_x - new_min_x);
      float rheight = (new_max_y - new_min_y);

      xscale = width / rwidth;
      yscale = height / rheight;
      if (xscale < yscale) scale2d = xscale; else scale2d = yscale;
     }

      // debug_print("BB new_min_x %f new_max_x %f === ", new_min_x, new_max_x);
      //   debug_print("Bounding rect: %f,%f - %f,%f === ",  min_x, min_y, max_x, max_y);
      //   debug_print(" Min rect: 0,0 - %f,%f\n", max_x - min_x, max_y - min_y);

      //Self-adjusting: did we overshoot aim? - compare last error to current 2d scale
      if (count > 0) {
         if ((error > 0 && scale2d < 1.0) ||
             (error < 0 && scale2d > 1.0))
         {
            adjust *= 0.75;
            if (adjust > ADJUST) adjust = ADJUST;
         }
         else {
            adjust *= 1.5;
            if (adjust < ADJUST) adjust = ADJUST;
         }
      }
    
      // Try to guess the best value adjustment using error margin
      //If error (devergence from 1.0) is > 0, scale down, < 0 scale up
      error = (scale2d - 1.0) / scale2d;

      //Adjust zoom translation by error * adjust (0.5 default)
      //float oldz = model_trans[2];
      model_trans[2] -= (model_trans[2] * error * adjust);
      //if (count > 4) {
      //   debug_print("[%d iterations] ... 2D Scaling factor %f ", count, scale2d);
      //   debug_print(" --> error: %f (adjust %f) zoom from %f to %f\n", error, adjust, oldz, model_trans[2]);
      //}
      count++;
      model_trans_lag[2] = model_trans[2];
   }
}

void View::drawRuler(float start[3], float end[3], float labelmin, float labelmax, int ticks)
{
   // Draw rulers with optional tick marks
   glPushAttrib(GL_ENABLE_BIT);
   glDisable(GL_LIGHTING);
   glDisable(GL_LINE_SMOOTH);
   glLineWidth(1.5f);

   float vec[3];
   float length;
   float rangle;

   vectorSubtract(vec, end, start);

            vec[0] *= scale[0];
            vec[1] *= scale[1];
            vec[2] *= scale[2];
            start[0] *= scale[0];
            start[1] *= scale[1];
            start[2] *= scale[2];

   // Length of the drawn vector = vector magnitude
   length = sqrt(dotProduct(vec,vec));

   glPushMatrix();
   // Undo any scaling factor
   if (scale[0] != 1.0 || scale[1] != 1.0 || scale[2] != 1.0)
      glScalef(1.0/scale[0], 1.0/scale[1], 1.0/scale[2]);


   // Translate to start of ruler
   glTranslatef(start[0], start[1], start[2]);

   // Rotate to orient ruler
   //...Want to align our z-axis to point along vector:
   // axis of rotation = (z x vec)
   // cosine of angle between vector and z-axis = (z . vec) / |z|.|vec|
   // Normalise vector first so OSMesa doesn't have a fit when given a tiny vector as a rotation axis
   vectorNormalise(vec); 
   //Angle of rotation = acos(vec . [0,0,1]) = acos(vec[2])
   rangle = RAD2DEG * acos(vec[2]);
   //Axis of rotation = vec x [0,0,1] = -vec[1],vec[0],0
   glRotatef(rangle, -vec[1], vec[0], 0);

   for (int i = 0; i < ticks; i++)
   {
      // Get tick value
      float scaledPos = i / (float)(ticks-1);
      // Calculate pixel position
      float pos = length * scaledPos;
      float height = -0.01 * model_size;

      // Draws the tick
      glBegin(GL_LINES);
      glVertex3f(0, 0, pos);
      glVertex3f(0, height, pos);
      glEnd();

      //Draw a label
      char label[20];
      float inc = (labelmax - labelmin) / (float)(ticks-1);
      sprintf(label, "%-10.3f", labelmin + i * inc);
      Print3dBillboard(0, 2.0*height, pos, 0.03*model_size, label);
   }

   //Draw ruler line
   glBegin(GL_LINES);
      glVertex3f(0,0,0);
      glVertex3f(0,0,length);
   glEnd();

   glPopMatrix();
   glLineWidth(1.0f);
   glPopAttrib();
}

void View::drawRulers()
{
   bool rulers = properties["rulers"].ToBool(false);
   if (!rulers) return;
   //Axis rulers
   float shift[3] = {0.01f/scale[0] * model_size, 0.01f/scale[1] * model_size, 0.01f/scale[2] * model_size};
   {
      float sta[3] = {min[0], min[1]-shift[1], max[2]+shift[2]};
      float end[3] = {max[0], min[1]-shift[1], max[2]+shift[2]};
      drawRuler(sta, end, min[0], max[0], 5);
   }
   {
      float sta[3] = {min[0]-shift[0], min[1], max[2]+shift[2]};
      float end[3] = {min[0]-shift[0], max[1], max[2]+shift[2]};
      drawRuler(sta, end, min[1], max[1], 5);
   }
   {
      float sta[3] = {min[0]-shift[0], min[1]-shift[1], min[2]};
      float end[3] = {min[0]-shift[0], min[1]-shift[1], max[2]};
      drawRuler(sta, end, min[2], max[2], 5);
   }
}

void View::drawBorder()
{
   int border = properties["border"].ToInt(1);
   if (border == 0) return;
   bool filled = properties["fillborder"].ToBool(false);
   Colour borderColour = Colour_FromJson(properties, "bordercolour", 127, 127, 127, 255);

   // Draw model bounding box with optional filled background surface
   float adj = 0.0001 * model_size;
   float minvert[3] = {min[0]-adj, min[1]-adj, min[2]-adj};
   float maxvert[3] = {max[0]+adj, max[1]+adj, max[2]+adj};
   glColor4ubv(borderColour.rgba);
   //Min/max swapped to draw inverted box, see through to back walls
   drawCuboid(maxvert, minvert, filled, border);
   GL_Error_Check;
}

void View::drawAxis() 
{
   bool axis = properties["axis"].ToBool(true);
   float axislen = properties["axislength"].ToFloat(0.1);

   if (!axis) return;
   float length = axislen;
   float headsize = 8.0;   //8 x radius (r = 0.01 * length)
   float LH = length * 0.1;
   float aspectRatio = width / (float)height;

   glPushAttrib(GL_ENABLE_BIT);
   glEnable(GL_LIGHTING);
   //Clear depth buffer
   glClear(GL_DEPTH_BUFFER_BIT);

   //Setup the projection
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   // Build the viewing frustum - fixed near/far
   float nearc = 0.01, farc = 10.0, left, right, top, bottom;
   top = tan(0.5f * DEG2RAD * 45) * nearc;
   right = aspectRatio * top;
   glFrustum(-right, right, -top, top, nearc, farc);
   //Modelview (rotation only)
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   //Position to the bottom left
   glTranslatef(-0.3 * aspectRatio, -0.3, -1.0);
   //Apply model rotation
   rotation_lag.apply();
   GL_Error_Check;
   // Switch coordinate system if applicable
   glScalef(1.0, 1.0, 1.0 * orientation); 

   float Xpos[3] = {length/2, 0, 0};
   float Ypos[3] = {0, length/2, 0};
   float Zpos[3] = {0, 0, length/2};

   glColor3f(1,0,0);
   {
      float vector[3] = {1.0, 0.0, 0.0};
      drawVector3d_( Xpos, vector, length, 0.01*length, headsize, 16.0f, NULL, NULL );
   }
      
   glColor3f(0,1,0);
   {
      float vector[3] = {0.0, 1.0, 0.0};
      drawVector3d_( Ypos, vector, length, 0.01*length, headsize, 16.0f, NULL, NULL );
   }

   glColor3f(0,0,1);
   if (is3d)
   {
      float vector[3] = {0.0, 0.0, 1.0};
      drawVector3d_( Zpos, vector, length, 0.01*length, headsize, 16.0f, NULL, NULL );
   }

   //Labels
   glDisable(GL_LIGHTING);
   glDisable(GL_DEPTH_TEST);

   Print3dBillboard(Xpos[0],    Xpos[1]-LH, Xpos[2], length, "X");
   Print3dBillboard(Ypos[0]-LH, Ypos[1],    Ypos[2], length, "Y");
   if (is3d)
      Print3dBillboard(Zpos[0]-LH, Zpos[1]-LH, Zpos[2], length, "Z");

   glPopAttrib();

   //Restore
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   GL_Error_Check;
}

void View::drawOverlay(Colour& colour, std::string timestamp)
{
#ifdef PDF_CAPTURE
   return;   //Skip overlay
#endif
   //Draw axis & rulers 
   drawAxis();
   drawRulers();

   //2D overlay objects
   Viewport2d(width, height);
   int w = width;
   int h = height;

   //Colour bars
   lucSetFontCharset(FONT_SMALL); //Bitmap fonts
   glDisable(GL_MULTISAMPLE);
   GL_Error_Check;
   float last_y = 0;
   for (unsigned int i=0; i<objects.size(); i++)
   {
      //Only when flagged as colour bar
      if (!objects[i] || !objects[i]->properties["colourbar"].ToBool(false) || !objects[i]->visible) continue;

      int length = w * objects[i]->properties["lengthfactor"].ToFloat(0.8); 
      int bar_height = objects[i]->properties["height"].ToInt(10);
      int startx = (w - length) / 2;  //Need an X pos / margin property
      int starty = last_y + objects[i]->properties["margin"].ToInt(16);
      //last_y = starty;   //Auto-increase y margin?

      objects[i]->colourMaps[lucColourValueData]->draw(objects[i]->properties, startx, starty, length, bar_height, colour);
      GL_Error_Check;
   } 

   GL_Error_Check;
   glEnable(GL_MULTISAMPLE);

   //Title
   if (properties.HasKey("title"))
   {
      const char* title = properties["title"].ToString().c_str();
      lucSetFontCharset(FONT_DEFAULT);  //Bitmap fonts
      Print(0.5 * (w - PrintWidth(title, 0.6)), h - 20, 0.6, title);
   }

   //Timestep (with scaling applied)
   if (properties["timestep"].ToBool(false))
   {
      // Use scaling coeff and units to display time
      lucSetFontCharset(FONT_SMALL);
      lucPrint(1, height - 9, timestamp.c_str());
      lucSetFontCharset(FONT_DEFAULT);
   }

   GL_Error_Check;
   //Restore 3d
   Viewport2d(0, 0);
   GL_Error_Check;
}


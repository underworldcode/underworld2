#ifndef Shaders__
#define Shaders__

#include "GraphicsUtil.h"

class Shader
{
  private:
   GLuint shaders[2];
   void print_log(const char *action, GLuint obj);

  public:
   GLuint program;
   const char* gl_version;
   bool supported;

   Shader();
   Shader(const char *fshader);
   Shader(const char *vshader, const char *fshader);
   void init(std::string vsrc, std::string fsrc);

   bool version();
   std::string read_file(const char *fname);
   bool compile(const char *src, GLuint type);
   bool build();
   void use();
   void loadUniforms(const char** names, int count);
   void loadAttribs(const char** names, int count);
   void setUniform(const char* name, int value);
   void setUniform(const char* name, float value);

   std::map<std::string, GLint> uniforms;
   std::map<std::string, GLint> attribs;

   static const char* path;
};

#endif //Shaders__

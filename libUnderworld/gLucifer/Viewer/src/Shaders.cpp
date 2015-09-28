#include "Shaders.h"

#ifndef SHADER_PATH
const char* Shader::path = NULL;
#else
const char* Shader::path = SHADER_PATH;
#endif

//Default shaders
const char *vertexShader = STRINGIFY(
void main(void)
{
   gl_TexCoord[0] = gl_MultiTexCoord0;
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
   gl_FrontColor = gl_Color;
}
);

const char *fragmentShader = STRINGIFY(
void main(void)
{
   gl_FragColor = gl_Color;
}
);

Shader::Shader()
{
   //Use both default shaders
   init("", "");
}

Shader::Shader(const char *fshader)
{
   //This constructor for a fragment shader only
   std::string fsrc = read_file(fshader);
   init("", fsrc);
}

Shader::Shader(const char *vshader, const char *fshader)
{
   //This constructor is for a single vertex and/or fragment shader only
   std::string vsrc = read_file(vshader);
   std::string fsrc = read_file(fshader);
   init(vsrc, fsrc);
}

void Shader::init(std::string vsrc, std::string fsrc)
{
   program = shaders[0] = shaders[1] = 0;
   supported = version();
   if (!supported) return;
   //Default shaders
   if (fsrc.length() == 0) fsrc = std::string(fragmentShader);
   if (vsrc.length() == 0) vsrc = std::string(vertexShader);
   //Attempts to load and build shader programs
   if (compile(vsrc.c_str(), GL_VERTEX_SHADER) && 
       compile(fsrc.c_str(), GL_FRAGMENT_SHADER)) build();
}

bool Shader::version()
{
   gl_version = (const char*)glGetString(GL_VERSION);
   if (!gl_version) return false;
#ifndef _WIN32
#ifndef GL_VERSION_2_1
   debug_print("%s does not support OpenGL 2.1 shaders, please upgrade your OpenGL drivers!\n", gl_version);
   program = 0;
   return false;
#else
   if (strstr(gl_version, "Mesa 7.10.2"))
   {
      debug_print("FATAL SHADER ERROR: %s has a bug in glCompileShader, please upgrade your Mesa library!\n", gl_version);
      program = 0;
      return false;
   }
   //Check function pointer not null
   if (!glCreateShader || !glGetUniformLocation)
   {
      debug_print("%s does not support OpenGL 2.1 shaders, please upgrade your OpenGL drivers!\n", gl_version);
      program = 0;
      return false;
   }
#endif
#endif
   return true;
}

//Read a fragment or vertex shader from a file into a shader object
std::string Shader::read_file(const char *fname)
{
   char filepath[2048] = "";
   if (!fname) return std::string("");

   if (Shader::path) strcpy(filepath, Shader::path);
   strcat(filepath, fname);
   debug_print("Shader loading: %s\n", filepath);

   std::ifstream ifs(filepath);
   std::stringstream buffer;
   if (ifs.is_open())
      buffer << ifs.rdbuf();
   else
      std::cerr << "Error opening shader: " << filepath << std::endl;
   return buffer.str();
}

bool Shader::compile(const char *src, GLuint type)
{
   GLint compiled;
   int idx = (type == GL_VERTEX_SHADER ? 0 : 1);
   GLuint shader = glCreateShader(type);
   glShaderSource(shader, 1, &src, NULL);
   glCompileShader(shader);
   glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
   if (!compiled)
   {
      print_log("Shader Compile", shader);
   }
   else
   {
      if (shaders[idx]) glDeleteShader(shaders[idx]);
      shaders[idx] = shader;
   }
   GL_Error_Check;
   return compiled;
}

//Attach and link shaders into program
bool Shader::build() 
{
   GLint linked;
   if (!supported) return false;

   if (program) glDeleteProgram(program);
   program = glCreateProgram();
   assert(glIsProgram(program));

   if (glIsShader(shaders[0]))
      glAttachShader(program, shaders[0]);
   if (glIsShader(shaders[1]))
      glAttachShader(program, shaders[1]);

   glLinkProgram(program);
   glGetProgramiv(program, GL_LINK_STATUS, &linked);
   if(!linked)
   {
      print_log("Shader Link", program);
      return false;
   }
   return true;
}

//Report shader compile/link errors
void Shader::print_log(const char *action, GLuint obj)
{
   if (!supported) return;
   int infologLength = 0;
   int maxLength;
   
   if(glIsShader(obj))
      glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
   else
      glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
         
   char* infoLog = new char[maxLength];
 
   if (glIsShader(obj))
      glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
   else
      glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);
 
   if (infologLength > 0)
      debug_print("%s:\n%s\n", action, infoLog);

   delete[] infoLog;
}

void Shader::use() 
{
   if (!supported || !program) return;
   glUseProgram(program);
   GL_Error_Check;
}

void Shader::loadUniforms(const char** names, int count)
{
   if (!supported || !program) return;
   for (int i=0; i<count; i++)
   {
      GLint loc = glGetUniformLocation(program, names[i]);
      if (loc < 0)
        debug_print("Uniform '%s' not found\n", names[i]);
      uniforms[names[i]] = loc;
   }
}

void Shader::loadAttribs(const char** names, int count)
{
   if (!supported || !program) return;
   for (int i=0; i<count; i++)
   {
      GLint loc = glGetAttribLocation(program, names[i]);
      if (loc < 0)
        debug_print("Attrib '%s' not found\n", names[i]);
      attribs[names[i]] = loc;
   }
}

void Shader::setUniform(const char* name, int value)
{
   if (!supported || !program) return;
   std::map<std::string,int>::iterator it = uniforms.find(name);
   if (it != uniforms.end())
   {
      GLint loc = uniforms[name];
      if (loc >= 0) glUniform1i(loc, value);
      GL_Error_Check;
   }
}

void Shader::setUniform(const char* name, float value)
{
   if (!supported || !program) return;
   std::map<std::string,int>::iterator it = uniforms.find(name);
   if (it != uniforms.end())
   {
      GLint loc = uniforms[name];
      if (loc >= 0) glUniform1f(loc, value);
      GL_Error_Check;
   }
}


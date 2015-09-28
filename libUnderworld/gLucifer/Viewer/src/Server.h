//Server - web server
#ifndef Server__
#define Server__

#ifndef DISABLE_SERVER

#include "OutputInterface.h"
#include "OpenGLViewer.h"
#include "mongoose/mongoose.h"

class Server : public OutputInterface
{
   //Singleton class, construct with Server::Instance()
  protected:
   //Server() {}   //Protected constructors
   Server(OpenGLViewer* viewer, std::string htmlpath, int port, int quality, int threads);
   //Server(Server const&) {} // copy constructor is protected
   //Server& operator=(Server const&) {} // assignment operator is protected

   static Server* _self;

   int port, threads;
   std::string path;

   // Thread sync
   pthread_mutex_t cs_mutex;
   pthread_cond_t condition_var;
   std::deque<std::string> commands;

   int client_id;
   int quality;
   bool updated;
   std::map<int,bool> synched; //Client status
   GLubyte *imageCache;

   OpenGLViewer* viewer; 

  public:
   unsigned char* jpeg;
   int jpeg_bytes;

   //Public instance constructor/getter
   static Server* Instance(OpenGLViewer* viewer, std::string htmlpath, int port, int quality=90, int threads=4);
   virtual ~Server();
   struct mg_context* ctx;
   
   static void* callback(enum mg_event event,
                         struct mg_connection *conn,
                         const struct mg_request_info *request_info);

   // virtual functions for window management
   virtual void open(int width, int height);
   virtual void resize(int new_width, int new_height);
   virtual void display();
   virtual void close();
   virtual void idle() {}

   bool compare(GLubyte* image);
};

#endif //DISABLE_SERVER
#endif //Server__

#ifndef MIMESERVER_H
#define MIMESERVER_H
#include <map>
#include <string>
#include "configuration.h"
namespace server
{
  class MimeServer;
  class Mime_T;

  class MimeServer
  {
  private:
    server::Config configuration;
    std::map<std::string, Mime_T> mimestore;
  public:
    MimeServer();
    MimeServer(const char* location);
    MimeServer(MimeServer& server);
    ~MimeServer();

    void load();
    void push();

    Mime_T& getMime(std::string file_extension);
    Mime_T& operator()(std::string file_extension);
    Mime_T& operator[](std::string file_extension);

    size_t getCount();
  };

  class Mime_T
  {
  public:
    std::string file_extension;
    std::string http_mime;
    std::string toString();
  };
}
#endif // MIMESERVER_H

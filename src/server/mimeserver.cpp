#include "mimeserver.h"
#include "stringproc.h"

server::MimeServer::MimeServer()
{}

server::MimeServer::MimeServer(const char* location) :
  configuration(location)
{
  load();
}

server::MimeServer::MimeServer(server::MimeServer& server) :
  configuration(server.configuration.getFilename())
{
  load();
}

server::Mime_T& server::MimeServer::getMime(std::string file_extension)
{
  size_t pos = file_extension.find_last_of('.');
  if(pos != std::string::npos)
  {
    return mimestore[file_extension.substr(pos + 1, file_extension.size())];
  }
  return mimestore[file_extension];
}

server::Mime_T& server::MimeServer::operator()(std::string file_extension)
{
  size_t pos = file_extension.find_last_of('.');
  if(pos != std::string::npos)
  {
    return mimestore[file_extension.substr(pos + 1, file_extension.size())];
  }
  return mimestore[file_extension];
}

server::Mime_T& server::MimeServer::operator[](std::string file_extension)
{
  size_t pos = file_extension.find_last_of('.');
  if(pos != std::string::npos)
  {
    return mimestore[file_extension.substr(pos + 1, file_extension.size())];
  }
  return mimestore[file_extension];
}

void server::MimeServer::load()
{
  configuration.refresh();
  const std::map<std::string, std::string> mp = configuration.getMap();
  for(std::map<std::string, std::string>::const_iterator it = mp.begin(); it != mp.end(); ++it)
  {
    mimestore[it->first].http_mime = it->second;
  }
}

void server::MimeServer::push()
{
  for(std::map<std::string, server::Mime_T>::const_iterator it = mimestore.begin(); it != mimestore.end(); ++it)
  {
    configuration[it->first] = it->second.http_mime;
  }
  configuration.flush();
}

server::MimeServer::~MimeServer()
{
  configuration.flush();
}

size_t server::MimeServer::getCount()
{
  return configuration.getMap().size();
}

std::string server::Mime_T::toString()
{
  return http_mime;
}

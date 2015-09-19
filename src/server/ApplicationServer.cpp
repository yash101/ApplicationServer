#include "ApplicationServer.h"
#include "stringproc.h"
#include "entry.h"
#include "defaults.h"
#include <regex>
#include <sstream>
#include <iostream>
#include <sys/stat.h>

ApplicationServer::ApplicationServer()
{
}


bool ApplicationServer::handle404(server::HttpServerSession& session)
{
  std::stringstream str;
#define o str << std::endl <<
  o "<DOCTYPE html>";
  o "<html>";
    o "<head>";
      o "<title>404 Not Found</title>";
    o "</head>";
    o "<body>";
      o "<h1><center>404 Not Found</center></h1>";
      o "<p>The path requested could not be loaded. This is all we know!</p>";
      o "<p>Please check the URL you may have entered or check the link you clicked!</p>";
      o "<h2>Error Traceback</h2>";
        o "<div style=\"background-color: #C6C6C6; margin: 32px;\">";
          o "<pre>";
            o "Path: [" << session.Path << "]";
          o "</pre>";
        o "</div>";
      o "<hr />";
      o "<div style=\"height: 300px;\"></div>";
      o "<p style=\"background-color: #C6C6C6;\">Powered by <a href=\"https://github.com/yash101/ApplicationServer/\">ApplicationServer</a>!</p>";
    o "</body>";
  o "<html>";
#undef o
  session.Response.stype = str.str();
  session.Response.type = server::STRING;
  session.ResponseStatusCode = 404;
  session.headers["content-type"] = "text/html";
}

void ApplicationServer::set_request_function(std::string query, ServerFunction_t callback)
{
  _functions[query] = callback;
}

#ifndef DISABLE_LAMBDAS
void ApplicationServer::set_request_lambda(std::string query, std::function<bool (server::HttpServerSession&)> callback)
{
  _lambdas[query] = callback;
}
#endif

void ApplicationServer::request_handler(server::HttpServerSession& session)
{
  bool success = false;

  for(std::map<std::string, ServerFunction_t>::const_iterator it = _functions.begin(); it != _functions.end(); ++it)
  {
    if(std::regex_match(session.Path, std::regex(it->first)))
    {
      if(it->second != NULL)
      {
        if(it->second(session))
        {
          success = true;
          break;
        }
      }
    }
  }

#ifndef DISABLE_LAMBDAS
  for(std::map<std::string, std::function<bool (server::HttpServerSession&)> >::const_iterator it = _lambdas.begin(); it != _lambdas.end(); ++it)
  {
    if(std::regex_match(session.Path, std::regex(it->first)))
    {
      if(it->second(session))
      {
        success = true;
        break;
      }
    }
  }
#endif

  if(!success)
  {
    handle404(session);
  }
}

void ApplicationServer::set_static(std::string regex)
{
  _functions[regex] = &this->static_handler;
}

bool ApplicationServer::static_handler(server::HttpServerSession& session)
{
  std::vector<std::string> parts = server::split(session.Path, '/');
  bool directory = false;

  std::string path;
  for(size_t i = 0; i < parts.size(); i++)
  {
    if(parts[i].size() == 0 || parts[i] == "/" || parts[i] == "../" || parts[i] == "./" || parts[i] == ".")
      continue;

    path += parts[i] + "/";

    struct stat st;
    stat(path.substr(0, path.size() - 1).c_str(), &st);
    if(S_ISREG(st.st_mode))
    {
      directory = false;
      break;
    }
    else if(S_ISDIR(st.st_mode))
    {
      directory = true;
    }
  }

  if(path.size() > 0)
    path.pop_back();

  if(!directory)
  {
    if((session.Response.ftype = fopen(path.c_str(), "r")) != NULL)
    {
      session.Response.type = server::FILE;
      session.ResponseStatusCode = 200;

      size_t pos = path.find_last_of('.');
      if(pos == std::string::npos || path.back() == '.')
        session.headers["content-type"] = "text/html";
      else
        session.headers["content-type"] = mime_server[path.substr(pos + 1, path.size())].http_mime;

      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    std::vector<std::string> indices = server::split(server::configuration()["indices"], ',');
    if(path.back() != '/') path.append("/");
    for(size_t i = 0; i < indices.size(); i++)
    {
      std::string npath = path + indices[i];
      if((session.Response.ftype = fopen(npath.c_str(), "r")) != NULL)
      {
        session.Response.type = server::FILE;
        session.ResponseStatusCode = 200;

        size_t pos = indices[i].find_last_of('.');
        if(pos == std::string::npos || npath.back() == '.')
          session.headers["content-type"] = "text/html";
        else
          session.headers["content-type"] = mime_server[npath.substr(pos + 1, npath.size())].http_mime;

        return true;
      }
    }

    return false;
  }
}

server::MimeServer mime_server(server::configuration()["mime_file"].c_str());

#include "ApplicationServer.h"
#include "stringproc.h"
#include "entry.h"
#include "defaults.h"
#include <regex>

ApplicationServer::ApplicationServer()
{}

void ApplicationServer::set_request_function(std::string query, ServerFunction_t callback)
{
  _functions[query] = callback;
}

#ifndef DISABLE_LAMBDAS
void ApplicationServer::set_request_lambda(std::string query, std::function<void (server::HttpServerSession&)> callback)
{
  _lambdas[query] = callback;
}
#endif


void ApplicationServer::request_handler(server::HttpServerSession& session)
{
  for(std::map<std::string, ServerFunction_t>::const_iterator it = _functions.begin(); it != _functions.end(); ++it)
  {
    if(std::regex_match(session.Path, std::regex(it->first)))
    {
      if(it->second != NULL)
      {
        it->second(session);
        break;
      }
    }
  }

#ifndef DISABLE_LAMBDAS
  for(std::map<std::string, std::function<void(server::HttpServerSession&)> >::const_iterator it = _lambdas.begin(); it != _lambdas.end(); ++it)
  {
    if(std::regex_match(session.Path, std::regex(it->first)))
    {
      it->second(session);
      break;
    }
  }
#endif
}

void ApplicationServer::set_static(std::string regex)
{
  _functions[regex] = &this->static_handler;
}

void ApplicationServer::static_handler(server::HttpServerSession& session)
{
  std::vector<std::string> parts = server::split(session.Path, '/');

  std::string out;
  for(size_t i = 0; i < parts.size(); i++)
  {
    if(parts[i].size() == 0 || parts[i] == "/" || parts[i] == "../" || parts[i] == "./" || parts[i] == ".")
      continue;
    out.append(parts[i] + "/");
  }
  if(out.size() > 0)
    out.pop_back();

  size_t pos;
  if((pos = out.find_last_of('.')) == std::string::npos || pos == out.size() - 1)
    session.headers["content-type"] = "octet/stream";
  else
  {
    std::string fext = out.substr(pos + 1, out.size());
    if(mime_server[fext].http_mime.size() == 0)
    {
      mime_server[fext].http_mime = "octet/stream";
    }
    else
    {
      session.headers["content-type"] = mime_server[fext].http_mime;
    }
  }

  session.Response.type = server::FILE;


  if(out.size() == 0 || out.back() == '/')
  {
    std::vector<std::string> extensions = server::split(server::configuration()["indices"], ',');
    for(size_t i = 0; i < extensions.size(); i++)
    {
      std::string loc = out + server::pad(extensions[i]);
      session.Response.ftype = fopen(loc.c_str(), "r");

      if(session.Response.ftype != NULL)
      {
        if((pos = loc.find_last_of('.')) == std::string::npos || pos == loc.size() - 1)
          session.headers["content-type"] = "octet/stream";
        else
        {
          std::string fext = loc.substr(pos + 1, loc.size());
          if(mime_server[fext].http_mime.size() == 0)
          {
            mime_server[fext].http_mime = "octet/stream";
          }
          else
          {
            session.headers["content-type"] = mime_server[fext].http_mime;
          }
        }
        break;
      }

    }
  }
  else
  {
    session.Response.ftype = fopen(out.c_str(), "r");
  }
}

server::MimeServer mime_server(server::configuration()["mime_file"].c_str());

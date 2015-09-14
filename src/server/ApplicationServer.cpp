#include "ApplicationServer.h"
#include "stringproc.h"
#include "entry.h"
#include "defaults.h"
#include <regex>
#include <iostream>

ApplicationServer::ApplicationServer() :
  mime_server(server::configuration()["mime_file"].c_str())
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
  out.pop_back();

  session.Response.type = server::FILE;
  std::cout << out << std::endl;
  session.Response.ftype = fopen(out.c_str(), "r");
}

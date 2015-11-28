/*
 * ApplicationServer.cpp
 *
 *  Created on: Nov 18, 2015
 *      Author: yash
 */

#include "ApplicationServer.h"
#include "HttpServer.h"
#include "stringproc.h"
#include "entry.h"

#include <regex>
#include <iostream>

#include <sys/stat.h>

daf::ApplicationServer::ApplicationServer()
{
}

daf::ApplicationServer::~ApplicationServer()
{
}

void daf::ApplicationServer::request_handler(daf::HttpServer::Session& session)
{
  if(session.Protocol == daf::Http::HTTP10)
  {
    if(!DefaultHostname(session))
    {
      throw daf::HttpServer::Exception("Could not resolve a hostname manager to handle the request!", -1, 500, __FILE__, __LINE__);
    }
  }
  else
  {
    bool complete = false;
    for(std::map<std::string, daf::Hostname>::const_iterator it = Hostnames.begin(); it != Hostnames.end(); ++it)
    {
      if(std::regex_match(session.Path, std::regex(it->first)))
      {
        complete = Hostnames[it->first](session);
        if(complete) break;
      }
    }

    if(!complete)
    {
      if(!DefaultHostname(session))
      {
        throw daf::HttpServer::Exception("Could not resolve a hostname manager to handle the request!", -1, 500, __FILE__, __LINE__);
      }
    }
  }
}

daf::Hostname::Hostname()
{
}

daf::Hostname::~Hostname()
{
}

bool daf::Hostname::operator()(daf::HttpServer::Session& session)
{
  bool complete = false;
  for(std::map<std::string, daf::PointerFunction>::iterator it = _pfncs.begin(); it != _pfncs.end(); ++it)
  {
    if(std::regex_match(session.Path, std::regex(it->first)))
      complete = (it->second)(session);
    if(complete) break;
  }

  if(complete) return true;

  for(std::map<std::string, daf::LambdaFunction>::iterator it = _lfncs.begin(); it != _lfncs.end(); ++it)
  {
    if(std::regex_match(session.Path, std::regex(it->first)))
      complete = (it->second)(session);
    if(complete) break;
  }

  if(complete) return true;

  for(std::map<std::string, _dfunc>::const_iterator it = _dfncs.begin(); it != _dfncs.end(); ++it)
  {
    if(std::regex_match(session.Path, std::regex(it->first)))
      complete = (it->second).func(session, it->second.data);
    if(complete) break;
  }

  return complete;
}

inline static std::string getHttpMime(std::string file_ext)
{
  std::string mime = Configuration()["ASHTTP.mime " + file_ext];
  if(mime.size() == 0)
  {
    Configuration()["ASHTTP.mime " + file_ext] = "octet/stream";
    Configuration().flush();
    mime = "octet/stream";
    std::cout << "Unable to find mime: " << file_ext << "! Adding as octet/stream!" << std::endl;
  }
  return mime;
}

inline static std::string getIndices()
{
  std::string ind = Configuration()["ASHTTP.indices"];
  if(ind.size() == 0)
  {
    Configuration()["ASHTTP.indices"] = "index.html,default.html,index.htm,default.htm";
    Configuration().flush();
    ind = Configuration()["ASHTTP.indices"];
  }
  return ind;
}

bool daf::static_handler(daf::HttpServer::Session& session, DPFDAT data)
{
  std::string upath;
  if(data != NULL)
    upath = std::string((const char*) data) + session.Path;
  else
    upath = session.Path;

  std::vector<std::string> parts = daf::split(upath, '/');
  bool directory = false;

  std::string fpath;
  struct stat st;

  for(size_t i = 0; i < parts.size(); i++)
  {
    if(parts[i].size() == 0 || parts[i] == "/" || parts[i] == "../" || parts[i] == "./" || parts[i] == ".")
      continue;

    fpath += parts[i] + '/';

    stat(fpath.substr(0, fpath.size() - 1).c_str(), &st);
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

  if(fpath.size() > 0)
    fpath.pop_back();

  if(!directory)
  {
    if((session.Response.file = fopen(fpath.c_str(), "r")) != NULL)
    {
      session.Response.DataType = daf::Http::FILE;
      session.StatusCode = 200;

      size_t pos = fpath.find_last_of('.');
      if(pos == std::string::npos || fpath.back() == '.')
      {
        session.Headers["content-type"] = "text/html";
      }
      else
      {
        session.Headers["content-type"] = getHttpMime(fpath.substr(pos + 1, fpath.size()));
      }

      return true;
    }
    else
    {
      session.StatusCode = 404;
      session.Response.DataType = daf::Http::STRING;
      return false;
    }
  }
  else
  {
    std::vector<std::string> indices = daf::split(getIndices(), ',');
    if(fpath.back() != '/') fpath.append("/");

    for(size_t i = 0; i < indices.size(); i++)
    {
      std::string npath = fpath + indices[i];
      if((session.Response.file = fopen(npath.c_str(), "r")) != NULL)
      {
        session.Response.DataType = daf::Http::FILE;
        session.StatusCode = 200;
        size_t pos = indices[i].find_last_not_of('.');
        if(pos == std::string::npos || npath.back() == '.')
          session.Headers["content-type"] = "text/html";
        else
          session.Headers["content-type"] = getHttpMime(npath.substr(pos + 1, npath.size()));

        return true;
      }
    }

    return false;
  }

  return false;
}




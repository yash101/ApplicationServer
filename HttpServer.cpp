/*
 * Server.cpp
 *
 *  Created on: Nov 11, 2015
 *      Author: yash
 */

#include "HttpServer.h"
#include "stringproc.h"
#include <string>
#include <sys/sendfile.h>
//To make it easier to throw an exception with debug info :)
#define HttpException(msg, code, httpstat) (daf::HttpServer::Exception(msg, code, httpstat, __FILE__, __LINE__))
#define MAX_HTTP_LINE_LENGTH (16384)
#define String std::string
#define Vector std::vector
#define ToString std::to_string

daf::HttpServer::Server::Server()
{
}

daf::HttpServer::Server::~Server()
{
}

void daf::HttpServer::Server::worker(daf::TcpServer::Connection* connection)
{
  daf::HttpServer::Session session;
  session.TcpConnection = connection;

  try
  {
    this->process_request(&session);
    this->check_request(&session);
    this->prepare_session(&session);
    this->request_handler(session);
    this->check_session_response(&session);
    this->send_response(&session);
  }
  catch(daf::HttpServer::Exception& e)
  {
    bad_request(&session, e);
  }
  catch(std::exception& e)
  {
  }
}

void daf::HttpServer::Server::bad_request(daf::HttpServer::Session* session, daf::HttpServer::Exception& e)
{
  std::stringstream write;
//  write << "HTTP/1.0 " << e.getStatusCode().getHttpStatusCode() << " " << e.getStatusCode().getStatusCode().message();;
}

void daf::HttpServer::Server::process_request(daf::HttpServer::Session* session)
{
  char flbuf[MAX_HTTP_LINE_LENGTH];
  flbuf[MAX_HTTP_LINE_LENGTH - 1] = '\0';

  if(session->TcpConnection->readline(flbuf, MAX_HTTP_LINE_LENGTH, '\n') >= MAX_HTTP_LINE_LENGTH)
  {
    throw HttpException("Bad Request: HTTP Field maximum length reached!", MAX_HTTP_LINE_LENGTH, 400);
  }

  String str = flbuf;
  if(str.back() == '\r' || str.back() == '\n') str.pop_back();
  daf::itrim(str);

  Vector<String> parts = daf::split(str, ' ');

  if(parts.size() < 3)
  {
    throw HttpException(
      std::string("400: Bad Request; Could not understand first line!\nReceived \"" + str + "\"!").c_str(),
      (int) parts.size(),
      400
    );
  }

  //To make our code cleaner :)
#define w (parts[0])
#define s session->RequestType =

  if(w == "GET") s daf::Http::GET;
  else if(w == "POST") s daf::Http::POST;
  else if(w == "HEAD") s daf::Http::HEAD;
  else if(w == "PUT") s daf::Http::PUT;
  else if(w == "DEL") s daf::Http::DEL;
  else if(w == "CONNECT") s daf::Http::CONNECT;
  else if(w == "OPTIONS") s daf::Http::OPTIONS;
  else if(w == "TRACE") s daf::Http::TRACE;
  else
  {
    char buffer[16384];
    snprintf(buffer, sizeof(buffer), "501 Unimplemented: \"%s\" has not been implemented or could not be understood!", w.c_str());
    throw HttpException(buffer, -1, 501);
  }

  //Clean up after ourselves (unneccessary)
#undef w
#undef s

  this->parse_get_queries(session, parts);

  daf::itrim(parts[2]);

  if(parts[2] == "HTTP/1.0") session->Protocol = daf::Http::HTTP10;
  else if(parts[2] == "HTTP/1.1") session->Protocol = daf::Http::HTTP11;
  else throw HttpException("400 Bad Request: Unknown protocol", -1, 400);

  while(true)
  {
    if(session->TcpConnection->readline(flbuf, MAX_HTTP_LINE_LENGTH, '\n') >= MAX_HTTP_LINE_LENGTH)
    {
      throw HttpException("400 Bad Request: Maximum HTTP Field Length Reached!", MAX_HTTP_LINE_LENGTH, 400);
    }

    str = flbuf;
    if(str.back() == '\r' || str.back() == '\n') str.pop_back();

    daf::itrim(str);

    if(str.size() == 0)
      break;

    ssize_t pos = daf::find(str, ':');
    if(pos <= 0) continue;

    String key = daf::tolower(daf::trim(str.substr(0, pos)));
    String value = daf::trim(str.substr(pos + 1, str.size()));

    if(key == "cookie")
    {
      Vector<String> cookies = daf::split(value, ';');
      for(size_t i = 0; i < cookies.size(); i++)
      {
        pos = daf::find(cookies[i], '=');

        key = daf::trim(cookies[i].substr(0, pos));
        value = daf::trim(cookies[i].substr(pos + 1, cookies[i].size()));

        session->IncomingCookies[daf::Http::decodeURI(key)] = daf::Http::decodeURI(value);
      }
    }
    else
    {
      session->IncomingHeaders[key] = value;
    }
  }

  parse_post_queries(session);
}

void daf::HttpServer::Server::parse_get_queries(daf::HttpServer::Session* session, Vector<String>& parts)
{
  session->CompletePath = parts[1];
  ssize_t loc = daf::find(parts[1], '?');
  if(loc < 0)
  {
    session->Path = parts[1];
  }
  else
  {
    session->Path = parts[1].substr(0, loc);
    String qstr = parts[1].substr(loc + 1, parts[1].size());
    Vector<String> queries = daf::split(qstr, '&');
    for(size_t i = 0; i < queries.size(); i++)
    {
      if(queries[i].size() == 0)
        continue;
      ssize_t pos = daf::find(queries[i], '=');
      if(pos == 0) continue;
      if(pos < 0)
        session->Get[daf::Http::decodeURI(queries[i])] = "";
      else
        session->Get[daf::Http::decodeURI(queries[i].substr(0, pos))] = daf::Http::decodeURI(queries[i].substr(pos + 1, queries[i].size()));
    }
  }
}

void daf::HttpServer::Server::parse_post_queries(daf::HttpServer::Session* session)
{
  if(session->RequestType == daf::Http::POST)
  {
    if(daf::tolower(session->IncomingHeaders["content-type"]) == "application/x-www-form-urlencoded")
    {
      if(session->IncomingHeaders["content-length"].size() != 0)
      {
        size_t len = atoll(session->IncomingHeaders["content-length"].c_str());
        daf::Http::DataSource tp = daf::Http::STRING;

        String key;
        String value;

        size_t i = 0;

        while(i < len)
        {
          while(i < len && key.back() != '=')
          {
            key += (char) session->TcpConnection->read();
            i++;
          }

          key.pop_back();

          while(i < len && value.back() != '&')
          {
            value += (char) session->TcpConnection->read();
            i++;
          }

          daf::Http::Data post;
          post.DataType = tp;
          post.string = daf::Http::decodeURI(value);

          session->Post[daf::Http::decodeURI(key)] = post;
        }
      }
    }
    else
    {
      throw HttpException("Unable to parse POST request. Multipart post not implemented!", -1, 501);
    }
  }
}

void daf::HttpServer::Server::check_request(daf::HttpServer::Session* session)
{
  if(session->Protocol != daf::Http::HTTP10)
  {
    if(session->IncomingHeaders["host"].size() == 0)
    {
      throw HttpException("No Host Provided! Routing failed!", 400, 400);
    }
  }
}

void daf::HttpServer::Server::prepare_session(daf::HttpServer::Session* session)
{
  session->Headers["content-type"] = "text/html";
  session->StatusCode = 200;
}

void daf::HttpServer::Server::check_session_response(daf::HttpServer::Session* session)
{
  if(session->Headers["content-type"].size() == 0)
    session->Headers["content-type"] = "octet/stream";
  if(session->Response.DataType == daf::Http::FILE)
  {
    if(session->Response.file == NULL || ftell(session->Response.file) < 0)
      throw HttpException("Attempting to send unopened file!", 500, 500);
    fseek(session->Response.file, 0, SEEK_END);
    session->Headers["content-length"] = ToString(ftell(session->Response.file));
    fseek(session->Response.file, 0, SEEK_SET);
  }
  else
  {
    session->Headers["content-length"] = ToString(session->Response.string.size());
  }

  session->Headers["date"] = daf::Http::timestamp();
}

void daf::HttpServer::Server::send_response(daf::HttpServer::Session* session)
{
  session->TcpConnection->write((session->Protocol == daf::Http::HTTP10) ? "HTTP/1.0 " : "HTTP/1.1 ");
  session->TcpConnection->write(ToString(session->StatusCode));
  session->TcpConnection->write(String(" " + daf::Http::statusString(session->StatusCode) + "\r\n").c_str());

  for(std::map<std::string, std::string>::const_iterator it = session->Headers.begin(); it != session->Headers.end(); ++it)
  {
    session->TcpConnection->write(String(it->first + ": " + it->second + "\r\n").c_str());
  }

  for(std::map<std::string, std::string>::const_iterator it = session->Cookies.begin(); it != session->Cookies.end(); ++it)
  {
    session->TcpConnection->write(String("Set-Cookie: " + daf::Http::encodeURI(it->first) + "=" + daf::Http::encodeURI(it->second) + "\r\n").c_str());
  }

  session->TcpConnection->write("\r\n");

  if(session->Response.DataType == daf::Http::STRING)
  {
    session->TcpConnection->write(session->Response.string.c_str());
  }
  else
  {
    fseek(session->Response.file, 0, SEEK_END);
    size_t len = ftell(session->Response.file);
    fseek(session->Response.file, 0, SEEK_SET);

    int ffd = fileno(session->Response.file);

    if(ffd < 0)
    {
      throw HttpException("Could not get file descriptor of file to be sent!", ffd, -500);
    }

#ifndef _WIN32
    ssize_t ret = sendfile(session->TcpConnection->getFileDescriptor(), ffd, 0, len);
#else
    ssize_t ret = 0;
#endif

    if(ret < 0)
    {
      throw HttpException("Could not complete transfer!", (int) ret, -500);
    }
    else if((size_t) ret < len)
    {
      char buffer[16384];
      size_t nreps = len / 16384;
      size_t carry = len % 16384;

      for(size_t i = 0; i < nreps; i++)
      {
        if(fread(buffer, 16384, 1, session->Response.file) > 0)
          session->TcpConnection->write(buffer, 16384);
      }

      if(fread(buffer, carry, 1, session->Response.file) > 0)
        session->TcpConnection->write(buffer, carry);
    }

    session->TcpConnection->write("\r\n\r\n");
  }
}

void daf::HttpServer::Server::request_handler(daf::HttpServer::Session& session)
{
  session.Response.string = "<!DOCTYPE html><html><body><h1>Hello World!</h1></body></html>";
}

void daf::HttpServer::Server::websocket_handler(daf::HttpServer::Socket& socket)
{
}
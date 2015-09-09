#include "httpserver.h"
#include "stringproc.h"
#include <sys/sendfile.h>
#include <unistd.h>
#include <signal.h>

#ifndef MAX_HTTP_LINE_LENGTH
#define MAX_HTTP_LINE_LENGTH (16384)
#endif

server::HttpServer::HttpServer()
{
}

void server::HttpServer::worker(server::TcpServerConnection* connection)
{
  server::HttpServerSession session;
  session.connection = connection;

  try
  {
    process_request(&session);
    check_request(&session);
    prepare_session(&session);
    request_handler(session);
    check_session_response(&session);
    send_response(&session);
  }
  catch(Exception& e)
  {
    bad_request(&session, e);
  }
}

void server::HttpServer::process_request(server::HttpServerSession* session)
{
  //This text buffer holds individual HTTP status lines
  char flbuf[MAX_HTTP_LINE_LENGTH];
  flbuf[MAX_HTTP_LINE_LENGTH] = '\0';

  //This reads the first line from the client
  if(session->connection->readline(flbuf, MAX_HTTP_LINE_LENGTH, '\n') >= MAX_HTTP_LINE_LENGTH - 1)
  {
    throw Exception(StatusCode("First line/GET request too long", MAX_HTTP_LINE_LENGTH));
  }

  //Convert to std string and then pad
  std::string str = flbuf;
  //Pop back extra '\r' or '\n's
  if(str.back() == '\r' || str.back() == '\n') str.pop_back();
  server::ipad(str);

  //Break down the string into its parts
  std::vector<std::string> parts = server::split(str, ' ');

  if(parts.size() < 3) raise(SIGABRT);

  //Begin processing first line
  if(parts[0] == "POST") session->RequestType = POST;
  else session->RequestType = GET;

  //Process the path
  parse_get_queries(session, parts);

  //Remove whitespace
  server::ipad(parts[2]);

  //Process the protocol version
  if(parts[2] == "HTTP/1.0") session->ProtocolVersion = HTTP_1_0;
  else if(parts[2] == "HTTP/1.1") session->ProtocolVersion = HTTP_1_1;
  else throw Exception(StatusCode("Unknown HTTP protocol version.", -1));

  //Download headers
  while(true)
  {
    //Download the header line
    if(session->connection->readline(flbuf, MAX_HTTP_LINE_LENGTH, '\n') == MAX_HTTP_LINE_LENGTH)
      throw Exception(StatusCode("Header field length too long", MAX_HTTP_LINE_LENGTH));
    //Turn the char pointer into a standard string
    str = flbuf;
    if(str.back() == '\r' || str.back() == '\n') str.pop_back();

    //Pad the whitespace from the header away
    server::ipad(str);

    //Escape, if needed
    if(str.size() == 0) break;

    //Find the position of the colon
    long pos = server::find(str, ':');
    if(pos <= 0) continue;

    //Separate the key from the value
    std::string key = server::tolower(server::pad(str.substr(0, pos)));
    std::string value = str.substr(pos + 1, str.size());
    server::ipad(value);

    //Add a cookie
    if(key == "cookie")
    {
      //Split the header line into each separate cookie
      std::vector<std::string> cookies = server::split(value, ';');
      for(size_t i = 0; i < cookies.size(); i++)
      {
        //Find the position of the '='
        pos = server::find(cookies[i], '=');

        //Separate the name from it's value
        key = cookies[i].substr(0, pos - 1);
        value = cookies[i].substr(pos + 1, cookies[i].size());

        //Remove whitespace. It's dirty
        server::ipad(key);
        server::ipad(value);

        //Set the cookie into the session object
        session->incoming_cookies[key] = server::HttpCookie(value);
      }
    }
    else
    {
      //Save the header line
      session->incoming_headers[key] = value;
    }
  }

  parse_post_queries(session);
}

void server::HttpServer::parse_get_queries(server::HttpServerSession* session, std::vector<std::string>& parts)
{
  session->UnprocessedPath = parts[1];
  long loc = server::find(parts[1], '?');
  if(loc < 0)
  {
    session->Path = parts[1];
  }
  else
  {
    session->Path = parts[1].substr(0, loc - 1);
    std::string qstr = parts[1].substr(loc + 1, parts[1].size());
    std::vector<std::string> queries = server::split(qstr, '&');
    for(size_t i = 0; i < queries.size(); i++)
    {
      if(queries[i].size() == 0) continue;
      long pos = server::find(queries[i], '=');
      if(pos == 0) continue;
      if(pos < 0)
      {
        session->get[queries[i]] = "";
      }
      else
      {
        session->get[queries[i].substr(0, pos - 1)] = queries[i].substr(pos - 1, queries[i].size());
      }
    }
  }
}

void server::HttpServer::parse_post_queries(server::HttpServerSession* session)
{
  //Only download request if clarified that we have a POST request
  if(session->RequestType == POST)
  {
    //If the POST request type is application/x-www-form-urlencoded
    if(server::tolower(session->incoming_headers["content-type"]) == "application/x-www-form-urlencoded")
    {
      //Check to make sure the content length is significant
      if(session->incoming_headers["content-length"].size() != 0)
      {
        //Gather the content length
        size_t len = std::atoll(session->incoming_headers["content-length"].c_str());

        //Pick whether to choose file or string
        server::HttpLocationType ftype = STRING;

        //Stores the key and the value
        std::string key;
        std::string value;

        //Position of the current stream
        size_t i = 0;

        while(true)
        {
          while(i < len && key.back() != '=')
          {
            key += (char) session->connection->read();
            i++;
          }

          while(i < len && value.back() != ';')
          {
            value += (char) session->connection->read();
            i++;
          }

          server::HttpPost pt;
          pt.type = ftype;
          pt.value = value;

          session->post[key] = pt;
        }
      }
    }
    ///If we got a multipart request...
    else if(server::tolower(session->incoming_headers["content-type"]).find("multipart/form-data") != std::string::npos)
    {
      //To be implemented... :(
    }
  }
}

void server::HttpServer::check_request(server::HttpServerSession* session)
{
  if(session->ProtocolVersion != HTTP_1_0)
  {
    if(session->incoming_headers["host"].size() == 0)
    {
      throw Exception(StatusCode("No host provided. Routing failed.", 500));
    }
  }
}

void server::HttpServer::prepare_session(server::HttpServerSession* session)
{
  session->headers["content-type"] = "text/html";
  session->ResponseStatusCode = 200;
}

void server::HttpServer::check_session_response(server::HttpServerSession* session)
{
  if(session->headers["content-type"].size() == 0) session->headers["content-type"] = "text/html";
  if(session->Response.type == FILE)
  {
    fseek(session->Response.ftype, 0, SEEK_END);
    session->headers["content-length"] = server::toString(ftell(session->Response.ftype));
    fseek(session->Response.ftype, 0, SEEK_SET);
  }
  else
  {
    session->headers["content-length"] = server::toString(session->Response.stype.size());
  }

  session->headers["date"] = server::getHTTPTimestamp();
}

void server::HttpServer::send_response(server::HttpServerSession* session)
{
  //Write back the first line
  session->connection->write((session->ProtocolVersion == HTTP_1_0) ? "HTTP/1.0 " : "HTTP/1.1 ");
  session->connection->write(server::toString(session->ResponseStatusCode));
  session->connection->write(" " + server::getHTTPStatus(session->ResponseStatusCode) + "\r\n");

  //Write back the headers
  for(std::map<std::string, std::string>::const_iterator it = session->headers.begin(); it != session->headers.end(); ++it)
  {
    session->connection->write(it->first + ": " + it->second + "\r\n");
  }

  //Send all cookies
  for(std::map<std::string, server::HttpCookie>::const_iterator it =
        session->cookies.begin();
      it != session->cookies.end();
      ++it
  )
  {
    session->connection->write("Cookie: " +
      server::encodeURI(it->first) +
      "=" +
      server::encodeURI(it->second.value) +
      " ; path=" +
      server::encodeURI(it->second.path) +
      "\r\n"
    );
  }

  session->connection->write("\r\n");
  if(session->Response.type == STRING)
  {
    session->connection->write(session->Response.stype);
  }
  else
  {
    fseek(session->Response.ftype, 0, SEEK_END);
    size_t len = ftell(session->Response.ftype);
    fseek(session->Response.ftype, 0, SEEK_SET);

    int ffd = fileno(session->Response.ftype);

    if(ffd < 0)
      throw Exception(StatusCode("Unable to read the file to be sent!", ffd));

    int ret = sendfile(session->connection->fd, fileno(session->Response.ftype), 0, len);

    if(ret < 0)
      throw Exception(StatusCode("Unable to complete file transfer!", ret));
    else if(ret < len)
      throw Exception(StatusCode("Write terminated prematurely", ret));
  }
}


void server::HttpServer::bad_request(server::HttpServerSession* session, Exception& e)
{
}

void server::HttpServer::request_handler(HttpServerSession& session)
{
  session.Response.type = STRING;
  session.Response.stype = "<!DOCTYPE html>\n<html><body><h1>Hello World!</h1></body></html>";
}

void server::HttpServer::websocket_handler(server::HttpSocketSession& session)
{}

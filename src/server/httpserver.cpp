#include "httpserver.h"
#include "stringproc.h"
#include "entry.h"
#define ENABLE_HTTP_SERVER_EXTRA_FUNCTIONS
#include "httpserverextrafunctions.h"
#include <string.h>

#ifndef _WIN32

#include <sys/sendfile.h>
#include <unistd.h>

#endif

#ifndef MAX_HTTP_LINE_LENGTH
#define MAX_HTTP_LINE_LENGTH (65536)
#endif

server::HttpServer::HttpServer() :
  logger(NULL)
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
  catch(std::bad_alloc& e)
  {
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

  //Chech if we dont have all the parts
  if(parts.size() < 3) throw Exception(StatusCode("Could not understand first line.", parts.size()));

  //Begin processing first line
  if(parts[0] == "POST") session->RequestType = POST;
  else if(parts[0] == "GET") session->RequestType = GET;
  else throw Exception(StatusCode("Unimplemented function.", -1));

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
        key = cookies[i].substr(0, pos);
        value = cookies[i].substr(pos + 1, cookies[i].size());

        //Remove whitespace. It's dirty
        server::ipad(key);
        server::ipad(value);

        //Set the cookie into the session object
        session->incoming_cookies[server::decodeURI(key)] = server::HttpCookie(server::decodeURI(value));
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
    session->Path = parts[1].substr(0, loc);
    std::string qstr = parts[1].substr(loc + 1, parts[1].size());
    std::vector<std::string> queries = server::split(qstr, '&');
    for(size_t i = 0; i < queries.size(); i++)
    {
      if(queries[i].size() == 0) continue;
      long pos = server::find(queries[i], '=');
      if(pos == 0) continue;
      if(pos < 0)
      {
        session->get[server::decodeURI(queries[i])] = "";
      }
      else
      {
        session->get[server::decodeURI(queries[i].substr(0, pos))] = server::decodeURI(queries[i].substr(pos + 1, queries[i].size()));
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

        while(i < len)
        {
          while(i < len && key.back() != '=')
          {
            key += (char) session->connection->read();
            i++;
          }

          key.pop_back();

          while(i < len && value.back() != '&')
          {
            value += (char) session->connection->read();
            i++;
          }

          server::HttpPost pt;
          pt.type = ftype;
          pt.value = server::decodeURI(value);

          session->post[server::decodeURI(key)] = pt;
        }
      }
    }
    ///If we got a multipart request...
    else if(server::tolower(session->incoming_headers["content-type"]).find("multipart/form-data") != std::string::npos)
    {
      throw Exception(StatusCode("Unable to parse POST request; Multipart POST request processing is under development!", -1));
      /****************************************
       * 					UNDER DEVELOPMENT						*
       ****************************************/
      if(session->incoming_headers["content-length"].size() != 0)
      {
        size_t reqsize = atoll(session->incoming_headers["content-length"].c_str());
        std::string boundary = "";
        std::vector<std::string> pts = server::split(session->incoming_headers["content-type"], ';');
        for(size_t i = 0; i < pts.size(); i++)
        {
          server::tolower(pts[i]);
          std::vector<std::string> d2x = server::split(pts[i], '=');
          if(d2x.size() < 2)
          {
            throw Exception(StatusCode("Unable to process POST data. Invalid POST data!", -1));
          }
          else
          {
            boundary = server::pad(d2x[1]);
          }
        }


        size_t nrec = 0;
        std::string rdbuf = "";

        while(nrec++ < reqsize && rdbuf.back() != '\n' && rdbuf[rdbuf.size() - 2] != '\r')
        {
          rdbuf += (char) session->connection->read();
        }

        if(rdbuf.back() == '\r' || rdbuf.back() == '\n')
        {
          rdbuf.pop_back();
        }
      }

      /***************************************
       * 				FINISH UNDER DEVELOPMENT     *
       ***************************************/
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

  if(server::configuration().tryBool("request_logging_enable"));
  log("Processed request for:\n\tPath: "
    + session->UnprocessedPath
    + std::string("\n\tRequest Type: ")
    + ((session->RequestType == GET) ? "GET" : "POST")
    + std::string("\n\tProtocol Version: ")
    + ((session->ProtocolVersion == HTTP_1_0) ? "HTTP/1.0 (HTTP 1.0)" : "HTTP/1.1 (HTTP 1.1)")
    + std::string("\n")
  );
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
    if(session->Response.ftype == NULL || ftell(session->Response.ftype) < 0) throw Exception(StatusCode("Attempting to send a file; file not open. Aborting.", -1));
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
  for(std::map<std::string, std::string>::const_iterator it =
        session->cookies.begin();
      it != session->cookies.end();
      ++it
  )
  {
    session->connection->write("Set-Cookie: " + server::encodeURI(it->first) + "=" + server::encodeURI(it->second) + "\r\n");
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
    {
      throw Exception(StatusCode("Unable to read the file to be sent!", ffd));
    }

#ifndef _WIN32
    int ret = sendfile(session->connection->fd, fileno(session->Response.ftype), 0, len);
#else
    int ret = 0;
#endif

    if(ret < 0)
    {
      throw Exception(StatusCode("Unable to complete file transfer!", ret));
    }
    else if(ret < len)
    {
      //Holds 16 KB chunks of data
      char buffer[16384];
      size_t nreps = len / 16384;
      size_t carry = len % 16384;

      for(size_t i = 0; i < nreps; i++)
      {
        if(fread(buffer, 16384, 1, session->Response.ftype) > 0)
        {
          session->connection->write(buffer, 16384);
        }
      }

      if(fread(buffer, carry, 1, session->Response.ftype) > 0)
      {
        session->connection->write(buffer, carry);
      }
    }

    session->connection->write("\r\n\r\n");
  }
}

void server::HttpServer::bad_request(server::HttpServerSession* session, Exception& e)
{
  log(e.getStatusCode().toString());

  session->connection->write("HTTP/1.0 500 Internal Server Error\r\nContent-Type: text/html\r\n");
  std::stringstream str;
  str << "<!DOCTYPE html>" << std::endl;
  str << "<html><head><title>Internal Server Error: " << e.what() << "</title></head>";
  str << "<body>";
    str << "<h1><center>Internal Server Error</center></h1>";
    str << "<b>Issue: The server encountered an error processing your request.</b>";
    str << "<h2>Error Traceback:</h2>";
    str << "<pre>" << e.getStatusCode().toString() << "</pre>";
  str << "</body>";
  str << "</html>";
  session->connection->write("Content-Length: " + server::toString(str.str().size()) + "\r\n\r\n");
  session->connection->write(str.str());
}

void server::HttpServer::request_handler(HttpServerSession& session)
{
  getDefaultHomePage(&session);
}

void server::HttpServer::websocket_handler(server::HttpSocketSession& session)
{}

server::HttpServer& server::HttpServer::setLogger(server::Logger& log)
{
  logger = &log;
  return (*this);
}

void server::HttpServer::log(std::string str)
{
  if(logger != NULL)
    (*logger)(str.c_str());
}

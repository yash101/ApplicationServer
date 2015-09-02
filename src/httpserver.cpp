#include "httpserver.h"
#include "stringproc.h"

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

  process_request(&session);
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
}

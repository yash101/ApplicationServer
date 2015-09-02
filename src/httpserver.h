#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include "tcpserver.h"
namespace server
{
  class HttpServer;
  class HttpServerSession;
  class HttpSocketSession;

  class HttpServer : public server::TcpServer
  {
  private:
    void worker(server::TcpServerConnection* connection);
    void bad_request(server::HttpServerSession* session);
    void process_request(server::HttpServerSession* session);
      void parse_get_queries(server::HttpServerSession* session);
      void add_cookie(server::HttpServerSession* session);
      void parse_post_queries(server::HttpServerSession* session);
    void check_request(server::HttpServerSession* session);
    void prepare_session(server::HttpServerSession* session);

    void check_session_response(server::HttpServerSession* session);
    void send_response(server::HttpServerSession* session);
  protected:
    virtual void request_handler(HttpServerSession& session);
    virtual void websocket_handler(HttpSocketSession& session);
  public:
    HttpServer();
  };

  class HttpServerSession
  {
  public:
    server::TcpServerConnection* connection;
  };

  class HttpSocketSession
  {
  };
}
#endif // HTTPSERVER_H

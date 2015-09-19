#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include "tcpserver.h"
#include "logger.h"
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <mutex>
namespace server
{
  class HttpServer;
  class HttpServerSession;
  class HttpSocketSession;
  class HttpCookie;
  class HttpPost;
  class HttpResponse;

  enum HttpRequestType
  {
    GET,
    POST,
    HEAD,
    PUT,
    DEL,
    CONNECT,
    OPTIONS,
    TRACE
  };

  enum HttpProtocolVersion
  {
    HTTP_1_0,
    HTTP_1_1
  };

  enum HttpLocationType
  {
    FILE, STRING
  };

  class HttpServer : public server::TcpServer
  {
  private:

    void worker(server::TcpServerConnection* connection);
    void bad_request(server::HttpServerSession* session, Exception& exe);
    void process_request(server::HttpServerSession* session);
      void parse_get_queries(server::HttpServerSession* session, std::vector<std::string>& parts);
      void parse_post_queries(server::HttpServerSession* session);
    void check_request(server::HttpServerSession* session);
    void prepare_session(server::HttpServerSession* session);

    void check_session_response(server::HttpServerSession* session);
    void send_response(server::HttpServerSession* session);

    server::Logger* logger;
    void log(std::string str);

  protected:

    virtual void request_handler(HttpServerSession& session);
    virtual void websocket_handler(HttpSocketSession& session);

  public:

    HttpServer();
    HttpServer& setLogger(server::Logger& logger);

  };

  class HttpResponse
  {
  public:
    server::HttpLocationType type;
    std::string stype;
    ::FILE* ftype;

    HttpResponse();
    ~HttpResponse();
  };

  class HttpServerSession
  {
  public:
    server::TcpServerConnection* connection;
    HttpRequestType RequestType;
    HttpProtocolVersion ProtocolVersion;
    std::string Path;
    std::string UnprocessedPath;

    server::HttpResponse Response;
    short int ResponseStatusCode;

    std::map<std::string, std::string> get;
    std::map<std::string, server::HttpPost> post;
    std::map<std::string, std::string> incoming_headers;
    std::map<std::string, std::string> headers;
    std::map<std::string, server::HttpCookie> incoming_cookies;
    std::map<std::string, std::string> cookies;
  };

  class HttpSocketSession
  {
  };

  class HttpCookie
  {
  public:
    std::string value;
    std::string expires;
    std::string domain;
    std::string path;

    HttpCookie();
    HttpCookie(std::string value);
  };

  class HttpPost
  {
  public:
    server::HttpLocationType type;
    std::string content_type;
    std::string file_location;
    std::string value;
    ::FILE* file;

    ~HttpPost();
  };

  std::string encodeURI(std::string in);
  std::string decodeURI(std::string in);
  std::string getHTTPStatus(short int code);
  std::string getHTTPTimestamp();
  extern std::string day[];
  extern std::string months[];
}
#endif // HTTPSERVER_H

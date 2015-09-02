#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "core.h"
#include <string>
#include <netinet/in.h>
namespace server
{
  class TcpServer;
  class TcpServerConnection;

  class TcpServer
  {
  protected:
    int _fd;
    void* _address;

    int _port;
    bool _alreadyRunning;

    ReturnStatusCode listener();
    void listenerProxy(void* connection);
    virtual void worker(server::TcpServerConnection* connection);
  public:
    TcpServer();
    ~TcpServer();

    server::TcpServer& set_port(int port);
    ReturnStatusCode start_server();
  };

  class TcpServerConnection
  {
  private:
  public:
    TcpServerConnection();
    ~TcpServerConnection();

    int fd;
    struct sockaddr_in* address;
    server::TcpServer* server;

    char read();
    std::string read(size_t mxlen);
    int read(void* buffer, size_t len);
    void write(std::string data);
    void write(void* data, size_t len);

    std::string readline(char end);
    int readline(char* buffer, size_t len, char end);
  };
}
#endif // TCPSERVER_H

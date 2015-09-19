#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "core.h"
#include <string>
#include <mutex>

#ifndef _WIN32

#include <netinet/in.h>

#else

#include <winsock.h>

#endif

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

    size_t _connected_clients;
    size_t _max_connected_clients;

    std::mutex _connected_clients_mutex;
    std::mutex _max_connected_clients_mutex;

  public:
    TcpServer();
    ~TcpServer();

    server::TcpServer& set_port(int port);
    ReturnStatusCode start_server();

    server::TcpServer& setReadTimeoutMicroSeconds(size_t usec);
    server::TcpServer& setReadTimeoutSeconds(size_t sec);

    server::TcpServer& setMaxConnectedClients(size_t max);

    struct timeval _timeout;
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

    std::mutex* _external_connected_clients_lock;
    size_t* _connected_clients;

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

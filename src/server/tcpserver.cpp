#include "tcpserver.h"
#include "entry.h"
#include <string.h>
#include <stddef.h>
#include <new>
#include <thread>
#include <sstream>

#ifndef _WIN32

#include <netinet/in.h>
#include <unistd.h>

#else

#pragma comment(lib, "Ws2_32.lib")

#include <Windows.h>
WSADATA wsadata;
WORD wVersionRequested = NULL;
int wsaerr = 0;
bool wsaInitialized = false;

#endif

server::TcpServer& server::TcpServer::set_port(int port)
{
  if(!_alreadyRunning)
    _port = port;
  return (*this);
}

ReturnStatusCode server::TcpServer::start_server()
{
  //Quit if the server is already running
  if(_alreadyRunning) return StatusCode("Server is already running.", -1);
  _alreadyRunning = true;

  //Create the socket
  _fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  //Check if socket creation successful
  if(_fd < 0)
    return StatusCode("Unable to create socket", -2);

  //Clear the data of (struct sockaddr_in*) _address
  memset(_address, 0, sizeof(struct sockaddr_in));

  //Set the socket family (internet)
  ((struct sockaddr_in*) _address)->sin_family = AF_INET;
  //Set the socket address to any address
  ((struct sockaddr_in*) _address)->sin_addr.s_addr = INADDR_ANY;
  //Set the port number to listen on
  ((struct sockaddr_in*) _address)->sin_port = htons(_port);

  //Set REUSEADDR to true
  int on = 1;
#ifndef _WIN32
  setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#else
  setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));
#endif

  //Attempt to bind to the socket
  if(bind(_fd, (struct sockaddr*) _address, sizeof(struct sockaddr_in)) < 0)
    return StatusCode("Unable to bind socket to address structure", -2);

  //Begin listening on the socket and accepting connections
  return this->listener();
}

ReturnStatusCode server::TcpServer::listener()
{
  //Begin listening on the socket
  listen(_fd, 3);

  int sockaddr_in_size = sizeof(struct sockaddr_in);

  //Infinitely accept new connections
  while(true)
  {
    //This class holds the information to play around with a connection
    server::TcpServerConnection* connection = NULL;

    //Allocate new connection class
    try
    {
      connection = new server::TcpServerConnection;
    }
    catch(std::bad_alloc& e)
    {
      //Sleep for 10 ms, hoping to have memory available afterwards
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }

    while(true)
    {
      //Check if we are allowed to accept another connection
      _max_connected_clients_mutex.lock();
      size_t max = _max_connected_clients;
      _max_connected_clients_mutex.unlock();

      _connected_clients_mutex.lock();
      size_t connected = _connected_clients;
      _connected_clients_mutex.unlock();

      if(max == 0 || connected < max)
        break;
    }

    //Accept a connection
#ifdef _WIN32
    typedef int socklen_t;
#endif

    connection->fd = accept(
      _fd,
      (struct sockaddr*) connection->address,
      (socklen_t*) &sockaddr_in_size
    );

    //Validate the accept()
    if(connection->fd < 0)
    {
      //delete the connection pointer and retry
      delete connection;
      continue;
    }

    //Set the read timeout
#ifndef _WIN32
    if(setsockopt(connection->fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*) &_timeout, sizeof(struct timeval)))
#else
    if(setsockopt(connection->fd, SOL_SOCKET, SO_RCVTIMEO, (char*) &_timeout, sizeof(struct timeval)))
#endif
    {
      delete connection;
      continue;
    }

    //Increment the connected client tracker
    _connected_clients_mutex.lock();
    _connected_clients++;
    _connected_clients_mutex.unlock();

    //Give the connection access to the client tracker
    connection->_external_connected_clients_lock =
      &_connected_clients_mutex;
    connection->_connected_clients =
      &_connected_clients;

    try
    {
      //Launch the worker in a separate thread
      std::thread(&server::TcpServer::listenerProxy, this, (void*) connection).detach();
    }
    catch(std::exception& e)
    {
      delete connection;
      continue;
    }
  }
}

void server::TcpServer::listenerProxy(void* connection)
{
  server::TcpServerConnection* conn = (server::TcpServerConnection*) connection;
  try
  {
    worker(conn);
    delete conn;
    return;
  }
  catch(std::exception& e)
  {
    delete conn;
  }
}

void server::TcpServer::worker(server::TcpServerConnection* connection)
{
  char buffer[1024];
  connection->write("Hello World!\nName: $ ");
  connection->readline(buffer, 1024, '\n');
  connection->write("Hello, " + std::string(buffer) + "! Check the docs on how to use this socket server!\n");
}

server::TcpServer& server::TcpServer::setReadTimeoutMicroSeconds(size_t usec)
{
  _timeout.tv_usec = usec;
  return (*this);
}

server::TcpServer& server::TcpServer::setReadTimeoutSeconds(size_t sec)
{
  _timeout.tv_sec = sec;
  return (*this);
}

server::TcpServer& server::TcpServer::setMaxConnectedClients(size_t max)
{
  _max_connected_clients_mutex.lock();
  _max_connected_clients = max;
  _max_connected_clients_mutex.unlock();
  return (*this);
}

server::TcpServer::TcpServer() :
  _address((struct sockaddr_in*) new struct sockaddr_in),
  _port(0),
  _alreadyRunning(false),
  _fd(-1),
  _connected_clients(0),
  _max_connected_clients(0)
{
#ifdef _WIN32
  
  if(!wsaInitialized)
  {
    wVersionRequested = MAKEWORD(2, 2);
    wsaerr = WSAStartup(wVersionRequested, &wsadata);

    if(wsaerr != 0)
    {
      fprintf(stderr, "Unable to find winsock!\n");
      server::log("Unable to find and initialize winsock! Die.");
      exit(EXIT_FAILURE);
    }
  }

#endif

  memset(&_timeout, 0, sizeof(struct timeval));
}

server::TcpServer::~TcpServer()
{
  delete (struct sockaddr_in*) _address;
#ifndef _WIN32
  shutdown(_fd, SHUT_RDWR);
  close(_fd);
#else
  shutdown(_fd, 1);
  closesocket(_fd);
#endif
}

server::TcpServerConnection::TcpServerConnection() :
  address(NULL),
  _external_connected_clients_lock(NULL),
  _connected_clients(NULL)
{
  address = new struct sockaddr_in;
  memset(address, 0, sizeof(struct sockaddr_in));
}

server::TcpServerConnection::~TcpServerConnection()
{
  delete (struct sockaddr_in*) address;
#ifndef _WIN32
  shutdown(fd, SHUT_RDWR);
  close(fd);
#else
  shutdown(fd, 1);
  closesocket(fd);
#endif

  if(_external_connected_clients_lock != NULL &&
     _connected_clients != NULL
  )
  {
    _external_connected_clients_lock->lock();
    (*_connected_clients)--;
    _external_connected_clients_lock->unlock();
  }
}

char server::TcpServerConnection::read()
{
  char ch;
#ifdef _WIN32
  if(::recv(fd, &ch, sizeof(char), 0) <= 0)
#else
  if(::recv(fd, &ch, sizeof(char), MSG_NOSIGNAL) <= 0)
#endif
  {
    if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS)
      throw Exception(StatusCode("Socket read timed out", errno));
    throw Exception(StatusCode("Unable to read from pipe", errno));
  }

  return ch;
}

std::string server::TcpServerConnection::read(size_t mxlen)
{
  char* buffer;
  try
  {
    buffer = new char[mxlen + 1];
  }
  catch(std::bad_alloc& e)
  {
    throw Exception(StatusCode("Unable to allocate memory", -1));
  }

  int ret;
#ifdef _WIN32
  if((ret = ::recv(fd, buffer, mxlen, 0) <= 0))
#else
  if((ret = ::recv(fd, buffer, mxlen, MSG_NOSIGNAL) <= 0))
#endif
  {
    if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS)
      throw Exception(StatusCode("Socket read timed out", errno));
    throw Exception(StatusCode("Unable to read from socket", ret));
  }

  buffer[ret + 1] = '\0';

  std::stringstream s(buffer);
  delete[] buffer;
  return s.str();
}

int server::TcpServerConnection::read(void* buffer, size_t len)
{
  int ret;
#ifdef _WIN32
  if((ret = ::recv(fd, (char*) buffer, len, 0) <= 0))
#else
  if((ret = ::recv(fd, buffer, len, MSG_NOSIGNAL) <= 0))
#endif
  {
    if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS)
      throw Exception(StatusCode("Socket read timed out", errno));
    throw Exception(StatusCode("Unable to read from socket", ret));
  }
  return ret;
}

void server::TcpServerConnection::write(void* data, size_t len)
{
  int ret;
  void* dpt = data;
#ifdef _WIN32
  ret = ::send(fd, (char*) data, len, 0);
#else
  ret = ::send(fd, data, len, MSG_NOSIGNAL);
#endif
  if(ret < 0) throw Exception(StatusCode("Unable to write to socket", ret));
}

void server::TcpServerConnection::write(std::string data)
{
  this->write((void*) data.c_str(), data.size());
}

std::string server::TcpServerConnection::readline(char end)
{
  std::string str;
  try
  {
    while(true)
    {
      char ch = this->read();
      if(ch == end) return str;
      else str += end;
    }
  }
  catch(Exception& e)
  {
    return str;
  }
  return str;
}

int server::TcpServerConnection::readline(char* buffer, size_t len, char end)
{
  size_t i = 0;
  try
  {
    while(i < len)
    {
      buffer[i] = this->read();
      if(buffer[i] == end)
      {
        buffer[i] = '\0';
        return i;
      }
      i++;
    }
  }
  catch(Exception& e)
  {
    buffer[i] = '\0';
    return i;
  }
}

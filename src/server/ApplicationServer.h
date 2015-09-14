#ifndef APPLICATIONSERVER_H
#define APPLICATIONSERVER_H
#include "httpserver.h"
#include <string>
#include <map>

//For using lambda functions
#ifndef DISABLE_LAMBDAS
#include <functional>
#endif

typedef void (*ServerFunction_t)(server::HttpServerSession&);

class ApplicationServer : public server::HttpServer
{
protected:
  void request_handler(server::HttpServerSession& session);

  std::map<std::string, ServerFunction_t> _functions;
#ifndef DISABLE_LAMBDAS
  std::map<std::string, std::function<void(server::HttpServerSession&)> > _lambdas;
#endif

  static void static_handler(server::HttpServerSession& session);
public:
  ApplicationServer();

  void set_request_function(std::string regex, ServerFunction_t function);
#ifndef DISABLE_LAMBDAS
  void set_request_lambda(std::string regex, std::function<void(server::HttpServerSession& lambda)>);
#endif
  void set_static(std::string regex);

};
#endif // APPLICATIONSERVER_H

/*
 * ApplicationServer.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yash
 */

#ifndef APPLICATIONSERVER_H_
#define APPLICATIONSERVER_H_
#include "HttpServer.h"
#include <functional>
#include <map>

namespace daf
{
  class ApplicationServer;
  class Hostname;

  typedef bool (*PointerFunction)(daf::HttpServer::Session&);
  typedef bool (*DataPointerFunction)(daf::HttpServer::Session&, void*);
  typedef void* DPFDAT;
  typedef std::function<bool(daf::HttpServer::Session&)> LambdaFunction;

  bool static_handler(daf::HttpServer::Session& session, DPFDAT path);

  class Hostname
  {
    friend class ApplicationServer;
  protected:
    bool request_handler(daf::HttpServer::Session& session);
    std::map<std::string, PointerFunction> _pfncs;
    std::map<std::string, LambdaFunction> _lfncs;

    class _dfunc
    {
    public:
      DataPointerFunction func;
      DPFDAT data;
    };

    std::map<std::string, _dfunc> _dfncs;
  public:
    bool operator()(daf::HttpServer::Session& session);
    Hostname();
    virtual ~Hostname();

    inline void setStatic(std::string regex, const char* base_path)
    {
      _dfncs[regex].func = ::daf::static_handler;
      _dfncs[regex].data = (DPFDAT) base_path;
    }

    inline bool run(daf::HttpServer::Session& session)
    {
      return (*this)(session);
    }

    inline PointerFunction& pointerFunction(std::string regex)
    {
      return _pfncs[regex];
    }

    inline DataPointerFunction& dataPointerFunction(std::string regex)
    {
      return _dfncs[regex].func;
    }

    inline void setDPF(std::string regex, DataPointerFunction func, DPFDAT data)
    {
      _dfncs[regex].func = func;
      _dfncs[regex].data = data;
    }

    inline DPFDAT& DPFData(std::string regex)
    {
      return _dfncs[regex].data;
    }

    inline LambdaFunction& lambdaFunction(std::string regex)
    {
      return _lfncs[regex];
    }
  };

  class ApplicationServer : public daf::HttpServer::Server
  {
    friend class Hostname;
  protected:
    virtual void request_handler(daf::HttpServer::Session& session);
    std::map<std::string, Hostname> Hostnames;
    Hostname DefaultHostname;
  public:
    ApplicationServer();
    virtual ~ApplicationServer();

    inline Hostname& operator[](std::string regex)
    {
      return Hostnames[regex];
    }

    inline Hostname& defaultHostname()
    {
      return DefaultHostname;
    }
  };
}
#endif /* APPLICATIONSERVER_H_ */

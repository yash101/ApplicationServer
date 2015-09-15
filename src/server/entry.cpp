#include "entry.h"
#include "ApplicationServer.h"

int main(int argc, char** argv)
{
  ProgramArguments.argc = argc;
  ProgramArguments.argv = argv;

  server::configuration.LoadConfiguration();

  ApplicationServer server;
  server.set_port(1234);
  server.setLogger(server::log);
  server.setMaxConnectedClients(16);
  server.setReadTimeoutSeconds(10);
//  server.set_request_lambda("/", [](server::HttpServerSession& session)
//  {
//    session.Response.type = server::STRING;
//    session.Response.stype = "<DOCTYPE html>\n<html><body><h1>Hello World!</h1></body></html>";
//  });
  server.set_static("/(.*)");
  server::log(server.start_server().toString().c_str());

  return 0;
}

//This structure <core.h> holds the arguments passed to this program
ProgramArguments_t ProgramArguments;

//This structure loads and checks the configuration file. It also provides the configuration
//		file to other classes
server::ConfigurationManager server::configuration;
server::Logger server::log;

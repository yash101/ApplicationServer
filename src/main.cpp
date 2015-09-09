#include "main.h"
#include "httpserver.h"
#include <stdio.h>

int main(int argc, char** argv)
{
  ProgramArguments.argc = argc;
  ProgramArguments.argv = argv;

  server::HttpServer server;
  server.set_port(1234);
  printf("%s\n", server.start_server().toString().c_str());

  return 0;
}

//This structure <main.h> holds the arguments passed to this program
ProgramArguments_t ProgramArguments;

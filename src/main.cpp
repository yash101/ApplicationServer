#include "main.h"
#include "tcpserver.h"
#include <stdio.h>

int main(int argc, char** argv)
{
  ProgramArguments.argc = argc;
  ProgramArguments.argv = argv;

  server::TcpServer srv;
  srv.set_port(1235);
  printf("%s\n", srv.start_server().toString().c_str());

  return 0;
}

//This structure <main.h> holds the arguments passed to this program
ProgramArguments_t ProgramArguments;

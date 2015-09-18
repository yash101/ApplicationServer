#ifndef ENTRY_H
#define ENTRY_H
#include "ConfigManager.h"
#include "logger.h"
#include "ApplicationServer.h"

//Entry point of the server code
int main(int argc, char** argv);

//This structure holds the arguments sent to this program
typedef struct
{
  char** argv;
  int argc;
} ProgramArguments_t;

//Program arguments are held here
extern ProgramArguments_t ProgramArguments;

namespace server
{
  //Configuration manager is loaded here
  extern ConfigurationManager configuration;
  extern Logger log;
}

extern ApplicationServer AppServer;
#endif // ENTRY_H

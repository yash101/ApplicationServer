#include "entry.h"
#include "server.h"
#include <string.h>
#include <sys/stat.h>

int main(int argc, char** argv)
{
  ProgramArguments.argc = argc;
  ProgramArguments.argv = argv;

  for(int i = 1; i < argc; i++)
  {
    if(!strcmp(argv[i], "-c") || !strcmp(argv[i], "--configuration-location"))
    {
      if(i + 1 < argc)
        server::configuration().setFilename(argv[++i]);
      else
      {
        fprintf(stderr, "-c || --configuration-location followed by null!\n");
      }
    }
  }

#ifdef _WIN32
  CreateDirectory("config", NULL);
#else
  mkdir("config", S_IRWXU | S_IRWXG | S_IROTH);
#endif

  server::configuration.LoadConfiguration();

  AppServer.set_port(atoi(server::configuration()["portno"].c_str()));
  AppServer.setReadTimeoutSeconds(atoi(server::configuration()["socket_timeout"].c_str()));
  AppServer.setLogger(server::log);
  AppServer.setMaxConnectedClients(atoi(server::configuration()["max_connected_clients"].c_str()));

  server_setup();

  AppServer.start_server();

  return 0;
}

//This structure <core.h> holds the arguments passed to this program
ProgramArguments_t ProgramArguments;

//This structure loads and checks the configuration file. It also provides the configuration
//		file to other classes
server::ConfigurationManager server::configuration;
server::Logger server::log;
ApplicationServer AppServer;

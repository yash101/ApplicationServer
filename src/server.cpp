#include "server/server.h"

void server_setup()
{
  AppServer.set_static("/(.*)");
}

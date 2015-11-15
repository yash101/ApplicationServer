/*
 * entry.cpp
 *
 *  Created on: Nov 7, 2015
 *      Author: yash
 */

#include "entry.h"
#include "Logger.h"
#include "HttpServer.h"

int main(int argc, char** argv)
{
  Arguments::count = argc;
  Arguments::arguments = argv;

  daf::HttpServer::Server server;
  server.setPort(1234);
  server.startServer();
}

// Arguments are stored in the namespace, Arguments
// Defined in entry.h

int Arguments::count;
char** Arguments::arguments;

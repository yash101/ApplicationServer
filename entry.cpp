/*
 * entry.cpp
 *
 *  Created on: Nov 7, 2015
 *      Author: yash
 */

#ifndef CONFIGURATION_FILE_LOCATION
#define CONFIGURATION_FILE_LOCATION "Config.cfg"
#endif

#include <stdio.h>

#include "entry.h"
#include "ApplicationServer.h"

static daf::Config* conf = NULL;

int init(int argc, char** argv)
{
  Arguments::count = argc;
  Arguments::arguments = argv;
  conf = new daf::Config(CONFIGURATION_FILE_LOCATION);
}

daf::Config& Configuration()
{
  return (*conf);
}

// Arguments are stored in the namespace, Arguments
// Defined in entry.h

int Arguments::count;
char** Arguments::arguments;

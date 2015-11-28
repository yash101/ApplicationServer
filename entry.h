/*
 * entry.h
 *
 *  Created on: Nov 7, 2015
 *      Author: yash
 */

#ifndef ENTRY_H_
#define ENTRY_H_
#include "Configuration.h"
int init(int argc, char** argv);

namespace Arguments
{
  extern int count;
  extern char** arguments;
}

daf::Config& Configuration();

#endif /* ENTRY_H_ */

#ifndef CONFIGURATIONMANAGER_H
#define CONFIGURATIONMANAGER_H
#include "defaults.h"
#include "configuration.h"

namespace server
{
  class ConfigurationManager
  {
  private:
    server::Config config;
  public:
    ConfigurationManager();
    void LoadConfiguration();

    server::Config& configuration();
    server::Config& operator()();
  };
}

#endif // CONFIGURATIONMANAGER_H

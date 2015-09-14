#include "ConfigManager.h"
#include "stringproc.h"
#include "defaults.h"
#include "entry.h"

server::ConfigurationManager::ConfigurationManager() :
  config(CONFIGURATION_FILE_LOCATION)
{
}

void server::ConfigurationManager::LoadConfiguration()
{
  //Check if the port number has been set
  if(config["portno"].size() == 0)
    config["portno"] = server::toString(DEFAULT_PORT);

  //Check if the socket timeout has been set (msec)
  if(config["socket_timeout"].size() == 0)
    config["socket_timeout"] = server::toString(DEFAULT_HTTP_SOCKET_TIMEOUT);

  if(config["cache_mimes"].size() == 0)
    config["cache_mimes"] = server::toString(DEFAULT_HTTP_CACHE_MIMES);

  if(config["cache_size"].size() == 0)
    config["cache_size"] = server::toString(DEFAULT_FILE_CACHE_SIZE);

  if(config["log_location"].size() == 0)
    config["log_location"] = LOG_FILE_DEFAULT_LOCATION;

  config.flush();
  config.refresh();

  //Open the log file
  server::log.open(config["log_location"].c_str());
}

server::Config& server::ConfigurationManager::configuration()
{
  return config;
}

server::Config& server::ConfigurationManager::operator()()
{
  return config;
}

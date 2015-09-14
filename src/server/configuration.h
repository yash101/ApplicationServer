#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <map>
#include <string>
#include <mutex>

#include "core.h"

namespace server
{
  class Config
  {
  private:
    const char* _file_location;
    std::mutex _map_mutex;
    std::map<std::string, std::string> _map;
  public:
    Config();
    Config(Config& cfg);
    Config(const char* file_location);
    Config(std::string file_location);

    std::string& operator[](std::string key);
    std::string& operator()(std::string key);
    std::string& access(std::string key);
    std::string& operator[](const char* key);
    std::string& operator()(const char* key);
    std::string& access(const char* key);

    const bool tryBool(std::string key);

    const std::map<std::string, std::string> getMap();
    void put(std::string key, std::string value);
    const std::string get(std::string key);
    void remove(std::string key);

    void refresh();
    void flush();

    const char* getFilename();
    void setFilename(const char* location);
  };
}

#endif

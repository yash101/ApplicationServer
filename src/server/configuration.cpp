#include "configuration.h"
#include "stringproc.h"
#include "auto_mutex.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

//Declaring the uriencode functions to avoid a few conflicts
namespace server
{
  std::string encodeURI(std::string str);
  std::string decodeURI(std::string str);
}

server::Config::Config() :
  _file_location("")
{}

server::Config::Config(server::Config& config) :
  _file_location(config._file_location),
  _map(config._map)
{}

server::Config::Config(const char* file_location) :
  _file_location(file_location)
{
  try
  {
    this->refresh();
  }
  catch(Exception& e)
  {}
}

server::Config::Config(std::string file_location) :
  _file_location(file_location.c_str())
{
  try
  {
    this->refresh();
  }
  catch(Exception& e)
  {}
}

std::string& server::Config::operator[](std::string key)
{
  auto_mutex<std::mutex>(&this->_map_mutex);
  return _map[key];
}

std::string& server::Config::operator()(std::string key)
{
  auto_mutex<std::mutex>(&this->_map_mutex);
  return _map[key];
}

std::string& server::Config::access(std::string key)
{
  auto_mutex<std::mutex>(&this->_map_mutex);
  return _map[key];
}

std::string& server::Config::operator[](const char* key)
{
  auto_mutex<std::mutex>(&this->_map_mutex);
  return _map[key];
}

std::string& server::Config::operator()(const char* key)
{
  auto_mutex<std::mutex>(&this->_map_mutex);
  return _map[key];
}

std::string& server::Config::access(const char* key)
{
  auto_mutex<std::mutex>(&this->_map_mutex);
  return _map[key];
}

const std::map<std::string, std::string> server::Config::getMap()
{
  auto_mutex<std::mutex>(&this->_map_mutex);
  return _map;
}

void server::Config::put(std::string key, std::string value)
{
  auto_mutex<std::mutex>(&this->_map_mutex);
  _map[key] = value;
}

const std::string server::Config::get(std::string key)
{
  auto_mutex<std::mutex>(&this->_map_mutex);
  return _map[key];
}

void server::Config::remove(std::string key)
{
  auto_mutex<std::mutex>(&this->_map_mutex);
  _map.erase(key);
}

void server::Config::refresh()
{
  auto_mutex<std::mutex>(&this->_map_mutex);
  std::string buffer;

  std::ifstream fin(_file_location, std::ios::in);
  if(!fin.is_open())
  {
    _map.clear();
    return;
  }

  _map.clear();

  while(std::getline(fin, buffer, '\n'))
  {
    server::ipad(buffer);
    if(buffer[0] == '#') continue;

    size_t pos = buffer.find_last_of('#');
    if(pos != std::string::npos)
    {
      buffer = buffer.substr(0, pos);
    }

    server::ipad(buffer);

    if(buffer.size() == 0) continue;

    std::string key;
    std::string value;

    size_t del = server::find(buffer, ':');
    if(del < 0 || buffer[buffer.size()] == ':')
    {
      if(buffer[buffer.size()] == ':')
        buffer = buffer.substr(0, buffer.size() - 1);
      server::ipad(buffer);
      key = buffer;
      value = "";
    }
    else
    {
      key = buffer.substr(0, del);
      value = buffer.substr(del + 1, buffer.size());
      server::ipad(key);
      server::ipad(value);
    }

    if(value[0] == '\"' && value[value.size() - 1] == '\"')
      value = value.substr(1, value.size() - 2);
    if(key[0] == '\"' && key[key.size() - 1] == '\"')
      key = key.substr(1, key.size() - 2);

    _map[server::decodeURI(key)] = server::decodeURI(value);
  }
  fin.close();
}

void server::Config::flush()
{
  auto_mutex<std::mutex>(&this->_map_mutex);
  FILE* fd = fopen(_file_location, "w");
  if(fd == NULL)
    throw Exception(StatusCode("Unable to open file for writing", -1));
  for(std::map<std::string, std::string>::const_iterator it = _map.begin(); it != _map.end(); ++it)
  {
    fprintf(fd, "\"%s\": \"%s\"\n", server::encodeURI(it->first).c_str(), server::encodeURI(it->second).c_str());
  }

  fclose(fd);
}

const char* server::Config::getFilename()
{
  return _file_location;
}

void server::Config::setFilename(const char* location)
{
  _file_location = location;
}

const bool server::Config::tryBool(std::string key)
{
  auto_mutex<std::mutex>(&this->_map_mutex);
  std::string h = server::tolower(_map[key]);
  return h == "true" || h == "yes" || h == "1" || h == "y";
}

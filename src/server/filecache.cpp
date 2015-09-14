#include "filecache.h"
#include <map>
#include <stdio.h>
#include "entry.h"

/*
 *
 * Map to hold the cached data
 * Map to hold tracked files and access count
 * Thread to manage file tracking
 *
 */


namespace server
{
  namespace filecache
  {
    //Stores the amount of data stored in the RAM cache
    size_t _current_cache_size = 0;

    //Stores the number of times a file was accessed
    std::map<std::string, size_t> _access_count;

    //Stores the file data
    std::map<std::string, std::string> _cache_data;
  }
}

bool server::filecache::cache_accept(std::string location)
{
  FILE* f = fopen(location.c_str(), "r");
  if(f == NULL) return false;

  fseek(f, 0, SEEK_END);
  //The amount of data which will be stored if this file is added
  size_t nsize = server::filecache::_current_cache_size + ftell(f);

  fclose(f);

  size_t msize = atoll(server::configuration()["cache_size"].c_str());

  if(nsize <= msize)
  {
    return true;
  }

  return false;
}

server::filecache::cache_status server::filecache::in_cache(std::string location)
{
  return
    (server::filecache::_cache_data.find(location) !=
    server::filecache::_cache_data.end()) ?
    server::filecache::CACHED : server::filecache::FILESYSTEM;
}

size_t server::filecache::access_count(std::string location)
{
  return server::filecache::_access_count[location];
}

std::string server::filecache::get_from_cache(std::string location)
{
  server::filecache::_access_count[location]++;
  if(server::filecache::in_cache(location))
  {
    return server::filecache::_cache_data[location];
  }
  else
  {
    if(server::filecache::cache_accept(location))
    {
      FILE* file = fopen(location.c_str(), "r");
    }
  }
}

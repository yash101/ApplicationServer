#ifndef FILECACHE_H
#define FILECACHE_H
#include <string>
namespace server
{
  namespace filecache
  {
    enum cache_status
    {
      FILESYSTEM,
      CACHED
    };

    bool cache_accept(std::string location);
    cache_status in_cache(std::string location);
    size_t access_count(std::string location);

    std::string get_from_cache(std::string location);
  }
}
#endif // FILECACHE_H

#ifndef LOGGER_H
#define LOGGER_H
#include <stddef.h>
#include <mutex>
namespace server
{
  class Logger
  {
  private:
    const char* _file_location;
    const char* _mode;
    void* _fd;
    bool _opened;
    std::mutex _write_lock;

    const char* time();
  public:
    Logger();
    Logger(const char* location, const char* mode);
    Logger(const char* location);
    ~Logger();
    void open(const char* location, const char* mode);
    void open(const char* location);

    const char* operator()(const char* message);

    const char* log(const char* message);

    void write_bin(void* write, size_t len);
    void write_hex(void* write, size_t len);

    size_t size();
    size_t clear();
  };
}
#endif

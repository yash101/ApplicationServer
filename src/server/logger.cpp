#include "logger.h"
#include "core.h"
#include "auto_mutex.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

#ifndef DEFAULT_LOG_WRITE_MODE
#define DEFAULT_LOG_WRITE_MODE "a"
#endif

server::Logger::Logger(const char* location, const char* mode) :
  _file_location(location),
  _opened(false),
  _mode(mode)
{
  this->open(_file_location, mode);
}

server::Logger::Logger() :
  _file_location(""),
  _opened(false),
  _mode(DEFAULT_LOG_WRITE_MODE)
{}

server::Logger::Logger(const char* location) :
  _file_location(location),
  _opened(false),
  _mode(DEFAULT_LOG_WRITE_MODE)
{
  this->open(_file_location, _mode);
}

server::Logger::~Logger()
{
  if(_opened)
  {
    fclose((FILE*) _fd);
  }
}

void server::Logger::open(const char* location, const char* mode)
{
  auto_mutex<std::mutex>(&this->_write_lock);

  _file_location = location;
  _mode = mode;
  if(_opened)
    fclose((FILE*) _fd);

  _fd = (void*) fopen(_file_location, _mode);
  if(_fd == NULL)
  {
    throw Exception(StatusCode("Unable to open log file for writing.", -1));
    _opened = false;
  }
  else _opened = true;
}

void server::Logger::open(const char* location)
{
  auto_mutex<std::mutex>(&this->_write_lock);
  this->open(location, _mode);
}

const char* server::Logger::time()
{
  time_t rawtime = ::time(NULL);
  struct tm* timeinfo = gmtime(&rawtime);
  char* ret = asctime(timeinfo);
  ret[strlen(ret) - 1] = '\0';
  return ret;
}

const char* server::Logger::operator()(const char* message)
{
  auto_mutex<std::mutex>(&this->_write_lock);
  if(!_opened) throw Exception(StatusCode("Unable to log message. File has not been opened!", -1));
  fprintf((FILE*) _fd, "[%s] => [%s]\n", this->time(), message);
  return message;
}

const char* server::Logger::log(const char* message)
{
  auto_mutex<std::mutex>(&this->_write_lock);
  if(!_opened) throw Exception(StatusCode("Unable to log message. File has not been opened!", -1));
  fprintf((FILE*) _fd, "[%s] => [%s]\n", this->time(), message);
  return message;
}

void server::Logger::write_bin(void* buffer, size_t len)
{
  auto_mutex<std::mutex>(&this->_write_lock);
  if(!_opened) throw Exception(StatusCode("Unable to log data. File has not been opened!", -1));
  fprintf((FILE*) _fd, "[%s =>\n{\n", this->time());
  fwrite((char*) buffer, len, 1, (FILE*) _fd);
  fprintf((FILE*) _fd, "\n}\n");
}

void server::Logger::write_hex(void* obj, size_t len)
{
  auto_mutex<std::mutex>(&this->_write_lock);
  if(!_opened) throw Exception(StatusCode("Unable to log data. File has not been opened!", -1));

  char* w = (char*) obj;

  size_t nrows = (len / 16);
  size_t ext = (len % 16);

  fprintf((FILE*) _fd, "[%s] -> \n{\n", this->time());
  fprintf((FILE*) _fd, "\tLine\tHex\n");

  for(size_t i = 0; i < nrows; i++)
  {
    fprintf((FILE*) _fd, "\t[%u]\t%02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX\n",
      (int) i + 1,
      *((char*) w + 1),
      *((char*) w + 2),
      *((char*) w + 3),
      *((char*) w + 4),
      *((char*) w + 5),
      *((char*) w + 6),
      *((char*) w + 7),
      *((char*) w + 8),
      *((char*) w + 9),
      *((char*) w + 10),
      *((char*) w + 11),
      *((char*) w + 12),
      *((char*) w + 13),
      *((char*) w + 14),
      *((char*) w + 15),
      *((char*) w + 16)
    );

    w += 16 * sizeof(char);
  }

  if(ext != 0)
  {
    fprintf((FILE*) _fd, "\t[%u]\t", (int)(nrows + 1));
    for(size_t i = 0; i < ext; i++)
    {
      if(i < ext)
        fprintf((FILE*) _fd, "%02hhX ", *w++);
      else
        fprintf((FILE*) _fd, "%02hhX ", *w++);
    }
  }

  fprintf((FILE*) _fd, "\n}\n");
}

size_t server::Logger::size()
{
  FILE* fd = fopen(_file_location, "r");
  if(fd == NULL)
  {
    return 0;
  }
  fseek(fd, 0, SEEK_END);
  size_t sz = ftell(fd);
  fclose(fd);
  return sz;
}

size_t server::Logger::clear()
{
  size_t len = this->size();
  FILE* fd = fopen(_file_location, "w");
  if(fd == 0) throw Exception(StatusCode("Unable to clear file. Unable to write to file.", -1));
  fclose(fd);
  return len;
}

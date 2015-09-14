#include "stringproc.h"
#include <string.h>
#include <regex>
bool operator==(std::string a, std::string b)
{
  return !strcmp(a.c_str(), b.c_str());
}

bool operator!=(std::string a, std::string b)
{
  return strcmp(a.c_str(), b.c_str());
}

long server::find(std::string in, char f)
{
  for(size_t i = 0; i < in.size(); i++)
  {
    if(in[i] == f) return (long) i;
  }
  return -1;
}

long server::find(std::string in, std::string f)
{
  //If the search string is shorter than query (micro-optimization)
  if(in.size() < f.size()) return -1;
  size_t x = in.find(f);
  //Lil' micro-optimization to produce inline assembly in dumber compilers
  return x == std::string::npos ? -1 : x;
}

template<typename T> void server::fromString(T& x, std::string in)
{
  std::stringstream str(in);
  str >> x;
  return;
}

bool server::contains(char find, std::string dict)
{
  for(size_t i = 0; i < dict.size(); i++)
  {
    if(find == dict[i]) return true;
  }
  return false;
}

bool iispace(char ch)
{
  return !(!isspace(ch) && ch != '\r' && ch != '\n');
}

std::string server::pad(std::string str)
{
  return server::lpad(server::rpad(str));
}

std::string server::lpad(std::string in)
{
  std::string out = in;

  auto it = std::find_if(
    out.begin(),
    out.end() ,
    [](char ch)
    {
      return !std::isspace<char>(ch, std::locale::classic());
    }
  );

  out.erase(out.begin(), it);
  return out;
}

std::string server::rpad(std::string in)
{
  std::string str = in;
  auto it = std::find_if(str.rbegin(),
    str.rend(),
    [](char ch)
    {
      return !std::isspace<char>(ch, std::locale::classic());
    }
  );
  str.erase( it.base() , str.end() );
  return str;
}

void server::ipad(std::string& in)
{
  in = server::pad(in);
}

void server::ilpad(std::string& in)
{
  in = server::lpad(in);
}

void server::irpad(std::string& in)
{
  in = server::rpad(in);
}

std::string server::tolower(std::string in)
{
  std::string x = in;
  for(size_t i = 0; i < x.size(); i++)
  {
    x[i] = ::tolower(x[i]);
  }
  return x;
}

std::string server::toupper(std::string in)
{
  std::string x = in;
  for(size_t i = 0; i < x.size(); i++)
  {
    x[i] = ::toupper(x[i]);
  }
  return x;
}

void server::tolowerf(const char* str)
{
  char* x = (char*) str;
  while(*x != '\0')
  {
    *x = ::tolower(*x);
    x++;
  }
}

void server::toupperf(const char* str)
{
  char* x = (char*) str;
  while(*x != '\0')
  {
    *x = ::toupper(*x);
    x++;
  }
}

void server::itolower(std::string& in)
{
  for(size_t i = 0; i < in.size(); i++)
  {
    in[i] = ::tolower(in[i]);
  }
}

void server::itoupper(std::string& in)
{
  for(size_t i = 0; i < in.size(); i++)
  {
    in[i] = ::tolower(in[0]);
  }
}

bool server::getline(std::string& buffer, std::string end, std::istream& stream)
{
  buffer.clear();
  std::string out;
  while(stream.good())
  {
    if(out.size() >= end.size())
    {
      if(out.substr(out.size() - end.size(), out.size()) == end)
      {
        buffer = out.substr(0, out.size() - end.size());
        return true;
      }
    }
    out += (char) stream.get();
  }
  return false;
}

std::vector<std::string> server::split(std::string stream, char f)
{
  if(stream.size() == 0) return std::vector<std::string>();
  std::string str = stream;
  std::vector<std::string> out;
  long ocurrence = 0;
  while((ocurrence = server::find(str, f)) >= 0)
  {
    out.push_back(str.substr(0, ocurrence));
    str = str.substr(ocurrence + 1, str.size());
  }
  out.push_back(str);
  return out;
}

std::vector<std::string> server::split(std::string stream, std::string f)
{
  std::string str = stream;
  std::vector<std::string> out;
  long ocurrence = 0;
  while((ocurrence = server::find(str, f)) >= 0)
  {
    out.push_back(str.substr(0, ocurrence));
    str = str.substr(ocurrence + f.size(), str.size());
  }
  if(str.size() != 0) out.push_back(str);
  return out;
}

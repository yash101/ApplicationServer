#include "httpserver.h"
#include "stringproc.h"

server::HttpCookie::HttpCookie()
{}

server::HttpCookie::HttpCookie(std::string value) :
  value(value)
{}

server::HttpResponse::HttpResponse() :
  type(STRING),
  ftype(NULL)
{}

server::HttpPost::~HttpPost()
{
  if(type == FILE && file != NULL)
  {
    fclose(file);
  }
}

server::HttpResponse::~HttpResponse()
{
  if(type == FILE && ftype != NULL)
    fclose(ftype);
}

std::string server::getHTTPTimestamp()
{
  time_t local_time;
  struct tm* Time;

  local_time = time(NULL);
  Time = gmtime(&local_time);

  std::stringstream str;
  str << server::day[Time->tm_wday] << ", ";
  str << Time->tm_mday << " ";
  str << server::months[Time->tm_mon] << " ";
  str << Time->tm_year << " ";
  str << Time->tm_hour << ":";
  str << Time->tm_min << ":";
  str << Time->tm_sec << " GMT";

  return str.str();
}

std::string server::day[] =
{
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat",
  "Sun"
};

std::string server::months[] =
{
  "Jan",
  "Feb",
  "Mar",
  "Apr",
  "May",
  "Jun",
  "Jul",
  "Aug",
  "Sep",
  "Oct",
  "Nov",
  "Dec"
};

std::string server::getHTTPStatus(short int code)
{
  std::map<int, std::string> s;
  s[200] = "OK";
  s[201] = "Created";
  s[202] = "Accepted";
  s[204] = "No Content";
  s[300] = "Multiple Choices";
  s[301] = "Moved Permanently";
  s[302] = "Moved Temporarily";
  s[304] = "Not Modified";
  s[400] = "Bad Request";
  s[401] = "Unauthorized";
  s[403] = "Forbidden";
  s[404] = "Not Found";
  s[500] = "Internal Server Error";
  s[501] = "Not Implemented";
  s[502] = "Bad Gateway";
  s[503] = "Service Unavailable";

  if(s.find(code) == s.end())
  {
    return "Unknown Code";
  }
  return s[code];
}

std::string server::encodeURI(std::string in)
{
  std::stringstream ret;
  char hexBuffer[10];

  for(size_t i = 0; i < in.size(); i++)
  {
    if(in[i] == ' ')
      ret << '+';
    else if(isalnum(in[i]) || in[i] == '-' || in[i] == '_' || in[i] == '.' || in[i] == '~')
      ret << in[i];
    else
    {
      hexBuffer[sprintf(hexBuffer, "%X", in[i])] = '\0';
      if(int(in[i]) < 16) ret << "%0";
      else ret << "%";
      ret << hexBuffer;
    }
  }
  return ret.str();
}

std::string server::decodeURI(std::string in)
{
  std::string ret;
  for(unsigned int i = 0; i < in.size(); i++)
  {
    if ((int) in[i] == 37)
    {
      int new_char;
      sscanf(in.substr(i + 1, 2).c_str(), "%x", &new_char);
      char ch = static_cast<char>(new_char);
      ret += ch;
      i += 2;
    }
    else
    {
      ret += in[i];
    }
  }
  for(unsigned int i = 0; i < ret.size(); i++)
  {
      if(ret[i] == '+') { ret[i] = ' '; }
  }
  return ret;
}

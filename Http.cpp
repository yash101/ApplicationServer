/*
 * Http.cpp
 *
 *  Created on: Nov 14, 2015
 *      Author: yash
 */

#include "HttpServer.h"
#include <string>
#include <vector>
#include <map>
#include <time.h>
#include <sstream>

#define String std::string
#define Vector std::vector
#define ToString std::to_string

static const char* const days[] =
{
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat",
  "Sun"
};

static const char* const months[] =
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

String daf::Http::decodeURI(String in)
{
  String ret;
  for(size_t i = 0; i < in.size(); i++)
  {
    if(in[i] == '%')
    {
      unsigned int new_char;
      sscanf(in.substr(i + 1, 2).c_str(), "%X", &new_char);
      char ch = static_cast<char>(new_char);
      ret += ch;
      i += 2;
    }
    else if(in[i] == '+')
    {
      ret += ' ';
    }
    else
    {
      ret += in[i];
    }
  }

  return ret;
}

String daf::Http::encodeURI(String in)
{
  std::stringstream ret;
  char hexbuf[10];

  for(size_t i = 0; i < in.size(); i++)
  {
    if(in[i] == ' ')
    {
      ret << "+";
    }
    else if(in[i] == '-' || in[i] == '_' || in[i] == '.' || in[i] == '~' || isalnum(in[i]))
    {
      ret << in[i];
    }
    else
    {
      snprintf(hexbuf, sizeof(hexbuf), "%X", in[i]);
      if(int(in[i]) < 16) ret << "%0";
      else ret << "%";
      ret << hexbuf;
    }
  }

  return ret.str();
}

void daf::Http::idecodeURI(String& in)
{
  in = daf::Http::decodeURI(in);
}

void daf::Http::iencodeURI(String& in)
{
  in = daf::Http::encodeURI(in);
}

String daf::Http::timestamp()
{
  time_t localtime;
  struct tm* tme;
  localtime = time(NULL);
  tme = gmtime(&localtime);

  std::stringstream str;
  str << days[tme->tm_wday] << ", ";
  str << tme->tm_mday << " ";
  str << months[tme->tm_mon] << " ";
  str << tme->tm_year << " " << tme->tm_hour << " " << tme->tm_min << " " << tme->tm_sec << " GMT";

  return str.str();
}

static std::map<short int, String> statuscodes;
static bool statuscodes_initialized = false;

String daf::Http::statusString(short int code)
{
  if(!statuscodes_initialized)
  {
    statuscodes[200] = "OK";
    statuscodes[201] = "Created";
    statuscodes[202] = "Accepted";
    statuscodes[204] = "No Content";
    statuscodes[300] = "Multiple Choices";
    statuscodes[301] = "Moved Permanently";
    statuscodes[302] = "Moved Temporarily";
    statuscodes[304] = "Not Modified";
    statuscodes[400] = "Bad Request";
    statuscodes[401] = "Unauthorized";
    statuscodes[403] = "Forbidden";
    statuscodes[404] = "Not Found";
    statuscodes[500] = "Internal Server Error";
    statuscodes[501] = "Not Implemented";
    statuscodes[502] = "Bad Gateway";
    statuscodes[503] = "Service Unavailable";

    statuscodes_initialized = true;
  }

  if(statuscodes.find(code) == statuscodes.end())
  {
    return "Unknown Code";
  }

  return statuscodes[code];
}

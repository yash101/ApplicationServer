#include "core.h"
#include <sstream>

ReturnStatusCode::ReturnStatusCode() :
  _message("A message is being returned"),
  _code(0),
  _source_file(__FILE__),
  _line_number(__LINE__)
{
}

ReturnStatusCode::ReturnStatusCode(const char* message,
                                   const int code,
                                   const char* source_file,
                                   const unsigned long long lnum
) :
  _message(message),
  _code(code),
  _source_file(source_file),
  _line_number(lnum)
{
}

const char* ReturnStatusCode::message()
{
  return _message;
}

const int ReturnStatusCode::code()
{
  return _code;
}

const char* ReturnStatusCode::sourceLocation()
{
  return _source_file;
}

const unsigned long long ReturnStatusCode::lineNumber()
{
  return _line_number;
}

std::string ReturnStatusCode::toString()
{
  std::stringstream str;
  str << "Message: " << _message << std::endl;
  str << "Code: " << _code << std::endl;
  str << "Source Location: " << _source_file << std::endl;
  str << "Line Number: " << _line_number << std::endl;

  return str.str();
}

Exception::Exception()
{
}

Exception::Exception(ReturnStatusCode status) :
  mscde(status),
  _message(status.message())
{
}

ReturnStatusCode& Exception::getStatusCode()
{
  return mscde;
}

const char* Exception::what() const throw()
{
  return _message;
}

Exception::~Exception()
{
}

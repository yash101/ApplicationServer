#ifndef RETURNSTATUSCODE_H
#define RETURNSTATUSCODE_H
#include <exception>
#include <string>

//Easier way to create a ReturnStatusCode. Uses preprocessor to automatically
//	insert file and line
#define StatusCode(msg, code) ReturnStatusCode(msg, code, __FILE__, __LINE__)


//This class can be used to return a message, code, source file and line number.
class ReturnStatusCode
{
private:
  const char* _message;
  const int _code;

  const char* _source_file;
  const unsigned long long _line_number;
public:
  ReturnStatusCode();
  ReturnStatusCode(const char* message,
                   const int code,
                   const char* source,
                   const unsigned long long line_num
  );

  const char* message();
  const int code();
  const char* sourceLocation();
  const unsigned long long lineNumber();

  std::string toString();
};

//General purpose Exception which wraps ReturnStatusCode.
class Exception : public std::exception
{
private:
  ReturnStatusCode mscde;
  const char* _message;
public:
  Exception();
  Exception(ReturnStatusCode status);

  ReturnStatusCode& getStatusCode();

  virtual const char* what() const throw();
  virtual ~Exception();
};
#endif // RETURNSTATUSCODE_H

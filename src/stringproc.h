#ifndef STRINGPROC_H
#define STRINGPROC_H
#include <string>
#include <vector>
#include <sstream>
bool operator==(std::string a, std::string b);
bool operator!=(std::string a, std::string b);

namespace server
{
  long find(std::string in, char f);
  long find(std::string in, std::string f);

  template<typename T> void fromString(T& x, std::string source);

  bool contains(char find, std::string dict);

  std::vector<std::string> split(std::string stream, char f);
  std::vector<std::string> split(std::string stream, std::string f);

  std::string pad(std::string in);
  void ipad(std::string& in);
  std::string lpad(std::string in);
  void ilpad(std::string& in);
  std::string rpad(std::string in);
  void irpad(std::string& in);

  std::string tolower(std::string in);
  void itolower(std::string& in);
  void tolowerf(const char* str);
  std::string toupper(std::string in);
  void itoupper(std::string& in);
  void toupperf(const char* str);

  inline bool getline(std::string& buffer, std::string end, std::istream& stream);

  template<class Type> std::string toString(Type str)
  {
    std::stringstream string;
    string << str;
    return string.str();
  }
}
#endif // STRINGPROC_H

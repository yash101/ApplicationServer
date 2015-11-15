/*
 * stringproc.h
 *
 *  Created on: Nov 11, 2015
 *      Author: yash
 */

#ifndef STRINGPROC_H_
#define STRINGPROC_H_

#include <string>
#include <vector>
#include <sstream>

inline bool operator==(std::string a, std::string b);
inline bool operator!=(std::string a, std::string b);

namespace daf
{
  ssize_t find(std::string in, char f);
  ssize_t find(std::string in, std::string f);

  template<typename T> void fromString(T& x, std::string source);

  bool contains(char find, std::string dict);

  std::vector<std::string> split(std::string stream, char f);
  std::vector<std::string> split(std::string stream, std::string f);

  std::string trim(std::string in);
  void itrim(std::string& in);
  std::string ltrim(std::string in);
  void iltrim(std::string& in);
  std::string rtrim(std::string in);
  void irtrim(std::string& in);

  std::string tolower(std::string in);
  void itolower(std::string& in);
  void tolowerf(char* str);
  std::string toupper(std::string in);
  void itoupper(std::string& in);
  void toupperf(char* str);

  inline bool getline(std::string& buffer, std::string end, std::istream& stream);
}

#endif /* STRINGPROC_H_ */

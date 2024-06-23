#ifndef ZUKO_CGI_UTILS_H_
#define ZUKO_CGI_UTILS_H_
#include <string>
#include <vector>

std::string lowercase(const std::string& str);
void strip_spaces(std::string& str);
std::vector<std::string> split(std::string s,const std::string& x);
int hexdigit_to_decimal(char ch);
std::string url_decode(const std::string& s);

#endif
#ifndef ZUKO_MULTIPART_H_
#define ZUKO_MULTIPART_H_
#include <exception>
#include <string>
#include <vector>
#include "zapi.h"


std::vector<std::string> split_ign_quotes(const std::string&,char);
class parse_error : public std::exception
{
    public:
        const char* what() { return "Bad or unsupported format of multipart form.";}
};
class multipart_parser
{
private:
    std::string B;
    size_t k = 0;
    char* data;
    size_t len;
    //
    std::string part_name;
    std::string part_filename;
    std::string part_content_type;
    size_t content_begin;
    size_t content_length;
    bool had_error = false;
    std::string err_msg;
    //
    void die(const std::string& msg);
    void consume(char ch);
    std::string header();
    std::vector<std::string> header_value();
    void space();
    void part();
    zobject build_part();
    zdict* mp();
    void boundary();
public:
    multipart_parser(char* data,size_t len,const std::string& boundary);
    zdict* parse();
};
#endif
#include <cstdio>
#include <string.h>
#include <string>
#include <vector>
#include "multipart.h"
#include "cgi-utils.h"
using namespace std;

/*
mp -> boundary CRLF part CRLF boundary-- | boundary CRLF part CRLF mp

Assuming there is atleast 1 header in each part( the Content-Disposition header)
part -> (header CRLF)+ CRLF content

header -> name colon header_value

*/
extern zclass* part_class;

multipart_parser::multipart_parser(char* data,size_t len,const std::string& boundary)
{
    this->B = boundary;
    this->data = data;
    this->len = len;
    k = 0;
}
void multipart_parser::die(const std::string& msg)
{
    throw parse_error();
}
void multipart_parser::consume(char ch)
{
    if(k >= len || data[k]!=ch)
        die("Bad or unnsupported multipart format");
    k++;
}
vector<string> split_ign_quotes(const string& s,char x)
{
    string read;
    vector<string> parts;
    bool inquotes = false;
    for(auto e: s)
    {
        if(e == '"')
        {
            inquotes = !inquotes;
            read+='"';
        }
        else if(e == x)
        {
            parts.push_back(read);
            read="";
        }
        else
            read+=e;
    }
    parts.push_back(read);
    return parts;
}
zobject multipart_parser::build_part()
{
    if(part_name == "")
        die("Unsupported or bad format. Missing part name.");
    
    if(part_content_type == "")
        part_content_type = "text/plain"; // assume text type
    //filename is optional
    bool isfile = (part_filename != "");
    
    if(part_content_type == "text/plain") //text content type
    {
        zstr* text = vm_alloc_zstr(content_length);
        memcpy(text->val,data+content_begin,content_length);
        return zobj_from_str_ptr(text);
    }
    else //just add content to a bytearray
    {
        zbytearr* bytearray = vm_alloc_zbytearr();
        zbytearr_resize(bytearray,content_length);
        memcpy(bytearray->arr,data + content_begin, content_length);
        //
        zclass_object* obj = vm_alloc_zclassobj(part_class);
        zclassobj_set(obj,"bytes",zobj_from_bytearr(bytearray));
        zclassobj_set(obj,"type",zobj_from_str(part_content_type.c_str()));
        if(isfile)
            zclassobj_set(obj,"filename",zobj_from_str(part_filename.c_str()));
        return zobj_from_classobj(obj);
    }
}
zdict* multipart_parser::mp()
{
    zdict* form = vm_alloc_zdict();
    while(k < len)
    {
        boundary();
        consume('\r');
        consume('\n');
        part();
        consume('\r');
        consume('\n');
        size_t dup = k;
        boundary();

        if(part_name == "")
            die("Unsupported or bad format. Missing part name");
        
        zdict_emplace(form,zobj_from_str(part_name.c_str()),build_part());
        
        if(k+1 < len && data[k] == '-' && data[k+1] == '-') // this was last boundary
        {
            k+=2;
            break;
        }
        else
            k = dup;
    }
    return form;
}
void multipart_parser::boundary()
{
    if(k+B.length()-1 < len && strncmp(data+k,B.c_str(),B.length()) == 0)
    {
        k+=B.length();
    }
    else
        die("Bad or unsupported format. Missing boundary");

}
void multipart_parser::part()
{
    part_name = "";
    part_filename = "";
    part_content_type = "";
    content_begin = 0;
    content_length = 0;
    header();
    consume('\r');
    consume('\n');
    while(isalpha(data[k]))
    {
        header();
        consume('\r');
        consume('\n');
    }
    consume('\r');
    consume('\n');
    //content
    content_begin = k;
    std::string tmp = "\r\n"+B;
    while(k < len)
    {
        if(data[k] == tmp[0] && k+tmp.length()-1 < len && strncmp(data+k,tmp.c_str(),tmp.length()) == 0)
            break;

        k++;
    }
    content_length = k - content_begin;
}

void multipart_parser::space()
{
    while(k < len && data[k] == ' ')
        k++;
}
std::vector<std::string> multipart_parser::header_value()
{
    std::vector<std::string> values;
    bool inquotes = false;
    std::string val;
    while(k<len && data[k]!='\r')
    {
        if(data[k] == '"')
        {
            inquotes = !inquotes;
            val += data[k];
        }
        else if(data[k] == ';')
        {
            if(k+1 > len || data[k+1] != ' ' || val == "")
                die("Bad or unsupported format");
            k+=1;
            values.push_back(val);
            val = "";
        }
        else
        {
            val += data[k];
        }
        k++;
    }
    if(val == "" || inquotes)
        die("Bad or unsupported format");
    values.push_back(val);
    return values;
}
std::string multipart_parser::header()
{
    std::string name;
    while(k<len && (isalpha(data[k]) || data[k] == '-' || isdigit(data[k]) || data[k] == ' '))
    {
        if(data[k]!=' ')
            name += data[k];
        k++;
    }
    consume(':');
    space();
    vector<string> values = header_value();

    // Inspect header
    if(lowercase(name) == "content-disposition")
    {
        for(auto value: values)
        {
            vector<string> parts = split_ign_quotes(value,'=');
            if(parts.size() == 1)
            {
                if(lowercase(parts[0])!="form-data")
                    die("Bad or unsuported format. Unknown value of header Content-Disposition");
            }
            else if(parts.size() == 2)
            {
                if(parts[0] == "name" && part_name == "")
                {
                    part_name = parts[1];
                    if(part_name.length() > 2 && part_name[0] == '"' && part_name.back()=='"')
                    {
                        part_name.pop_back();
                        part_name.erase(0,1);
                    }
                }
                else if(parts[0] == "filename" && part_filename == "")
                {
                    part_filename = parts[1];
                    if(part_filename.length() > 2 && part_filename[0] == '"' && part_filename.back()=='"')
                    {
                        part_filename.pop_back();
                        part_filename.erase(0,1);
                    }
                }
                else
                    die("Bad or unsuported format");
            }
            else
                die("Bad or unsuported format. Invalid value of header Content-Disposition");
        }
    }
    else if(lowercase(name) == "content-type")
    {
        if(values.size()!=1 || values[0] == "" || part_content_type!="")
            die("Bad or unsupported format!");
        part_content_type = values[0];
    }
    else
        die("Unknown Header used. Bad or unsupported format of multipart request");
    return ""; //returns complete header A: B
}
zdict* multipart_parser::parse()
{
    return mp();
}

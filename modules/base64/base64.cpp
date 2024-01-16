#include "base64.h"
#include <string>

ZObject nil;
char base64Table[64];

ZObject init()
{
    nil.type = Z_NIL;
    Module* m = vm_allocModule();
    m->name = "base64";
    Module_addNativeFun(m,"encode",&ENCODE);
    Module_addSigNativeFun(m,"decode",&DECODE,"s");
    //init base64Table
    size_t k = 0;
    for(char i='A';i<='Z';i++)
        base64Table[k++] = i;
    for(char i='a';i<='z';i++)
        base64Table[k++] = i;
    for(char i='0';i<='9';i++)
        base64Table[k++] = i;    
    base64Table[62] = '+';
    base64Table[63] = '/';
    return ZObjFromModule(m);
}
//
char reverse_base64(char ch)
{
  if(ch >= 'A' && ch <= 'Z')
    return ch - 'A';
  else if(ch >= 'a' && ch <= 'z')
    return ch - 'a' + 26;
  else if(ch >= '0' && ch <= '9')
    return ch - '0' + 52;
  else if(ch == '+')
    return 62;
  else if(ch == '/')
    return 63;
  return 69;
}
std::string base64_encode(const char* blob,size_t len)
{

  std::string res;
  char prevChunk = 0;
  size_t prevChunkSize = 0; // number of bits in prev Chunk
  for(size_t k = 0; k<len;k++)
  {
    if(prevChunkSize == 0)
    {
      char ch = (blob[k] & 0xfc) >> 2; // get first 6 bits
      res += base64Table[ch];
      prevChunk = (blob[k] & 0x03); // set prevChunk 
      prevChunkSize = 2;
    }
    else 
    {
      size_t curr = 6 - prevChunkSize; //number of bits to get from current byte
      char mask = 0xff;
      mask <<= (8-curr);
      char ch = (((blob[k] & mask) >> (8-curr)) | (prevChunk << curr));
      res += base64Table[ch];

      char remainingMask = 0xff >> (curr);
      prevChunkSize = 8-curr;
      prevChunk = blob[k] & remainingMask;
      if(prevChunkSize == 6)
      {
        res += base64Table[prevChunk];
        prevChunkSize = 0;
        prevChunk = 0;
      }
    }
  }
  if(prevChunkSize != 0)
  {
    if(prevChunkSize == 6)
      res += base64Table[prevChunk];
    else if(prevChunkSize == 2)
    {
      res += base64Table[prevChunk<<4];
      res += "==";
    }
    else if(prevChunkSize == 4)
    {
      res += base64Table[prevChunk<<2];
      res += '=';
    }
  }
  return res;
}
ZByteArr* base64_decode(const char* str,size_t len)
{
  if(len == 0)
    return nullptr;
  size_t padding = 0;
  if(str[len-1] == '=')
  {
    len--;
    padding += 2;
  } 
  if(len == 0)
    return nullptr;
  if(str[len-1] == '=')
  {
    len--;
    padding += 2;
  } 
  if(len == 0)
    return nullptr;

  char mask = 0x3f; //initial mask
  size_t currBits = 6;
  ZByteArr* res = vm_allocByteArray();

  for(size_t k=0;k<len-1;k++)
  {
    char val = reverse_base64(str[k]);
    size_t bitsToBorrow = 8 - currBits;
    char borrowMask = (0x3f << (6 - bitsToBorrow)) & 0x3f; // highest 2 bits are 00 if it's valid base64
    char ch =  ((val & mask) << bitsToBorrow);
    
    if(bitsToBorrow != 0)
      ch |= ((reverse_base64(str[k+1]) & borrowMask) >> (6 - bitsToBorrow));
    //calculate next mask and curr bits
    if(bitsToBorrow == 6) // borrowed 6 bits from next byte already, so it's useless,skip it
    {
        k++;
        currBits = 6;
        mask = 0x3f;
    }
    else
    {
      currBits = 6 - bitsToBorrow;
      mask = (0x3f >> bitsToBorrow);
    }
    ZByteArr_push(res,ch);
  }
  return res;
}
//
ZObject ENCODE(ZObject* args,int32_t n)
{
  if(n!=1)
    return Z_Err(ArgumentError,"1 argument needed!");
  const char* val;
  size_t len;
  if(args[0].type == Z_STR)
  {
    auto str = AS_STR(args[0]);
    val = str->val;
    len = str->len;
  }
  else if(args[0].type == Z_BYTEARR)
  {
    auto arr = AS_BYTEARRAY(args[0]);
    val = (const char*)arr->arr;
    len = arr->size;
  }
  else
    return Z_Err(TypeError,"Argument must be a string or bytearray!");
  std::string res = base64_encode(val,len);
  return ZObjFromStr(res.c_str());
}

ZObject DECODE(ZObject* args,int32_t n)
{
  ZStr* str = AS_STR(args[0]);
  ZByteArr* arr = base64_decode(str->val,str->len);
  if(!arr)
    return Z_Err(Error,"Invalid base64! Unable to decode!");
  return ZObjFromByteArr(arr);
}
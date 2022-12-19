#ifndef VAR_H_
#define VAR_H_
#include "PltObject.h"

using namespace std;

string replace(int startpos,int endpos,string x,string s)
{
  string p1,p2,p3;
  if(startpos!=0)
  {
     p1 = substr(0,startpos-1,s);
  }
  p2 = x;
  if(endpos!=len(s))
  {
    p3 = substr(endpos+1,len(s),s);
  }
  s = p1+p2+p3;
  return s;
}
string replace_all(string x,string y,string s)//replaces all x strings in s with string y
{
	int  startpos = 0;
	while((size_t)startpos<s.length())
	{
		if(s[startpos]==x[0] && substr(startpos,startpos+len(x),s)==x)
		{
		  s = replace(startpos,startpos+len(x),y,s);
		  startpos+=len(y)+1;
		  continue;
	    }
		startpos+=1;
	}
	return s;
}

string unescape(string s)
{
    //This function replaces real escape sequences with printable ones
    s = replace_all("\\","\\\\",s);
    s = replace_all("\n","\\n",s);
    s = replace_all("\r","\\r",s);
    s = replace_all("\v","\\v",s);
    s = replace_all("\t","\\t",s);
    s = replace_all("\a","\\b",s);
  	s = replace_all("\"","\\\"",s);
	  return s;
}

string PltObjectToStr(const PltObject& a)
{
        string IntToHex(int);
        if(a.type=='i')
          return str(a.i);
        else if(a.type=='l')
          return str(a.l);
        else if(a.type=='f')
          return str(a.f);
        else if(a.type=='m')
          return IntToHex(a.i);
        else if(a.type=='e')
        {
          return "<Error Object>";
        }
        else if(a.type=='b')
        {
            if(a.i)
                return "true";
            return "false";
        }
        else if(a.type=='u')
        {
            char buff[100];
            snprintf(buff,100,"<Opened File at %p>",a.ptr);
            string s = buff;
            return s;
        }
				else if(a.type=='w')
				{
          FunObject* p = (FunObject*)a.ptr;
					return "<Function "+p->name +">";
				}
        else if(a.type=='y')
          return "<Native Function>";
				else if(a.type=='v')
				  return "<Class "+*(string*)a.ptr+">";
			  else if(a.type=='o')
        {
				  return "<"+((KlassInstance*)a.ptr) -> klass->name+" Object "+to_string((long long int)a.ptr)+" >";
        }
        else if(a.type=='q')
        {
            char buff[100];
            snprintf(buff,100,"<Module Object at %p>",a.ptr);
            string s = buff;
            return s;
        }
        else if(a.type=='r')
        {
          return "<Native Method>";
        }
        else if(a.type=='z')
        {
          return "<Coroutine Object>";
        }
        else if(a.type=='g')
        {
          return "<Coroutine>";
        }
        else if(a.type=='j')
        {

            string PltListToStr(PltList*,vector<void*>* = nullptr);
            PltList* p = (PltList*)a.ptr;
            return PltListToStr(p);
        }
        else if(a.type=='s')
        {
          return *(string*)a.ptr;
        }
        else if(a.type=='a')
        {
            //printf("returning PltObjectToStr of dictionary\n");
						string DictToStr(Dictionary*,vector<void*>* = nullptr);
            Dictionary* p = (Dictionary*)a.ptr;
            return DictToStr(p);
        }
        return "nil";
    }

string PltListToStr(PltList* p,vector<void*>* prev=nullptr)
{
  PltList l = *p;
  vector<void*> seen;
  if(prev!=nullptr)
    seen = *prev;
	seen.push_back(p);
  string res="[";
  for(size_t k=0;k<l.size();k+=1)
  {
      if(l[k].type=='j')
      {
         if( std::find(seen.begin(), seen.end(), l[k].ptr) != seen.end())
           res+="[...]";
         else
         {
            res+=PltListToStr((PltList*)l[k].ptr,&seen);
         }
      }
			else if(l[k].type=='a')
      {
				 string DictToStr(Dictionary*,vector<void*>* = nullptr);

         if( std::find(seen.begin(), seen.end(), l[k].ptr) != seen.end())
           res+="{...}";
         else
         {
            res+=DictToStr((Dictionary*)l[k].ptr,&seen);
         }
      }
			else if(l[k].type=='s')
			{
				res+= "\""+unescape(*(string*)l[k].ptr)+"\"";
			}
      else
      {
        res+=PltObjectToStr(l[k]);
      }
      if(k!=l.size()-1)
        res+=",";
  }
  return res+"]";
}

string DictToStr(Dictionary* p,vector<void*>* prev=nullptr)
{

    string res = "{";
    size_t k = 0;
		vector<void*> seen;
		if(prev!=nullptr)
		  seen = *prev;
		seen.push_back((void*)p);
		Dictionary d = *p;
    for(auto e: d)
    {
       PltObject key = e.first;
       PltObject val = e.second;
       if(key.type=='s')
          res+= "\""+unescape(*(string*)key.ptr)+"\"";
       else if(key.type=='a')
			 {
				 if( std::find(seen.begin(), seen.end(), key.ptr) != seen.end())
					 res+="{...}";
				 else
				 {
						res+=DictToStr((Dictionary*)key.ptr,&seen);
				 }
			 }
			 else if(key.type=='j')
			 {
				 if(std::find(seen.begin(),seen.end(),key.ptr)!=seen.end())
				   res+="[...]";
				 else
				   res+=PltListToStr((PltList*)key.ptr,&seen);
			 }
			 else
          res+=PltObjectToStr(key);
					/////////////////
       res+=(" : ");
			 //////////////
       if(val.type=='s')
          res+= "\""+unescape(*(string*)val.ptr)+"\"";
			 else if(val.type=='a')
			 {
				 if( std::find(seen.begin(), seen.end(), val.ptr) != seen.end())
					 res+="{...}";
				 else
				 {
						res+=DictToStr((Dictionary*)val.ptr,&seen);
				 }
			 }
			 else if(val.type=='j')
			 {
				 if(std::find(seen.begin(),seen.end(),val.ptr)!=seen.end())
					 res+="[...]";
				 else
					 res+=PltListToStr((PltList*)val.ptr,&seen);
			 }
       else
          res+=PltObjectToStr(val);
       if(k!=d.size()-1)
         res+=(",");
       k+=1;
    }
    res+="}";
   return res;
}

#endif

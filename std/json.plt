#  JSON Library Plutonium
#  3 May 2022 Written by Shahryar Ahmad
#  The code is free to use and modify and comes without any warrantee.
#  Do not edit unless you really know what you are doing!
namespace json
{
    class Token
    {
        var type = nil
        var content = nil
        function __construct__(var t,var c)
        {
            self.type = t
            self.content = c
        }
    }
    function generateTokens(var str)
    {
        var tokens = []
        var l = len(str)
        for(var i=0 to l-1 step 1)
        {
            var c = str[i]

            if(c=="\"")
            {
              i+=1
              var j = i
              var terminated = false
              while(j<l)
              {
                  if(str[j]=="\"")
                  {
                      terminated = true
                      break
                  }
                  j+=1
              }
              if(!terminated)
              {
                  println("SyntaxError string not terminated!")
                  exit()
              }
              tokens.push(Token("str",substr(i,j-1,str)))
              i = j
            }
            else if(isnumeric(c))
            {
                var j = i
                var decimal = false
                while(j<l)
                {
                  if(str[j]=="." and !decimal)
                  {
                    decimal = true
                  }
                  if(!isnumeric(str[j]))
                  {
                     break
                  }
                  j+=1
                }
                if(str[j-1]==".") # floats like 2. are not considered valid it must be 2.0
                {
                   throw @ValueError,"Invalid JSON"
                }
                if(decimal)
                  tokens.push(Token("float",float(substr(i,j-1,str))))                  
                else
                  tokens.push(Token("num",int(substr(i,j-1,str))))
                i = j
                continue
            }
            else if(isalpha(c))
            {
                var j = i
                while(j<l)
                {
                  if(!isalpha(str[j]))
                  {
                     break
                  }
                  j+=1
                }
                var id = substr(i,j-1,str)
                if(id=="true")
                {
                  tokens.push(Token("bool",true))
                }
                else if(id=="false")
                  tokens.push(Token("bool",false))
                else if(id=="null")
                {
                    tokens.push(Token("nil",nil))
                }
                else
                {
                   throw @ValueError,"Invalid JSON"
                }

                i = j
                continue
            }
            else if(c==",")
            {
                tokens.push(Token("comma",","))
            }
            else if(c=="[")
            {
                tokens.push(Token("lsb","["))
            }
            else if(c=="]")
            {
                tokens.push(Token("rsb","]"))
            }
            else if(c=="{")
            {
                tokens.push(Token("lcb","{"))
            }
            else if(c=="}")
            {
                tokens.push(Token("rcb","}"))
            }
            else if(c==":")
            {
                tokens.push(Token("col",":"))
            }
            else if(c==" " or c=="\t" or c=="\n" or c=="\r")
            {
                #just ignore the space
            }
            else
            {
                throw @ValueError,"Invalid JSON"
            }
        }
        return tokens
    }
    function match_lsb(var start,var tokens)
    {
        var I = 0
        var l = len(tokens)-1
        for(var i=start to l step 1)
        {
            if(tokens[i].type=="lsb")
            {
                I+=1
            }
            else if(tokens[i].type=="rsb")
            {
                I-=1
                if(I==0)
                  return i
            }
        }
        return nil
    }
    function match_lcb(var start,var tokens)
    {
        var I = 0
        var l = len(tokens)-1
        for(var i=start to l step 1)
        {
            if(tokens[i].type=="lcb")
            {
                I+=1
            }
            else if(tokens[i].type=="rcb")
            {
                I-=1
                if(I==0)
                  return i
            }
        }
        return nil
    }
    function isValTok(var tok)
    {
        return (tok.type=="str" or tok.type=="num" or tok.type=="float" or tok.type=="nil")
    }
    function extract_list(var i,var j,var tokens)
    {

        var list = []
        if(j==i+1)
          return list
        var elem = []
        for(var k=i+1 to j-1 step 1)
        {
            if(tokens[k].type=="comma")
            {
                if(len(elem)!=1)
                {
                   throw @ValueError,"Invalid JSON"
                }
                if(typeof(elem[0])=="List")
                  list.push(elem[0])
                else
                  list.push(elem[0].content)
                elem.clear()
            }
            else if(isValTok(tokens[k]))
            {
              elem.push(tokens[k])
            }
            else if(tokens[k].type=="lsb")
            {
                var end = match_lsb(k,tokens)
                if(end==nil or end==j)
                {
                   throw @ValueError,"Invalid JSON"
                }
                var sublist = extract_list(k,end,tokens)
                elem.push(sublist)
                k = end
            }
            else
            {
                throw @ValueError,"Invalid JSON"
            }
        }
        if(len(elem)!=1)
        {
            println("SyntaxError")
            exit()
        }
        if(typeof(elem[0])=="List")
            list.push(elem[0])
        else
            list.push(elem[0].content)
        return list
    }
    function fromTokens(var start,var end,var tokens)
    {
        var l = end-start+1
        var obj = {}
        if(l==0)
          return obj # return empty dictionary
        if(tokens[start].type!="lcb" or tokens[end].type!="rcb")
        {
            throw @ValueError,"Invalid JSON"
        }
        if(l==2) # empty json {}
          return obj
        var key = nil
        var val = nil
        for(var i=start+1 to end-1 step 1)
        {
            if(i+2 > end-1)
            {
                   throw @ValueError,"Invalid JSON"
            }
            key = tokens[i]
            if(key.type!="str")
            {
                   throw @ValueError,"Invalid JSON"
            }
            i+=1
            if(tokens[i].type!="col")
            {
                   throw @ValueError,"Invalid JSON"
            }
            i+=1
            val = tokens[i]
            if(val.type=="col" or val.type=="comma"  or val.type=="rcb" or val.type=="rsb")
            {
                   throw @ValueError,"Invalid JSON"
            }
            if(val.type=="lsb")
            {
                var e = match_lsb(i,tokens)
                if(e==nil or e==end)
                {
                   throw @ValueError,"Invalid JSON"
                }
                var list = extract_list(i,e,tokens)
                obj.emplace(key.content,list)
                i = e
            }
            else if(val.type=="lcb")
            {
                var e = match_lcb(i,tokens)
                if(e==nil or e==end)
                {
                   throw @ValueError,"Invalid JSON"
                }
                var subobject = fromTokens(i,e,tokens)
                obj.emplace(key.content,subobject)
                i = e
            }
            else
              obj.emplace(key.content,val.content)
            i+=1
            if(i> end-1)
            {
                # no more key/value pairs
            }
            else
            {
              if(tokens[i].type!="comma")
              {
                   throw @ValueError,"Invalid JSON"
              }
              if(i==end-1) # comma can't be at the end
              {
                   throw @ValueError,"Invalid JSON"
              }
            }
        }
        return obj
    }
    function loads(var str) # makes Dictionary from json string
    {
        var tokens = generateTokens(str)
        return fromTokens(0,len(tokens)-1,tokens)
    }
    function jsonStringFromDict(var obj,var indent)
    {
      var list = obj.asList()
        var l = len(list)-1
        var indentSpace = ""
        for(var i=1 to indent step 1)
          indentSpace+=" "
        var str = indentSpace+"{\n"
        var end = indentSpace+"}"
        indentSpace+=" "
        for(var i=0 to l step 1)
        {
           var pair = list[i]
           if(typeof(pair[0])!="String")
             throw @ValueError,"Can't convert dictionary to json"
           if(typeof(pair[1])=="Dictionary")
           {
             str+=indentSpace+"\""+pair[0]+"\": \n"+ jsonStringFromDict(pair[1],indent+2)
           }
           else if(typeof(pair[1])=="nil")
           {
             str+=indentSpace+"\""+pair[0]+"\": null"
           }
           else if(typeof(pair[1])=="Boolean")
           {
             if(pair[1])
              str+=indentSpace+"\""+pair[0]+"\": true"
             else
              str+=indentSpace+"\""+pair[0]+"\": false"
 
           }
           else if(typeof(pair[1])=="String")
             str+=indentSpace+"\""+pair[0]+"\": \""+ pair[1]+"\""
           else
             str+=indentSpace+"\""+pair[0]+"\": "+ str(pair[1])
          if(i!=l)
            str+=","
          str+="\n"
        }
        str+=end
        return str
    }
    function dumps(var obj) # Generates a json string from dictionary
    {
        return jsonStringFromDict(obj,0)
    }
}

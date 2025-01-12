function generateTokens(var str)
{
  var k = 0
  var tokens = []
  while(k<len(str))
  {
    if(str[k]=="+" or str[k]=="-")
    {
      tokens+=[{"op": str[k]}]
    }
    else if(isnumeric(str[k]))
    {
      var num = str[k]
      var j = k+1
      while(j<len(str))
      {
        if(isnumeric(str[j]))
        {
          num+=str[j]
        }
        else
          break
        j+=1
      }
      tokens+=[{"num": num}]
      k = j
      continue
    }
    else
    {
      println("SyntaxError in expression!")
      exit()
    }
    k+=1
  }
  return tokens
}
var src = "50+20-4"
var tokens = generateTokens(src)
println(tokens)

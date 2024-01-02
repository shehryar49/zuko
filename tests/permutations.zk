#Note: This is not the best way to generate permutations!
#It's just a test and was used for performance benchmarking as well.
#list+=[x] expands as list = list + [x]
#which allocates a new list and stores the result of concatenation
#list.push() is better because it does not allocate a new list instead it just adds a
#new element
var calls = 0
function permute(var str,var k)
{
  calls+=1
  if(k==1)
  {
    var permutations = []
    var l = 0
    while(l<len(str))
    {
      permutations+=[str[l]]
      l+=1
    }
    return permutations
  }
  var l = 0
  var permutations = []
  while(l<len(str))
  {
    var sub = permute(substr(0,l-1,str)+substr(l+1,len(str)-1,str),k-1)
    var j = 0
    while(j<len(sub))
    {
      sub[j] = str[l]+sub[j]
      j+=1
    }
    permutations+=sub
    l+=1
  }
  return permutations
}

println(permute("abcdef",6))
println(calls," function calls")

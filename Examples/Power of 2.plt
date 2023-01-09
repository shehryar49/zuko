function Check(var c)
{
  if(c==0)
  return 0
  while(c!=1)
  {
    if(c%2!=0 )
    return 0
   
     c=c/2
  }
  return 1

}

println("Enter Number You want to check : ")
var n = int (input())
if(Check(n)==1)
println(" The Given Number is a Power of Two ")
else
println(" The Given Number is not a Power of Two ")
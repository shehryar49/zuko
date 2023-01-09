function Check(var c)
{
  if(c==0)
  return 0
  var new = c
  c=c-1
  new=new&c
  if(new==0)
  return 1
  else
  return 0
}

println("Enter Number You want to check : ")
var n = int (input())
if(Check(n)==1)
println(" The Given Number is a Power of Two ")
else
println(" The Given Number is not a Power of Two ")
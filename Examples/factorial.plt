var n=0
var fact=1
println("Enter the number n for factorial:")
n= int(input())
if(n<0)
{
    println("Factorials for negative numbers don't exist!")
}
else if(n==0 or n==1)
{
    println(n,"! = 1")
}
else
{
    for(var i=2 to n step 1)
    {
   fact=fact*i
    }

 println("The factorial of the number is:\n",n,"! = ",fact)
}


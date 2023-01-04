var num1=0
var num2=0
var gcd=0
println("Enter any two number:")
num1= int(input())
num2= int(input())
var smaller=0
if(num1>num2)
smaller=num2
else
smaller=num1
for(var i=1 to smaller step 1)
{
    if(num1%i==0 and num2%i==0)
    {
        gcd=i
    }
}

println("The greatest common divisor is:",gcd)

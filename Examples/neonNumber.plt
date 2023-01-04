#A neon number is a number where the sum of digits of square of the number is equal to
#the number itself. Forexample, 9 is a neon number. Its square is 9 * 9 = 81 and sum of the digits (8 + 1) is 9. 


var num=0

println("Enter a number:")
num=int(input())

var temp=num
var digit=0
var square=pow(temp,2)
var sum=0

while(temp>0)
{
   digit=temp%10
   temp/=10
   sum+=digit
   
}

if(sum==num)
   println("Neon number")
else
   println("Not a Neon number")

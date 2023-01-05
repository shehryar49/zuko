#An Armstrong number of three digits is an integer such that the sum of the cubes of its digits is equal to the number itself. 
#For example, 371 is an Armstrong number since 3**3 + 7**3 + 1**3 = 371.

var num=0

println("Enter a three digit number")
num=int(input())

var temp=num
var d1=temp%10
temp/=10
var d2=temp%10
var d3=temp/10
var Armstrong= pow(d1,3)+pow(d2,3)+pow(d3,3)
if(num==Armstrong)
   println("Armstrong number")
else
   println("Not an Armstrong number")

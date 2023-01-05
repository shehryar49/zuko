

#Program that checks whether a number is positive, negative or zero.

var num=0
println("Enter any number:")
num= int(input())
if(num==0)
println("The number is Zero")
else if(num>0)
println("The number is Positive")
else if(num<0)
println("The number is Negative")
else
println("Invalid Input")

#actually, there is no need for the last else condition
#because
#plutonium is effiecient enough to raise error if user input char or string
#and treats floats as int when mentioned int(input())
#as same in cpp 

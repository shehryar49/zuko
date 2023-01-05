
#A program that calculates the average of array values

var num=0
var list=[]
println("Enter the numbers of array")
num=int(input())
println("Enter the numbers in the array")
for(var i=0 to num-1 step 1)
{
    var x=int(input())
    list.push(x)
}
var sum=0
for(var i=0 to num-1 step 1)
{
    sum+=list[i]
}

println("The average of the list numbers is =",sum/num)


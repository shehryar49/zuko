#Given an array of integers nums and an integer target
#Checks for indices of the two numbers such that they add up to target.
#and retuns two dimensional array that stores all those indices.
#the return list will be empty if indices not found.
#also print how many pairs you found for that target

 var array=[3,4,9,8,7,1,2,5,0,10]
 var target=10


function indices(var array, var target)
{
   var first=0
   var count=0
   var list=[]
   for(var i=0 to len(array)-1 step 1)
   {
      first=array[i]
      for(var j=i+1 to len(array)-1 step 1)
      {
         if(first+array[j]==target)
         {
            list.push([i,j])
         }
      }
   }
   return list
}
var matrix=indices(array,target)
println("The indices pairs that sum up to the Target = ",len(matrix),"\n",matrix)

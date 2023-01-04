#Given an array of integers nums and an integer target
#Return indices of the two numbers such that they add up to target.
#and retuns [-1,-1] if indices not found.
#Example:

#Input: nums = [2,7,11,15], target = 9
#Output: [0,1]
#Explanation: Because nums[0] + nums[1] == 9, we return [0, 1]

 var array=[3,4,9,8,7,1,2,5,10]
 var target=10


function indices(var array, var target)
{
   var first=0
   for(var i=0 to len(array)-1 step 1)
   {
      first=array[i]
      for(var j=i+1 to len(array)-1 step 1)
      {
         if(first+array[j]==target)
         {
            var indices=[i,j]
            return indices
         }
      }
   }
   return [-1,-1]
}

println(indices(array,target))

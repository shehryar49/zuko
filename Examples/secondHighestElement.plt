#program to find the second highest interger in an array 

#given array
 
 var array=[3,4,9,8,7,1,2,5,110,10,12]
 
 var size=len(array)

 println("The array : ",array)

#using simple bubble sort algorithm 


function sort(var array, var n)
{
   var temp=0
   for(var i=0 to n-1 step 1)
   {     var flag=0
      for(var j=0 to ((n-2)-i) step 1)
      {
         if(array[j]>array[j+1])
         {
            temp=array[j]
            array[j]=array[j+1]
            array[j+1]=temp
            flag=1
         }
      }
      if(flag==0)
      {  println("The sorted array : ", array)
         return array
      }
   }
        println("The sorted array : ", array)
        
  return array
}

function secondHighest(var array, var size)
{
   var tempArray=sort(array,size)
   return tempArray[size-2]
}

println("The second Highest Integer of the array is = ", secondHighest(array,size))

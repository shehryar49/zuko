#program to find the second highest interger in an array 

#given array
 
     var array=[3,4,9,8,7,1,2,5,0,69,110,10,12]

     var size=len(array)

     if(size==0 or size==1)
    {
     #there is no point of array of this size
     
     println("Size must be greater than or equal to 2")
     exit()
    }
 
println("The array : ",array)

#using simple bubble sort algorithm 

function sort(var array, var size)
{
   var temp=0
   
   for(var i=0 to size-1 step 1)
   {     
          var flag=0
      
      for(var j=0 to ((size-2)-i) step 1)
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
         break
   }
   
}

#function that returns the second last element of sorted array

function secondHighest(var array, var size)
{
   sort(array,size)
   println("The sorted array : ", array)
   return array[size-2]
}

println("The second Highest Integer of the array is = ", secondHighest(array,size))


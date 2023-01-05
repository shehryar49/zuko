var end=0
println("Enter number of values you want to print from Fibonacci series")
end=int(input())
var pre=0 
var next=1 
var combo=0
var count=0

for(var i=0 to end step 1)
    {
        if(i==0)
        {
        print(pre," ")
        count=count+1
        }
        
        else if(i==1)
        {
        print(next," ")
        count=count+1  
        }
            
       combo=pre+next  
       pre=next
       next=combo
       if(count<end)
       {
       print(combo," ")
       count=count+1
       }
       else
       break
       
    }

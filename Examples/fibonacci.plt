var end=0
println("Enter the limiting number for Fibonnaci")
end=int(input())
var pre=0 
var next=1 
var combo=0
    
for(var i=0 to end step 1)
    {
        if(i==0)
        print(pre," ")
       
        else if(i==1)
       print(next," ")
       
       combo=pre+next  
       pre=next
       next=combo
       if(combo<=end)
       print(combo," ")
       else
       break
       
    }

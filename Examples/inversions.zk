function __merge(var arr,var l,var mid,var h)
{
    var temp = []
    var i = l
    var j = mid+1
    var inv=0
    while(i<=mid and j<=h)
    {
        if(arr[i]<= arr[j])
        {
          temp.push(arr[i])
          i+=1
        }
        else
        {
          temp.push(arr[j])
          inv+=mid-i+1
          j+=1
        }
    }
    while(i<=mid)
    {
        temp.push(arr[i])
        i+=1
    }
    while(j<=h)
    {
        temp.push(arr[j])
        j+=1
    }
    for(var i=0 to len(temp)-1 step 1)
    {
        arr[l+i] = temp[i]
    }
    return inv
} 
function countInv(var arr,var l,var h)
{
    if(l<h)
    {
        var mid = (l+h)/2
        var inv=countInv(arr,l,mid)
        inv+=countInv(arr,mid+1,h)
        inv+=__merge(arr,l,mid,h)
        return inv
    }
    return 0
}
var arr = [3,2,1,100]
var inv = countInv(arr,0,3)
println(inv)

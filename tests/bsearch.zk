function binary_search(var arr,var x)
{
  var l = 0
  var h = len(arr)-1
  while(l<=h)
  {
    var mid = (l+h)/2
    if(arr[mid]==x)
      return mid
    else if(x > arr[mid])
      l = mid+1
    else
      h = mid-1
  }
  return nil
}
var nums = [10,20,30,40,50]
println(binary_search(nums,100))

#simple plutonium program for binary search
function binarySearch(var nums, var target){
    var l = 0
    var h = len(nums)-1
    var mid = 0
    while(l<=h)
    {
        mid=(l+h)/2
        if(nums[mid]==target)
          return mid
        else if(target>nums[mid])
          l=mid+1
        else
          h=mid-1
    }
    return -1      #returns -1 if the element is not found
}

var nums = [1, 3, 5, 7, 2, 90, 10]
var key = 90
println(binarySearch(nums, key))

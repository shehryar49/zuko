#simple plutonium program for binary search
function binarySearch(var nums, var target){
    for(var i = 0 to len(nums) step 1)
    {
        if(nums[i] == target)
        return i
    }
    return -1      #returns -1 if the element is not found
}

var nums = [1, 3, 5, 7, 2, 90, 10]
var key = 90
println(binarySearch(nums, key))
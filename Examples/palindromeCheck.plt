#Plutonium program to check palindrome list
function checkPalindrome(var nums){
    var start = 0
    var end = len(nums)-1
    while(start<=end){
        if(nums[start]!=nums[end])
            return false
        start+=1
        end-=1
    }
    return true
}
var list1 = [1, 2, 5, 7, 5, 2, 1]
println(checkPalindrome(list1))

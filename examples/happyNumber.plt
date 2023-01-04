#this program checks if a number is happy or not

function numSquareSum(var n)
{
    var squareSum = 0
    while (n!= 0) {
        squareSum += (n % 10) * (n % 10)
        n /= 10
    }
    return squareSum
}
function checkHappyNumber(var n)
{
    var fast = n 
    var slow = n
    dowhile (slow != fast) {
        # move slow number by one iteration
        slow = numSquareSum(slow)
        # move fast number by two iteration
        fast = numSquareSum(numSquareSum(fast))
    } 
    #if both number meet at 1, then return true
    return (slow == 1)
}
var x = 13
if(checkHappyNumber(x)) 
    println("this is a happy number")
else 
    println("this ain't a happy number")
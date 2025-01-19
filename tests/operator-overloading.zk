class Integer
{
    private var val = nil
    function __construct__(var val)
    {
        self.val = val
    }
    function __smallerthan__(var rhs)
    {
       return self.val < rhs.val
    }
    function __greaterthan__(var rhs)
    {
       return self.val > rhs.val
    }
    function __smallerthaneq__(var rhs)
    {
        return self.val <= rhs.val
    }
    function __greaterthaneq__(var rhs)
    {
       return self.val >= rhs.val
    }
    function __neg__()
    {
       return -self.val
    }
    function __lshift__(var n)
    {
        return self.val << n
    }
    function __rshift__(var n)
    {
        return self.val >> n
    }
    function __complement__()
    {
        return ~self.val
    }
    function __bitwiseand__(var rhs)
    {
        return self.val & rhs.val
    }
    function __bitwiseor__(var rhs)
    {
        return self.val | rhs.val
    }
    function __xor__(var n)
    {
        return self.val ^ n.val
    }
    function __add__(var n)
    {
        return self.val + n.val
    }
    function __sub__(var n)
    {
        return self.val - n.val
    }
    function __mul__(var n)
    {
        return self.val * n.val
    }
    function __div__(var n)
    {
        return self.val / n.val
    }
    function __eq__(var rhs)
    {
        return self.val == rhs.val
    }
    function __noteq__(var rhs)
    {
        return self.val != rhs.val
    }
    function __not__()
    {
        return self.val == 0
    }
    function __index__(var idx)
    {
        return "Bitch please!"
    }
    function __del__()
    {
        println("Object being garbage collected")
    }
    function getval()
    {
        return self.val
    }
}
var x = Integer(10)
var y = Integer(20)
println(x < y)
println(x > y)
println(x <= y)
println(y >= y)
println(-y)
println(!y)
println(x ^ y)
println(x & y)
println(x | y)
println(~y)
println(x[0])
println(x+y)
println(y-x)
println(y/x)
println(x*y)
println(x<<1)
println(y>>1)

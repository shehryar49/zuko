function fib(var n)
{
    var a = 0
    var b = 1
    var c = 0
    for(var i=1 to n-1 step 1)
    {
        yield a
        c = b
        b = a+b
        a = c
    }
    return a
}
function series()
{
    var x = 1
    while(true)
    {
      var c = yield x
      x = c+3
    }
}
var coro = fib(5)
while(coro.isAlive())
{
    println(coro.resume())
}
println()
var siri = series()
var x = siri.resume()
for(var i=1 to 5 step 1)
{
    println(x)
    x = siri.resume(x)
}
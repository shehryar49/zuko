class Window
{
    var width = nil
    var height = nil
    private var x = 30
    function __construct__(var a,var b)
    {
      width = a
      height = b
    }
    function display()
    {
      println(width,"x",height)
    }
    function testIncrment()
    {
      width+=1
      height+=1
      width+=5
      height+=5
    }
    function double()
    {
      width = width*2
      height = height*2
    }
    function g()
    {
      println(x*x)
    }
    function f()
    {
      g()
    }
}
var win = Window(2,5)
win.display()
win.double()
win.display()
win.testIncrment()
win.display()
win.f()
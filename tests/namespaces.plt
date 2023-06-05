var x = 30
namespace std
{
    var x = 40
    function f1()
    {
        println(x) #40
        println(std::x) # 40
    }
}

std::f1()
println(x)
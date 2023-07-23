for(var i=1 to 10000)
{
    println(i)
    if(i == 1)
    {
      i+=1
      continue # skips the increment
    }
}
println()
for(var i=1 to 5)
{
    if(i == 4)
      break
    println(i)
}
var global = 0
for(global = 1 to 3)
{
}
println(global)
if(true)
{
    var local = 0
    for(local=1 to 4)
    {

    }
    println(local)
}
for(var i=1 to 10 step 2)
{
  println(i)
}
for(var i=10 dto 1)
{
  println(i)
}
for(var i=10 dto 1 step 2)
{
  println(i)
}
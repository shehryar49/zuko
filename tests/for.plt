for(var i=1 to 100000 step 1)
{
    println(i)
    if(i == 1)
    {
      i+=1
      continue # skips the increment
    }
}

for(var i=1 to 5 step 1)
{
    if(i == 4)
      break
    println(i)
}
var global = 0
for(global = 1 to 3 step 1)
{
}
println(global)
if(true)
{
    var local = 0
    for(local=1 to 4 step 1)
    {

    }
    println(local)
}
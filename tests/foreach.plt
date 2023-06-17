var nums = [10,30,40]

foreach(var num: nums)
{
    println(num)
}
foreach(var num: nums)
{
    println(num)
    continue # continues to next iteration and loads next num
}
foreach(var num: nums)
{
    println(num)
    if(num == 30)
      break
}
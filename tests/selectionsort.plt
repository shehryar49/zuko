function selectionSort(var arr)
{
  var i = 0
  while(i<len(arr))
  {
    var idx = i
    var j = i+1
    while(j<len(arr))
    {
      if(arr[j] < arr[idx])
        idx = j
      j+=1
    }
    var c = arr[i]
    arr[i] = arr[idx]
    arr[idx] = c
    i+=1
  }
}
var arr = [10,-40,0,4,4,7, 8, 9, 1, 5]
println(arr)
selectionSort(arr)
println(arr)

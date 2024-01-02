
function insertionSort(var arr)
{
  var i = 1
  while(i<len(arr))
  {
    var key = arr[i]
    var j = i-1
    while(j>=0)
    {
      if(key < arr[j])
      {
        var c = arr[j]
        arr[j] = arr[j+1]
        arr[j+1] = c
        j-=1
      }
      else
        break
    }
    arr[j+1] = key
    i+=1
  }
}

var arr = [10,-40,0,4,4,7, 8, 9, 1, 5]
println(arr)
insertionSort(arr)
println(arr)

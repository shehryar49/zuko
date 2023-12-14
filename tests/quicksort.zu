

function partition(var arr,var low,var high)
{
	var i = (low-1)
	var pivot = arr[high]
  var j = low
  var c = 0
	while(j<high)
  {
		if(arr[j] <= pivot)
    {
			i = i+1
      c = arr[i]
      arr[i] = arr[j]
      arr[j] = c
    }
    j+=1
  }
  c = arr[i+1]
  arr[i+1] = arr[high]
  arr[high] = c
	return i+1
}

function quickSort(var arr,var low,var high)
{
	if(low < high)
  {
		var pi = partition(arr, low, high)
		quickSort(arr, low, pi-1)
		quickSort(arr, pi+1, high)
    return 0
  }
  return 0
}
var arr = [10,-40,0,4,4,7, 8, 9, 1, 5]
var n = len(arr)
println(arr)
quickSort(arr, 0, n-1)
println(arr)

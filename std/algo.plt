# Plutonium Algorithm Standard Library
# 6 Feb 2022 Written by Shahryar Ahmad
# Do not edit unless you really know what you are doing ;)
# The code is completely free to use/modify and comes without any warantee

namespace algo
{
  function bubbleSort(var arr)
  {
    var n = len(arr)
    var i = 1
    while(i<=n-1)
    {
      var j = 0
      while(j<n-i)
      {
        if(arr[j] > arr[j+1])
        {
          var c = arr[j]
          arr[j] = arr[j+1]
          arr[j+1] = c
        }
        j+=1
      }
      i+=1
    }
  }
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
          arr[j+1] = arr[j]
          j-=1
        }
        else
          break
      }
      arr[j+1] = key
      i+=1
    }
  }
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
  function binary_search(var arr,var x)
  {
    var l = 0
    var h = len(arr)-1
    while(l<=h)
    {
      var mid = (l+h)/2
      if(arr[mid]==x)
        return mid
      else if(x > arr[mid])
        l = mid+1
      else
        h = mid-1
    }
    return nil
  }
  function linear_search(var arr,var x)
  {
    for(var i=0 to len(arr)-1 step 1)
    {
      if(arr[i]==x)
        return i
    }
    return nil
  }
  function __partition(var l,var h,var arr)
  {
    var pivot = arr[h]
    var i = l-1
    for(var j=l to h-1 step 1)
    {
      if(arr[j] <= pivot)
      {
        i+=1
        var c = arr[i]
        arr[i] = arr[j]
        arr[j] = c
      }
    }
    arr[h] = arr[i+1]
    arr[i+1] = pivot
    return i+1
  }
  function quickSort(var arr,var l,var h)
  {
    if(l<h)
    {
      var p = __partition(l,h,arr)
      quickSort(arr,l,p-1)
      quickSort(arr,p+1,h)
    }
  }
  function __merge(var arr,var l,var mid,var h)
  {
    var temp = []
    var i = l
    var j = mid+1
    while(i<=mid and j<=h)
    {
        if(arr[i]<= arr[j])
        {
          temp.push(arr[i])
          i+=1
        }
        else
        {
          temp.push(arr[j])
          j+=1
        }
    }
    while(i<=mid)
    {
        temp.push(arr[i])
        i+=1
    }
    while(j<=h)
    {
        temp.push(arr[j])
        j+=1
    }
    for(var i=0 to len(temp)-1 step 1)
    {
        arr[l+i] = temp[i]
    }
  } 
  function mergeSort(var arr,var l,var h)
  {
    if(l<h)
    {
        var mid = (l+h)/2
        mergeSort(arr,l,mid)
        mergeSort(arr,mid+1,h)
        __merge(arr,l,mid,h)
    }
  }
  function sublist(var l,var h,var list)
  {
    var res = []
    for(var i=l to h step 1)
    {
      res.push(list[i])
    }
    return res
  }
  function filter(var fn,var list)
  {
    var res = []
    foreach(var elem: list)
    {
      if(fn(elem))
        res.push(elem)
    }
    return res
  }
  function apply(var fn,var list) #applies function fn on all elements of list
  {
        var i = 0
        while(i<len(list))
        {
            list[i] = fn(list[i])
            i+=1
        }
  } 
  function Max(var list)
  {
    var l = len(list)
    if(l==0)
      return nil
    var max = list[0]
    for(var i=1 to l-1 step 1)
    {
      if(list[i]>max)
        max = list[i]
    }
    return max
  }
  function Min(var list)
  {
    var l = len(list)
    if(l==0)
      return nil
    var min = list[0]
    for(var i=1 to l-1 step 1)
    {
      if(list[i]<min)
        min = list[i]
    }
    return min
  }
  function count(var fn,var list)
  {
    #Counts elements in the list fulfilling the given criteria
    var count = 0
    foreach(var elem: list)
    {
      if(fn(elem))
        count+=1
    }
    return count
  }
  function first_mismatch(var list1,var list2)
  {
    var i = 0
    var j = 0
    var l1 = len(list1)
    var l2 = len(list2)
    while(i<l1 and j<l2)
    {
      if(list1[i]!=list2[j])
        return i
      i+=1
      j+=1
    }
    #no mismatch
    return nil 
  }
  function transpose(var matrix)
  {
    var rows = len(matrix)
    if(rows==0)
      return nil
    var cols = len(matrix[0])
    var res = []
    for(var i=0 to cols-1 step 1)
    {
      var r = []
      for(var j=0 to rows-1 step 1)
      {
         r.push(matrix[j][i])
      }
      res.push(r)
    }
    return res
  }
  function submatrices(var matrix,var m,var n)
  {
    var r = len(matrix)
    if(r==0)
      return nil
    var c = len(matrix[0])
    var a = r-m
    var b = c-n
    var all = []
    for(var i=0 to a step 1)
    {
        for(var j=0 to b step 1)
        {
            # (i,j) gives leftmost corner of each submatrix
            var sub = []
            for(var x=i to i+m-1 step 1)
            {
                var row = []
                for(var y =j to j+n-1 step 1)
                  row.push(matrix[x][y])
                sub.push(row)
            }
            all.push(sub)
        }
    }
    return all
  }
  function count_set_bits(var x)
  {
    var f = 1
    var count = 0
    while(f<=x)
    {
      if(x&f==f)
        count+=1
      f = f<<1
    }
    return count
  }
  function isPowerOfTwo(var x)
  {
    if(x==1)
      return true
    #Use Bitwise Magic
    return x & (x-1) == 0
  }
  function median(var list)
  {
    var dummy = clone(list)
    bubbleSort(dummy)
    if(len(dummy)%2==0)
      return (dummy[len(dummy)/2 - 1] + dummy[len(dummy)/2 ]) / 2.0
    else
      return dummy[len(dummy)/2]
  }
  function hex(var n)
  {
    if(n==0)
      return "0"
    var ans = ""
    var r = 0
    while(n!=0)
    {
      r = n%16
      if(r>=10)
        ans+=char(r+87)
      else
        ans+=str(r)
      n/=16
    }
    ans = reverse(ans)
    return ans
  }
  function bin(var n)
  {
    if(n==0)
      return "0"
    var ans = ""
    var r = 0
    while(n!=0)
    {
      r = n%2
      ans+=char(48+r)
      n/=2
    }
    ans = reverse(ans)
    return ans
  }
  function octal(var n)
  {
    if(n==0)
      return "0"
    var ans = ""
    var r = 0
    while(n!=0)
    {
      r = n%8
      ans+=char(48+r)
      n/=8
    }
    ans = reverse(ans)
    return ans
  }
  function decodeASCII(var bytes)
  {
    var str = ""
    foreach(var byte: bytes)
      str+=char(ByteToInt(byte))
    return str
  }
  function encodeASCII(var str)
  {
    var bytes = []
    foreach(var ch: str)
      bytes.push(ascii(ch))
    return bytes
  }
}

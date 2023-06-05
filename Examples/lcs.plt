function lcs(var A,var B,var m,var n)
{
    if(m == 0 or n==0)
      return 0
    if(A[m-1] == B[n-1])
      return 1+lcs(A,B,m-1,n-1)
    else
    {
        var i = lcs(A,B,m-1,n)
        var j = lcs(A,B,m,n-1)
        if(i > j)
          return i
        return j
    }
}

# Bottom up approach (Dynamic programming)
function lcsDP(var A,var B)
{
    var m = len(A)
    var n = len(B)
    var tmp = [0]*(m+1)
    for(var i=0 to m step 1)
      tmp[i] = [0] * (n+1)
    for(var i=1 to m step 1)
    {
        for(var j=1 to n step 1)
        {
           if(A[i-1] == B[j-1])
             tmp[i][j] = 1 + tmp[i-1][j-1]
           else
           {
            var p = tmp[i-1][j]
            var q = tmp[i][j-1]
            if(p > q)
              tmp[i][j] = p
            else 
               tmp[i][j] = q
          }
        }
    }
    return tmp[m][n]
}
println(lcs("AGGTAB","GXTXAYB",6,7))
println(lcsDP("AGGTAB","GXTXAYB"))
println(lcs("ACADB","CBDA",5,4))
println(lcsDP("ACADB","CBDA"))


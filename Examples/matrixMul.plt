# Matrix Chain Multiplication cost minimization problem
# implemented in Plutonium


# Simple recursive solution
function minCost(var input,var l,var h)
{
    if(l+1 == h) # only one matrix
      return 0
 
    var min = @INT_MAX
    for(var i=l+1 to h-1 step 1)
    {
        var chunk1 = minCost(input,l,i)
        var chunk2 = minCost(input,i,h)
        var cost = chunk1+chunk2+(input[l]*input[i]*input[h])
        if(cost < min)
          min = cost
    }
    return min

}
# Solution using Bottom up approach (Dynamic programming)
function minCostDP(var input)
{
    var tmp = [0]*len(input)
    for(var i=1 to len(input) step 1)
      tmp[i-1] = [0]*len(input)
    var l = len(input)
    for(var probSize=2 to l-1 step 1)
    {
      var min = @INT_MAX
      for(var i=0 to l-probSize-1 step 1)
      {
        var l = i
        var h = i+probSize
        var min = @INT_MAX
        for(var k=l+1 to h-1 step 1)
        {
            var chunk1 = tmp[l][k]
            var chunk2 = tmp[k][h]
            var cost = chunk1+chunk2+(input[l]*input[k]*input[h])
            if(cost < min)
              min = cost
        }
        tmp[l][h] = min
      }
    }
    return tmp[0][l-1]
}
println(minCost([1,2,3,4,3],0,4))
println(minCostDP([1,2,3,4,3]))
println(minCost([1,2,3,4],0,3))
println(minCostDP([1,2,3,4]))
println(minCost([10,20,30],0,2))
println(minCostDP([10,20,30]))

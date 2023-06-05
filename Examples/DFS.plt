#!/usr/bin/plutonium
function DFS(var src,var G) #this is a comment
{
    var visited = []
    for(var i=1 to len(G) step 1)
      visited.push(false)
    var stack = [src]
    while(len(stack)!=0)
    {
        var e = stack.pop()
        if(!visited[e])
          print(e," ")
        visited[e] = true
        foreach(var n: G[e])
        {
            if(!visited[n])
              stack.push(n)
        }
    }
    println()
}
var visited = []
function DFSRecursive(var src,var G)
{
  visited[src] = true
  print(src," ")
  foreach(var n: G[src])
  {
    if(!visited[n])
      DFSRecursive(n,G)
  }
}
var topG = [[1,5],[0,3,4,5],[3,5],[1,2],[1],[0,1,2]]
for(var i=1 to len(topG) step 1)
  visited.push(false)
#the above loop does same thing as the following
#visited = [false,false,false,false,false,false]
DFSRecursive(1,topG)

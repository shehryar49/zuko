function BFS(var src,var G)
{
    var visited = []
    for(var i=1 to len(G) step 1)
      visited.push(false)
    var queue = [src]
    visited[src] = true
    while(len(queue)!=0)
    {
        var e = queue.pop()
        print(e," ")
        foreach(var n: G[e])
        {
            if(!visited[n])
            {
              queue.insert(0,n)
              visited[n] = true
            }
        }
    }
    println()
}
var topG = [[1,5],[0,3,4,5],[3,5],[1,2],[1],[0,1,2]]
BFS(1,topG)
#program that finds the smallest element in each row and coloumn of a 3*3 Matrix and store in 2 differnet 1D arrays.

#for rows
var matrix1=[[1,4,7],[2,5,8],[3,6,9]]

var rows=len(matrix1)

var minrows = []

for (var i=0 to rows-1 step 1)
{
    var min = 0 
    for(var j=0 to 2 step 1)
    {
        if(matrix1[i][j]<min or j==0)
          min = matrix1[i][j]
    }
    minrows.push(min)
}

#printing 1D array of minimum elements from matrix rows

println("1D array of minimum elements from matrix rows : \n",minrows)

#for coloumns
var matrix2=[[1,4,7],[2,5,8],[3,6,9]]
var row=len(matrix2)
var mincols=[]
for (var i=0 to row-1 step 1)
{
    var min = 0 
    for(var j=0 to 2 step 1)
    {
        if(matrix2[j][i]<min or j==0)
          min = matrix2[j][i]
    }
    mincols.push(min)
}

#printing 1D array of minimum elements from matrix coloumns
println("1D array of minimum elements from matrix coloumns : \n",mincols)

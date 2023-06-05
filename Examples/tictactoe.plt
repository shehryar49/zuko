# Simple Tic Tac Toe Game in Plutonium

var moves = 0
var a = " "
var b = " "
var c = " "
var d = " "
var e = " "
var f = " "
var g = " "
var h = " "
var i = " "
var sts = ""
var cursor = "O"
function checkWin()
{
  if((a=="O" and b=="O" and c=="") or (d=="O" and e=="O" and f=="O") or (g=="O" and h=="O" and i=="O") or (a=="O" and d=="O" and g=="O") or (b=="O" and e=="O" and h=="O") or (c=="O" and f=="O" and i=="O") or (a=="O" and e=="O" and i=="O") or (c=="O" and e=="O" and g=="O") )
  {
    sts = "Player 1 Wins!"
    return nil
  }
  if((a=="X" and b=="X" and c=="") or (d=="X" and e=="X" and f=="X") or (g=="X" and h=="X" and i=="X") or (a=="X" and d=="X" and g=="X") or (b=="X" and e=="X" and h=="X") or (c=="X" and f=="X" and i=="X") or (a=="X" and e=="X" and i=="X") or (c=="X" and e=="X" and g=="X") )
  {
    sts = "Player 2 Wins!"
    return nil
  }
  if(moves==9)
    sts = "Draw"
}
function changeCursor()
{
  if(cursor=="O")
    cursor = "X"
  else
    cursor = "O"
}

var opt = 0
while(moves!=9)
{
  println(" ",a," | ",b," | ",c,"  ")
  println("___|___|___")
  println(" ",d," | ",e," | ",f,"  ")
  println("___|___|___")
  println(" ",g," | ",h," | ",i,"  ")
  println("   |   |   ")
  print("Enter place to tick(1 to 9): ")
  opt = int(input())
  if(opt==1 and a==" ")
  {
    a = cursor
    moves+=1
    changeCursor()
  }
  else if(opt==2 and b==" ")
  {
    b = cursor
    moves+=1
    changeCursor()
  }
  else if(opt==3 and c==" ")
  {
    c = cursor
    moves+=1
    changeCursor()
  }
  else if(opt==4 and d==" ")
  {
    d = cursor
    moves+=1
    changeCursor()
  }
  else if(opt==5 and e==" ")
  {
    e = cursor
    moves+=1
    changeCursor()
  }
  else if(opt==6 and f==" ")
  {
    f = cursor
    moves+=1
    changeCursor()
  }
  else if(opt==7 and g==" ")
  {
    g = cursor
    moves+=1
    changeCursor()
  }
  else if(opt==8 and h==" ")
  {
    h = cursor
    moves+=1
    changeCursor()
  }
  else if(opt==9 and i==" ")
  {
    i = cursor
    moves+=1
    changeCursor()
  }
  system("clear") #use system("cls") if you are using stupid windows
  checkWin()
  if(sts!="")
    break
}
println(" ",a," | ",b," | ",c,"  ")
println("___|___|___")
println(" ",d," | ",e," | ",f,"  ")
println("___|___|___")
println(" ",g," | ",h," | ",i,"  ")
println("   |   |   ")
println(sts)

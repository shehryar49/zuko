var k = 1
var max = 10
var nstars = 1
while(k<=max)
{
  var j = 1
  while(j<=max-k)
  {
    print(" ")
    j+=1
  }
  j = 1
  while(j<=nstars)
  {
    if(j==1 or j==nstars or k==max)
      print("*")
    else
      print(" ")
    j+=1
  }
  println("")
  nstars+=2
  k+=1
}

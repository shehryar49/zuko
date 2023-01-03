#A program that converts kilometers per hour to miles per hour.

var kph=0
var mps=0
println("Enter the speed in kph:")
kph = tonumeric(input())
mps=(kph*1000)/3600.0
println("The speed in mps=", mps)

#there is no need for explicit type conversion
#just add .0 (point zero) in the denominator to get a floating point value
#see type conversion rules for more detail

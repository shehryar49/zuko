import libcurl as lc
import std/algo.plt
var curl = lc.Curl()
curl.setopt(lc.OPT_URL,"https://www.google.com")
curl.setopt(lc.OPT_FOLLOWLOCATION,1)
curl.setopt(lc.OPT_WRITEFUNCTION,lc.WriteMemory) #use default WriteMemory function
#it comes with the module
var res = curl.perform()
if(res != lc.CURLE_OK) #error occurred
{
    println(lc.strerror(res))
    exit()
}

#request was successful
#curl.data stores response data as bytearray
#decode them as ascii bytes, only do this when response is text
#even if the text is utf8 it will be atleast printed correctly
var str = algo::decodeASCII(curl.data)
println(str)

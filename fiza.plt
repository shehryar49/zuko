#!/usr/bin/plutonium
#-
 Plutonium Package Manager
 Written by Shahryar Ahmad
 10 Feb 2022
 This code is available under MIT license
-#


import libcurl


var openedFile = nil
var size = 0
var clearCommand = nil
function WriteMemory(var bytes)
{
  system(clearCommand)
  size+=len(bytes)
  println("Downloaded ",size, " bytes ")
  fwrite(bytes,openedFile)
}
function downloadFile(var url,var outputPath)
{
  openedFile = open(outputPath,"wb")
  var req = libcurl.Curl()
  req.setopt(libcurl.OPT_URL,url)
  req.setopt(libcurl.OPT_FOLLOWLOCATION,1)
  req.setopt(libcurl.OPT_WRITEFUNCTION,WriteMemory)
  var res = req.perform()
  if(res != libcurl.CURLE_OK)
    throw Error(libcurl.strerror(res))
  close(openedFile)
}
function installPackage(var name,var url,var ver)
{

  if(@os == "Windows 32 bit" or @os == "Windows 64 bit")
  {
    # download dll from repo
    if(@os == "Windows 64 bit")
      url = url + "/releases/download/"+ver+"/"+name+"-win64.dll"
    else
      url = url + "/releases/download/"+ver+"/"+name+"-win32.dll"
    downloadFile(url,"C:\\plutonium\\modules\\"+name+".dll")
  }
  else # unix like systems (install from source)
  {
    url = url + "/archive/refs/tags/"+ver+".tar.gz"
    downloadFile(url,"fizatmp.tar.gz")
    var ret = system("mkdir fizatmp && tar -xf fizatmp.tar.gz -C fizatmp --strip-components 1")
    if(ret != 0)
      throw Error("Failed to extract archive!")
    ret = system("cd fizatmp && cmake . -DCMAKE_BUILD_TYPE=Release && make && cd ..")
    if(ret != 0)
      throw Error("Build failed.")
    ret = system("sudo cp fizatmp/"+name+".so /opt/plutonium/modules")
    if(ret != 0)
      throw Error("[-] Failed to copy files")
    ret = system("rm -rf fizatmp && rm fizatmp.tar.gz")
  }
}
function removePackage(var name)
{
  if(@os == "Windows 32 bit" or @os == "Windows 64 bit")
    system("del /F /Q C:\\plutonium\\modules\\"+name+".dll")
  else
    system("sudo rm /opt/plutonium/modules/"+name+".so")
}
function listPackages()
{
  if(@os == "Windows 32 bit" or @os == "Windows 64 bit")
    system("dir C:\\plutonium\\modules")
  else 
    system("ls -1 /opt/plutonium/modules")
}
function updatePackageList()
{
  var c = libcurl.Curl()
  c.setopt(libcurl.OPT_URL,"https://raw.githubusercontent.com/shehryar49/fiza/main/packages/packages.txt")
  c.setopt(libcurl.OPT_FOLLOWLOCATION,1)
  c.setopt(libcurl.OPT_WRITEFUNCTION,libcurl.WriteMemory)
  var res = c.perform()
  if(res != libcurl.CURLE_OK)
  {
    println("Error updading package list.")
    println(libcurl.strerror(res))
    return nil
  }
  var file = nil
  if(@os == "Windows 64 bit" or @os == "Windows 32 bit")
    file = open("C:\\plutonium\\fiza\\packages.txt","wb")
  else
    file = open("/opt/plutonium/fiza/packages.txt","wb")
  fwrite(c.data,file)
  close(file)
  println("Updated package list successfully!")
}

#########

if(len(argv) == 1 and argv[0] == "update")
  updatePackageList()
else if(len(argv)==1 and argv[0]=="list")
  listPackages()
else if(len(argv) == 2)
{
  var action = argv[0]
  var name = argv[1]

  if(action == "install")
  {
    var srcfile = nil
    if(@os == "Windows 64 bit" or @os == "Windows 32 bit")
    {
      srcfile = open("C:\\plutonium\\fiza\\packages.txt","r")
      clearCommand = "cls"
    }
    else
    {
      srcfile = open("/opt/plutonium/fiza/packages.txt","r")
      clearCommand = "clear"
    }
    var lines = readlines(srcfile)
    var url = nil
    var ver = nil
    foreach(var line: lines)
    {
      var parts = split(line," ")
      if(parts[0] == name)
      {
        url = parts[1]
        ver = parts[2]
      }
    }
    if(ver[len(ver)-1] == "\r") # remove carriage returns
    {
      ver = ver.substr(0,len(ver)-2)
    }
    if(url == nil)
    {
      println("Package ",name," was not found!\nYou can update package list and retry.")
      exit()
    }
    try
    {
      installPackage(name,url,ver)
    }
    catch(err)
    {
      println("[-] ",err.msg)
      println("[-] Installation failed.")
      exit()
    }
    println("[+] Installation done!")
  }
  else if(action == "remove")
    removePackage(name)
  else
    println("Invalid action specified.See DOCS")
}
else
{
  println("usage: plutonium fiza <action> [package]")
  println("Examples: ")
  println("\tinstall packageName")
  println("\tremove packageName")
  println("\tupdate")
  println("\tlist")
}

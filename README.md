# Official Repository of Zuko Programming Langauge

Zuko is a dynamically typed and lightweight programming language written in C++. Zuko is compiled to bytecode and then interpreted by a virtual machine. 

# HOW TO COMPILE:
## Visual Studio:
    
- Install CMake
- Run ```cmake . -DCMAKE_BUILD_TYPE=Release```
- It will generate zuko.sln file which you can open and build


IMPORTANT: You MUST build in Release mode and using Multithreaded runtime library if you want your build to be compatible with
binaries distributed under release tags. Because these are the options that were used to build those binaries.
To install your build create C:\zuko and copy zuko.exe to it. **Note that binaries for Windows are already available. See releases. So you don't really have to build zuko from source on Windows unless you want to contribute.**
    
## Linux/Unix:
Just run the shell script install.sh it will build,test and install zuko. It uses CMake for the build process. Also make sure you have libreadline-dev installed on your system. This is used by the Zuko REPL on Unix like systems. The installation path is /opt/zuko. The install.sh script also creates a symbolic link of /opt/zuko/zuko in /usr/local/bin 
   

## Testing your build

You can test your build using test_all tool (src in tests/). You will have to build it from source. The CMake script does that for you. On Linux/Unix, install.sh script runs the tests automatically using this tool. To use it manually or on Windows just run it from command line and pass the zuko interpreter's path.

# Documentation 
The website: https://zuko-lang.github.io

# Discord
Discord Server: https://discord.gg/cD4cDAszQh

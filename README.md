# Official Repository of Plutonium Programming Langauge

Plutonium is a dynamically typed and lightweight programming language written in C++. Plutonium is compiled to bytecode and then interpreted by a virtual machine. 

# HOW TO COMPILE:
## Visual Studio:
    
- Install CMake
- Run ```cmake . -DCMAKE_BUILD_TYPE=Release```
- It will generate plutonium.sln file which you can open and build


IMPORTANT: You MUST build in Release mode and using Multithreaded runtime library if you want your build to be compatible with
binaries distributed on plutonium's website. Because these are the options that were used to build those binaries.
To install your build create C:\plutonium and copy plutonium.exe to it. **Note that binaries for Windows are already available on the [website](https://plutonium-lang.000webhostapp.com). So you don't really have to build plutonium from source on Windows unless you want to contribute.**
    
## Linux/Unix:
Just run the shell script install.sh it will build,test and install plutonium. It uses CMake for the build process. Also make sure you have libreadline-dev installed on your system. This is used by the Plutonium REPL on Unix like systems. The installation path is /opt/plutonium. The install.sh script also creates a symbolic link of /opt/plutonium/plutonium in /usr/bin . NOTE: Mac OS users will have to configure PATH manually because the symbolic link creation in /usr/bin is not allowed there.
   

## Testing your build

You can test your build using test_all tool (src in tests/). You will have to build it from source. The CMake script does that for you. On Linux/Unix, install.sh script runs the tests automatically using this tool. To use it manually or on Windows just run it from command line and pass the plutonium interpreter's path.

The website: https://plutonium-lang.000webhostapp.com

Discord Server: https://discord.gg/6CnnV4Es

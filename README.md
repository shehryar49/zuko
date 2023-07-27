# Official Repository of Plutonium Programming Langauge

Plutonium is a dynamically typed and lightweight programming language written in C++. Plutonium is compiled to bytecode and then interpreted by a virtual machine. This project has absolutely no external dependencies(except for some modules like MySQL and libCurl).

# HOW TO COMPILE:
## Visual Studio:
    
- Create an empty project
- Add the plutonium.cpp and include folder as sources
- Change the runtime library to Multithreaded and make sure to build in Release mode
- Compile and enjoy

IMPORTANT: You MUST build in Release mode and using Multithreaded runtime library if you want your build to be compatible with
binaries distributed on plutonium's website. Because these are the options that were used to build those binaries.
Additional modules can be built the same way(just build them as dlls and with same options Release+Multithreaded runtime library).To install your build create C:\plutonium and copy plutonium.exe to it. **Note that binaries for Windows are already available on the [website](https://plutonium-lang.000webhostapp.com). So you don't really have to build plutonium from source on Windows unless you want to contribute.**
    
## Linux:
Just run the shell script install.sh it will build,test and install plutonium. 
   
## MinGW or TDM-GCC:
Use the following command:
   
```g++ plutonium.cpp -o plutonium -DNDEBUG -O3 -m64```

## CMake

  The CMake script is in progress and will replace  all above methods once done.Just run CMake from the root directory and set BUILD_TYPE=Release . This will generate plutonium executable in the root directory and shared libraries in **modules** directory
  
## Testing your build

You can test your build using test_all tool (src in tests/) .You will have to build it from source. To use it to test your build just run it from command line and pass the plutonium interpreter's path.This is done by default when building on Linux using install.sh .On Windows you can also use this tool.

The website: https://plutonium-lang.000webhostapp.com

Discord Server: https://discord.gg/6CnnV4Es

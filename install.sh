#!/usr/bin/sh
mkdir /opt/plutonium
mkdir /opt/plutonium/modules
g++ plutonium.cpp -o plutonium -ldl -DNDEBUG -O3 -w
strip plutonium
cp plutonium /opt/plutonium/
cp -r std /opt/plutonium/
sudo ln -s /opt/plutonium/plutonium /usr/bin/plutonium 

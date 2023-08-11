#!/usr/bin/sh
echo "[+] Creating /opt/plutonium"
mkdir /opt/plutonium
echo "[+] Creating /opt/plutonium/modules"
mkdir /opt/plutonium/modules
echo "[+] Starting compilation"
g++ plutonium.cpp -o plutonium -ldl -DNDEBUG -O3 -w
strip plutonium
cp include/PltObject.h /opt/plutonium/
cp -r std /opt/plutonium
g++ -shared modules/math/math.cpp -o modules/math.so -I include/ -fPIC -DNDEBUG -O3
g++ -shared modules/regex/regex.cpp -o modules/regex.so -I include/  -fPIC -DNDEBUG -O3
g++ -shared modules/json/json.cpp -o modules/json.so -I include/  -fPIC -DNDEBUG -O3
g++ -shared modules/cgi/cgi.cpp -o modules/cgi.so -I include/  -fPIC -DNDEBUG -O3
g++ -shared modules/datetime/datetime.cpp -o modules/datetime.so -I include/  -fPIC -DNDEBUG -O3
#g++ -shared modules/mysql/mysql.cpp -o modules/mysql.so -I include/  -fPIC $(mariadb_config --include --libs) -DNDEBUG -O3

g++ tests/test_all.cpp -o tests/test_all
echo "[+] Running tests"
cd tests/
if ./test_all; then
  echo "\nAll Tests passed."
else
  echo "\nTests failed.\nAborting Install"
  return
fi
cd ..
echo "[+] Copying files"
cp plutonium /opt/plutonium/
cp -r std /opt/plutonium/
cp modules/*.so /opt/plutonium/modules
sudo ln -s /opt/plutonium/plutonium /usr/bin/plutonium 

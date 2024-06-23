#!/usr/bin/sh
echo "[+] Starting compilation"
if cmake . -DCMAKE_BUILD_TYPE=Release;then
  echo "[+] Cmake configure done."
else
  echo "[+] Cmake configure failed. Aborting install"
  exit
fi
if make; then
  echo "[+] Build complete"
else
  echo "[-] Build failed. Aborting install"
  exit
fi

echo "\n[+] Copying files (might require password)\n"
# sudo section
sudo mkdir /opt/zuko
sudo mkdir /opt/zuko/modules
sudo mkdir /opt/zuko/include
sudo mkdir /opt/zuko/lib
sudo cp zuko /opt/zuko/
sudo cp -r std /opt/zuko
sudo cp -r fiza /opt/zuko
sudo cp fiza.zk /opt/zuko/
sudo cp modules/*.so /opt/zuko/modules/
sudo cp modules/*.dylib /opt/zuko/modules/

# Copy dev kit
sudo cp include/coroutineobj.h /opt/zuko/include
sudo cp include/funobject.h /opt/zuko/include
sudo cp include/klass.h /opt/zuko/include
sudo cp include/klassobject.h /opt/zuko/include
sudo cp include/module.h /opt/zuko/include
sudo cp include/nativefun.h /opt/zuko/include
sudo cp include/strmap.h /opt/zuko/include
sudo cp include/zapi.h /opt/zuko/include
sudo cp include/zbytearray.h /opt/zuko/include
sudo cp include/zdict.h /opt/zuko/include
sudo cp include/zfileobj.h /opt/zuko/include
sudo cp include/zlist.h /opt/zuko/include
sudo cp include/zobject.h /opt/zuko/include
sudo cp include/zstr.h /opt/zuko/include
sudo cp include/apiver.h /opt/zuko/include
sudo cp include/apifunctions.h /opt/zuko/include
sudo cp libzapi.a /opt/zuko/lib
#

sudo chmod 777 /opt/zuko/fiza/packages.txt
sudo chmod +x /opt/zuko/fiza.zk
sudo ln -s /opt/zuko/zuko /usr/local/bin/zuko
sudo ln -s /opt/zuko/fiza.zk /usr/local/bin/fiza
echo "[+] Running tests"
cd tests/
if ./test_all; then
  echo "[+] Installation done and all tests passed!"
else
  echo "[-] Installation was done but some tests failed."
  
fi
# Hasta la vista baby

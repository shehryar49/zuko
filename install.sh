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

echo "[+] Running tests"
if tests/test_all; then
  echo "[+] All tests passed!"
else
  echo "[-] Tests failed. Aborting install"
  
fi

# Finally copy the files
echo "[+] Copying files (might require password)"
# sudo section
sudo mkdir /opt/zuko
sudo mkdir /opt/zuko/modules
sudo mkdir /opt/zuko/include
sudo mkdir /opt/zuko/lib
sudo cp zuko /opt/zuko/
sudo cp -r std /opt/zuko
sudo cp -r fiza /opt/zuko
sudo cp modules/*.so /opt/zuko/modules/ 2> /dev/null
sudo cp modules/*.dylib /opt/zuko/modules/ 2> /dev/null

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

# Configure permissions
sudo chmod 777 /opt/zuko/fiza/packages.json
sudo chmod 777 /opt/zuko/modules
sudo chmod +x /opt/zuko/fiza/fiza.zk
sudo ln -s /opt/zuko/zuko /usr/local/bin/zuko
sudo ln -s /opt/zuko/fiza/fiza.zk /usr/local/bin/fiza

# Hasta la vista baby
echo "[+] Installation done"

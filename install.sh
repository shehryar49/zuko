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

echo "[+] Copying files"
# sudo section
sudo mkdir /opt/zuko
sudo mkdir /opt/zuko/modules
sudo cp zuko /opt/zuko/
sudo cp -r std /opt/zuko
sudo cp -r fiza /opt/zuko
sudo cp fiza.zk /opt/zuko/
sudo cp modules/*.so /opt/zuko/modules/
sudo cp include/ZObject.h /opt/zuko
#sudo cp include/c_api.h /opt/zuko
#sudo cp include/c_api.cpp /opt/zuko
sudo chmod 777 /opt/zuko/fiza/packages.txt
sudo chmod +x /opt/zuko/fiza
sudo ln -s /opt/zuko/zuko /usr/bin/zuko
sudo ln -s /opt/zuko/fiza.zk /usr/bin/fiza
echo "[+] Running tests"
cd tests/
if ./test_all; then
  echo "\n[+] Installation done and all tests passed!"
else
  echo "\n[-] Installation was done but some tests failed."
  
fi
# Hasta la vista baby

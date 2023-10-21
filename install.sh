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
sudo mkdir /opt/plutonium
sudo mkdir /opt/plutonium/modules
sudo cp plutonium /opt/plutonium/
sudo cp -r std /opt/plutonium
sudo cp -r fiza /opt/plutonium
sudo cp fiza.plt /opt/plutonium/fiza
sudo cp modules/*.so /opt/plutonium/modules/
sudo cp include/PltObject.h /opt/plutonium
sudo cp include/c_api.h /opt/plutonium
sudo cp include/c_api.cpp /opt/plutonium
sudo chmod 777 /opt/plutonium/fiza/packages.txt
sudo chmod +x /opt/plutonium/fiza
sudo ln -s /opt/plutonium/plutonium /usr/bin/plutonium
sudo ln -s /opt/plutonium/fiza /usr/bin/fiza
echo "[+] Running tests"
cd tests/
if ./test_all; then
  echo "\n[+] Installation done and all tests passed!"
else
  echo "\n[-] Installation was done but some tests failed."
  
fi
# Hasta la vista baby

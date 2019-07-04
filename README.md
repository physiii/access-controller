# NFC Lock
ESP32 lock controller with NFC and motion detection.


## Install [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html)

```
sudo apt install -y \
  libncurses5 gcc git wget make libncurses-dev flex bison gperf python python-pip genromfs \
  python-setuptools python-serial python-cryptography python-future python-pyparsing 
echo 'export PATH="$PATH:/usr/local/src/esp/xtensa-esp32-elf/bin"' >> ~/.bashrc
echo 'export IDF_PATH="/usr/local/src/esp/esp-idf"' >> ~/.bashrc
source ~/.bashrc

sudo chmod 777 /usr/local/src
cd /usr/local/src
```
*download esp.zip or:*
```
mkdir esp
cd esp
git clone --recursive https://github.com/espressif/esp-idf.git
wget https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz
tar -zxvf xtensa-esp32* 

python2.7 -m pip install --user -r $IDF_PATH/requirements.txt
```


## Install [lws-factory](https://github.com/warmcat/lws-esp32-factory)
```
git clone https://github.com/warmcat/lws-esp32-factory
cd lws-esp32-factory
export ESPPORT=/dev/ttyUSB0
```
Put device in program mode.  
```
make erase_flash
```
Put device in program mode.  
```
make all flash monitor
```
Press *reboot* button  
Should see lws-factory load in terminal  
Connect ESP device on wifi  
Go to https://192.168.4.1  
Set wifi name/password  
Wait for IP address  

## Get NFC Lock code
```
git clone https://github.com/physiii/nfc-lock
cd nfc-lock/code
make -j16 all
```
Goto https://192.168.4.1 - or you can go to new IP address  
Upload binary from code/build folder  
After binary uploads, you can use `make all lws_flash_ota monitor` in the future

#### Program Mode
Program mode makes device ready to be flashed.  
Hold down *prog* button and press/release *reboot* button.  

#### Factory Mode
Factory mode boots from lws-factory to upload new binaries or set AP info.  
Hold down *fact* button then press/release *reboot* button.  

# flir_gige
A ros driver for FLIR ax5 thermal camera using Pleora eBus SDK 4.0.5

## Installing Pleora eBUS SDK
You can find the latest version of Pleora eBus SDK from [here](http://www.pleora.com/support-center/documentation-downloads).  
If you couldn't download one from their website, this driver comes with version 4.0.5.  
To install, run the following command:

```bash
cd install
sudo ./eBUS_SDK_4.0.5.3150_Ubuntu-12.04-x86_64.run
# accept all default options
```

This will install the eBUS SDK to `/opt/pleora`.   
If you are using ubuntu 14.04, you need to install `libudev-dev` and link it into `/usr/lib`, since eBUS SDK links to that version by default.

```bash
sudo apt-get install libudev-dev
cd /usr/lib
sudo ln -s x86_64/libudev.so libudev.so.0
```

## Running the node
Running the node is easy. Just do

```
roslaunch flir_gige node.launch <ip_address:=xxx.xxx.xxx.xxx> <fps:=20> <bit:=2> <color:=false>
```

Notice that the `bit` option specifies whether to receive 8 bit or 14 bit data.  
`bit - 2`: This will tell the device to return data in 8 bit format, which is good for visualization  
`bit - 3`: This will tell the device to return data in 14 bit format, which can be used to get the temperature

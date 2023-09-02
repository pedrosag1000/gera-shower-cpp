# Gera display


## Change NVIDIA logo

first of all you should flash the jetson with a custom logo to remove the nvidia logo from boot loader of the board

### Jetson board's image

you should have 1920x1080 image with less than around 80kb in filesize.

after that you should convert image to bmp.blob (which is a format that jetson support for his splash screen on his hardware)
. clone this repository and convert your image to bmp.blob [Link](https://github.com/VladimirSazonov/jw-boot-logo-for-Jetson-Nano)

or you can follow this:
1) sudo apt install liblz4-tool
2) git clone https://github.com/VladimirSazonov/jw-boot-logo-for-Jetson-Nano
3) cd jw-boot-logo-for-Jetson-Nano
4) git clone https://github.com/nothings/stb
5) g++ -o jw_boot_image main.cpp
6) ./jw_boot_image [path_to_your_logo]
7) Make sure there is an output file named "bmp.blob"
8) Copy "bmp.blob" into your "[path_to_your_bsp]/Linux_for_Tegra/bootloader/"


You should download board image. for this, download the Nvidia SDK manager and install it on ****ubuntu 18.0.4**** (important)
and just download the os flash things. after download files you should see downloaded files in `/home/{user}/Download/nvidia`

After extract the image file, you should replace the bmp.blob file to `bootloader` folder.

Start your board in Force recovery mode (connect pin 3 to pin 4).

Run this command on **ubuntu 18.0.4** (i think you can flash board in other ubuntu versions but if you want to be sure, run this in 18.0.4)

```shell
sudo ./flash.sh jetson-nano-qspi mmcblk0p
```

[useful link for flashing the board](https://developer.technexion.com/docs/flash-step)

when you fully flashed your board, first image you was sawing is changed now!

### Jetson OS images (Splash, Logo, Background)

when you flash the board, os will not install, and you should install os manually.

you can downloadOS files and follow steps of install OS in [this link](https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit#write)

you should replace /usr/share/backgrounds/NVIDIA_* files with your images (check `images` folder)

## install OpenCV with OpenGL and GStreamer (Not Tested yet)

check this link for more information

https://yunusmuhammad007.medium.com/build-and-install-opencv-4-5-3-on-jetson-nano-with-cuda-opencl-opengl-and-gstreamer-enable-6dc7141be272

## build application

```shell
sudo apt install cmake make gcc

mkdir ~/Projects
cd ~/Projects
git clone https://github.com/gbmhunter/CppLinuxSerial
cd CppLinuxSerial
mkdir build
cd build
cmake ..
make
sudo make install

cd Projects
git clone https://github.com/pedrosag1000/gera-shower-cpp
cd gera-shower-cpp
mkdir build
cd build
cmake ..
make 
```

Run the `./DisplayImage` with needed arguments


## Make /dev/ttyTHS1 available in non-root mode (without sudo)
 
````shell
systemctl stop nvgetty
systemctl disable nvgetty
udevadm trigger
sudo usermod -a -G dialout $USER
````

create ` /etc/udev/rules.d/55-tegraserial.rules` and fill the file with this:
```shell
KERNEL=="ttyTHS*", MODE="0666"
```

## Run application on startup

Open Startup Applications via the Activities overview. Alternatively you can press `Alt + F2` and run the `gnome-session-properties` command.
 
Fill the command field with your application command.



"rtspsrc location=rtsp://admin:Admin1401@192.168.1.64:554/streaming/channels/103 latency=10 drop-on-latency=1 ! rtph264depay ! h264parse ! decodebin ! autovideoconvert ! video/x-raw,format=BGRx ! videoconvert ! video/x-raw,format=BGR ! appsink drop=true sync=false" /dev/ttyTHS1 1080 "appsrc ! video/x-raw, format=BGR ! queue ! videoconvert ! video/x-raw,format=NV12 ! autovideoconvert ! x265enc ! h265parse ! qtmux ! filesink location=video1.mp4 sync=false" 
"rtspsrc location=rtsp://admin:Admin1401@192.168.1.64:554/streaming/channels/103 latency=10 drop-on-latency=1 ! rtph264depay ! h264parse ! avdec_h264 ! appsink drop=true sync=false" /dev/ttyTHS1 1080 "appsrc ! video/x-raw, format=BGR ! queue ! videoconvert ! video/x-raw,format=NV12 ! autovideoconvert ! x265enc ! h265parse ! qtmux ! filesink location=video1.mp4 sync=false" 
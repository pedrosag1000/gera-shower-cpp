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


You should download Jetpack 4.6.4 and rootfs filesystem. for doing this, Download the Nvidia SDK manager and install it on ****ubuntu 18.0.4**** (important)
and just download the Jetson and Rootfs files. after download files you should see downloaded files in `/home/{user}/Download/nvidia`


Extract the Jetpack and add Rootfs files to `Linux_for_Tegra/rootfs`

For changing boot logo you should replace the `bmp.blob` file in `bootloader` folder.

Start your board in Force recovery mode (connect pin 3 to pin 4).

Run this command on **ubuntu 18.0.4** (i think you can flash board in other ubuntu versions but if you want to be sure, run this in 18.0.4)


**NOTE: Nvidia manager has issue in flashing jetson nano boards !!! I couldn't flash the board by that. use this command instead (manually)**
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


#### for multi stream in gstreamer check this [Link](https://gstreamer.freedesktop.org/documentation/coreelements/tee.html?gi-language=c)


## Make /dev/ttyTHS1 available in non-root mode (without sudo)
 
````shell
systemctl stop nvgetty
systemctl disable nvgetty
udevadm trigger
sudo usermod -a -G dialout $USER
````
Reset bash

nano ` /etc/udev/rules.d/55-tegraserial.rules` and fill the file with this:

```shell
KERNEL=="ttyTHS*", MODE="0666"
```

## Run application on startup

Open Startup Applications via the Activities overview. Alternatively you can press `Alt + F2` and run the `gnome-session-properties` command.
 
Fill the command field with your application command. (Running the application from the outside of his folder will raise an exception on loading resources ... please run the application from his folder)

Note:

I'd tried another solution for run an application on the os startup but I failed because, the application could not read display size. 
In this method you just add your command in ~/.xsessionrc (more info [Here](https://unix.stackexchange.com/questions/281858/difference-between-xinitrc-xsession-and-xsessionrc))
# Gera display


## change NVIDIA logo

first of all you should flash the jetson with a custom logo to remove the nvidia logo from boot loader of the board

You hava 4 places that you should have image for them.
1) Jetson board's image

you should have 1920x1080 image with less than around 80kb in filesize.

after that you should convert image to bmp.blob (which is a format that jetson support for his splash screen on his hardware)
. clone this repository and convert your image to bmp.blob [Link](https://github.com/VladimirSazonov/jw-boot-logo-for-Jetson-Nano)

You should download board image. for this, download the Nvidia SDK manager and install it on ****ubuntu 18.0.4**** (important)
and just download the os flash things. after download files you should see downloaded files in `/home/{user}/Download/nvidia`

After extract the image file, you should replace the bmp.blob file to `bootloader` folder.

Start your board in Force recovery mode (connect pin 3 to pin 4).

Run this command on **ubuntu 18.0.4** (i think you can flash board in other ubuntu versions but if you want to be sure, run this in 18.0.4)

```sudo ./flash.sh jetson-nano-qspi mmcblk0p```

[useful link for flashing the board](https://developer.technexion.com/docs/flash-step)

when you fully flashed your board, first image you was sawing is changed now!

2) OS images (Splash, Logo, Background)

when you flash the board, os will not install, and you should install os manually.

you can downloadOS files and follow steps of install OS in [this link](https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit#write)

you should replace /usr/share/backgrounds/NVIDIA_* files with your images (check `images` folder)



## build application


```
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


Run the `./DisplayImage` with you inputs


# Pedalboard software

To compile you need Raspberry Pi Pico SDK:

cd pedalboard
mkdir build
cd build
#export PICO_SDK_PATH=/usr/share/pico-sdk
cmake -DPICO_BOARD=pico_w ..

This will generate the build/pico-example-midi.uf2 file that you need to upload
to Raspberry Pi Pico.

To upload the program, you need to reset the Pico by disconnecting it, pressing
 the BOOTSEL button and, while button is pressed, reconnecting the device to
the computer using USB cable.

Once done, the Raspberry Pi will appear as a usb stick to which you just upload
the program pico-example-midi.uf2

This repo contains a binary in the root directory, so if you don't have to build
it unless you make changes to the source code in pico-example-midi.c


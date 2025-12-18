# PicoWiimote

This is a repo containing the code necessary for connecting a Wii Controller (Wiimote) to a Raspberry Pi Pico W. This is untested on a Pico 2 W, but I imagine that it should work out of the box, or at worst with minor configurations. 

It uses the [BTStack library](https://github.com/bluekitchen/btstack) and also enables the IR Camera; any IR source should work (including a burning candle!). Please refer to the [Wiibrew wiki page](https://wiibrew.org/wiki/Wiimote) on the Wiimote for more information on the raw data output from the Wiimote. Out of the box, this code configures the Wiimote to send data formatted in the 0x33 reporting mode. 

## Disclaimer
This code was originally made for a final, open-ended design project for [ECE 4760 - Digital Systems Design Using Microcontrollers](https://ece4760.github.io/) at Cornell University. As such, parts of it might not be as clean or efficient as possible (we were under a time crunch and prioritized delivering a working project above all else); use at your own risk, and feel free to modify the files to make them more efficient.

## How to use

1. In src/wiimote_bt/wiimote_bt.c, change remote_addr_string to the Bluetooth address of your Wiimote. I got mine by first connecting the Wiimote to a PC through Dolphin.
2. Upload the code to the Pico, then leave it in an unpowered state.
3. Put the Wiimote into pairing mode by holding down the 1+2 buttons (the LEDs should all be flashing).
4. Power on the Pico. If all goes well, the Wiimote should only have the Player 1 LED illuminated, indicating that the connection was established. Steps 2-4 will need to be performed whenever the Pico or Wiimote shut off.

## Code Functionality

### main branch
The main branch contains just the Bluetooth connection code, as well as a header file which contains global variables the Wiimote updates constantly.

### final-project-ece4760
The final-submission branch contains code very close to our final project submission. The professor of the course ([V. Hunter Adams](https://vanhunteradams.com/)) provided [VGA driver code](https://vanhunteradams.com/Pico/VGA/VGA.html) for the Pico; with this, we could drive a VGA display and construct basic apps and games which take advantage of the Wiimote. Please see our final project webpage for more information. 

One thing I'll note here is that the driver code does make some approximations for code simplicity (which technically violate the VGA standard); some monitors can't properly render the Pico's VGA output, but some can. Also, a few apps are modified versions of lab code, and so are redacted here for academic integrity reasons. 

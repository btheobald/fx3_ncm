This project is an initial attempt at getting a NCM driver functional for the Cypress FX3.

It gets as far as reporting the correct descriptor on the USB bus, but never shows up as a network interface.

Debugging the FX3 is a bit of a pain, so progress has stalled for now.

The limited ammount of RAM and ROM on the FX3 itself means its probably isn't going to have the space to host a full TCP/IP stack. (Even LwIP needs too much memory.)
This means that this would probably need to be hosted on another device, which limits the usefulness of this device driver somewhat given the cost of the FX3.

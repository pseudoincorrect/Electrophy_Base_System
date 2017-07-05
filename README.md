# Neural activity transmission device 

This repository describe the code of a neural activity transmission device embedded on a rodent.
The purpose of this system is to log the the electrical activity of a rat's neurones onto a computer.

## Global view of the acquisition system 

![alt text][global_system]

[global_system]: https://raw.githubusercontent.com/pseudoincorrect/Electrophy_Base_System/master/pictures/global_system.png "Global data acquisition system"


### THis repository contain the code files necessary program the *interfacing board* between the embedded system on the rodent and a PC/Oscilloscope (see above)


### Features (Embedded Part)

*. Omnetics adapter
*. Transmit 4 channels of 16 bits at 20 kHz 
*. Or 8 channels compressed (16 bits after decompression) at 20 kHz
*. Consume 18 mA at 3.7 V at full transmission load
*. Weight 7 grams 
*. Volume : 2cm *2cm * 2cm 


### Features (Interface Part)

*. USB audio protocol interface to display and log on PC
*. 8 channels DAC to connect to an acquisition card
*. USB as power supply


## Technical part

On this version, we use the an ST Microelectronic Development board discovery F4 with a Microcontroler [STM32f411VET6](http://www.st.com/web/en/catalog/tools/PF260946) to interface the Nordic Semiconductor [Nrf24l01+](http://www.nordicsemi.com/eng/Products/2.4GHz-RF/nRF24L01) RF transceiver, the Digital to analog converter [DAC8568ICPW](http://fr.rs-online.com/web/p/cna-a-usage-general/7093131/) and the USB audio protocol.

### Short description of the working process of the Interface system


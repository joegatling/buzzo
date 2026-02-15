# Making a Buzzo

Here's a basic guide for building a Buzzo. 

## Step 1 - Gather the parts
A single Buzzo requires...

- 1x Assembled Buzzo LED ring PCB (See step 2)
- 3D printed parts (See Step 3)
- 1x Adafruit ESP32-S3 Feather dev board (https://www.adafruit.com/product/5477)
- 1x 9mm Piezeo Buzzer (https://www.amazon.com/dp/B0827DS4HH)
- 1x 500mAh Lithium Ion Battery (https://www.adafruit.com/product/1578)
- 1x 10mm (tall) x 12mm x 12mm tactile switch (https://www.lcsc.com/product-detail/C127505.html)
- 7x M2x10mm screws
- Wire

## Step 2 - Ordering the PCBs
The main component of the Buzzo Button is a custom LED ring PCB. The gerber files can be found [here](/hw/). My initial run of PCBs were ordered from JLCPCB.com.
![The LED ring PCB](/img/Buzzo_LEDRingPCB.png).

# Step 3 - 3D Print the Parts

All the STEP files for 3D printing can be found [here](/3d/Button/). Most of the parts can be printed out of any kind of PLA but the part `Button_ButtonCap_TPU.step` needs to be printed in white TPU as it needs to be flexible and diffuse light.

# Step 4 - Assemble the wiring
The wiring is quite simple. Connect everything up as follows:

- Feather `3V` ↔ `3.3v` on Ring PCB
- Feather `GND` ↔ `GND` on Ring PCB
- Feather `Pin 5` ↔ `LED` on Ring PCB
- Feather `Pin 6` ↔ `BTN` on Ring PCB
- Feather `Pin 9` ↔ Piezo Pin A (This component has no polarity)
- Feather `GND` ↔ Piezo Pin B (Or `GND` on the Ring PCB)

![Example completed Buzzo wiring](/img/Buzzo_Wiring.png)

# Step 5 - Upload the firmware
The firmware is written in C++, and is crea

1. Clone this repo to your computer
2. Install Visual Studio Code
3. Inside Visual Studio Code, install the PlatformIO Add-on
4. Open the project in Visual Studio Code, and set the environment to **buzzo_button_adafruit_USBc**.
5. Plug in your assembled Buzzo Button
6. Click **->** (PlatformIO: Upload)

~[Visual studio code environment](/img/VSCode_Environment.png)
~[Visual studio code environment](/img/VSCode_Build.png)

   

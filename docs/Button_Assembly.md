# Making a Buzzo

Here's a basic guide for building a Buzzo. 

## Making the Buzzo Button

## Step 1 - Gather the parts
A single Buzzo Button requires...

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

## Step 3 - 3D Print the Parts

All the STEP files for 3D printing can be found [here](/3d/Button/). Most of the parts can be printed out of any kind of PLA but the part `Button_ButtonCap_TPU.step` needs to be printed in white TPU as it needs to be flexible and diffuse light.

## Step 4 - Assemble the wiring
The wiring is quite simple. Connect everything up as follows:

- Feather `3V` ↔ `3.3v` on Ring PCB
- Feather `GND` ↔ `GND` on Ring PCB
- Feather `Pin 5` ↔ `LED` on Ring PCB
- Feather `Pin 6` ↔ `BTN` on Ring PCB
- Feather `Pin 9` ↔ Piezo Pin A (This component has no polarity)
- Feather `GND` ↔ Piezo Pin B (Or `GND` on the Ring PCB)

![Example completed Buzzo wiring](/img/Buzzo_Wiring.png)

All the wiring can be done before assembling the 3D printed parts. Below is what a partially assembled device looks like. 

Some tips:
- Feed the assembled wiring in through the top by sliding the Feather through the hole.
- Note that the LED PCB sits on top of the printed collar piece.
- A small blob of hot glue can be used to hold the piezo speaker in place.

![Buzzo button top](/img/Buzzo_AssembledTop.png)
![Buzzo button top](/img/Buzzo_AssembledBottom.png)

## Step 5 - Upload the firmware
The firmware is written in C++, and is crea

1. Clone this repo to your computer
2. Install Visual Studio Code
3. Inside Visual Studio Code, install the PlatformIO Add-on
4. Open the project in Visual Studio Code, and set the environment to **buzzo_button_adafruit_USBc**.
5. Plug in your assembled Buzzo Button
6. Click **->** (PlatformIO: Upload)

![Visual studio code environment](/img/VSCode_Environment.png)
![Visual studio code environment](/img/VSCode_Build.png)

# Making the Buzzo Controller

## Step 1 - Gather the parts
A single Buzzo Controller requires...

- Some kind of button PCB (See Step 2)
- 3D printed parts (See Step 3)
- 1x Adafruit ESP32-S3 Feather dev board (https://www.adafruit.com/product/5477)
- 1x 500mAh Lithium Ion Battery (https://www.adafruit.com/product/1578)
- 4x M2x10mm screws
- 1x 3mm LED
- 1x Resistor, (value depending on the LED forward voltage)
- 4x Tactile Switches (Specifics unknown, see step 2)
- Wire

## Step 2 - PCB Sidequest
This is where the instructions get murky. Since the Buzzo Controller is still an early prototype, there is actually no PCB designed for it. All existing version were built using a scavanged PCB. Below is a reference of what the PCB looked like. Designing something that fits that shape would allow the existing 3D printable models to be used.

![Buzzo Controller PCB](/img/Controller_ScrapPCB.png)

You can see the scrap PCB in the assembled photo below.

## Step 3 - The Wiring

- Feather `Pin 5` ↔ Reset Button (Other pin on switch to GND)
- Feather `Pin 6` ↔ Incorrect Button (Other pin on switch to GND)
- Feather `Pin 9` ↔ Correct Button (Other pin on switch to GND)
- Feather `Pin 10` ↔ Pause Button (Other pin on switch to GND)
- Feather `Pin 11` ↔ Resistor ↔ LED ↔ GND

![Assembled Buzzo Controller](/img/Controller_Assembled.png)

## Step 4 - Upload the firmware
The firmware is written in C++, and is crea

1. Clone this repo to your computer
2. Install Visual Studio Code
3. Inside Visual Studio Code, install the PlatformIO Add-on
4. Open the project in Visual Studio Code, and set the environment to **buzzo_controller_adafruit_USBc**.
5. Plug in your assembled Buzzo Controller
6. Click **->** (PlatformIO: Upload)

![Visual studio code environment](/img/VSCode_Build.png)






# StoryButtons
StoryButtons is an interactive audio installation featuring wireless LED-ringed buttons that play recorded stories from children about their inventions. Designed for libraries and educational spaces, each button triggers a unique story, creating a playful and engaging way to explore young creativity. This repository contains the code, hardware setup, and deployment instructions for building your own StoryButtons system.

*⚡A  [comon](http://www.comon.gent) spark⚡*    
*Tinkered with* ❤ *by Bas Baccarne*   
*Supported with*🔥 *by Jeroen Bougonjon, Line Winey, Margot Bulcke, Fran Burger & Stephanie Van Hove*

## Concept
**Independent push buttons**
  * One button per invention (as many as you want), each powered by 5V.
  * When pushed, they send their ID to the hub that answers with the length of the associated audio file (over ESP-NOW).
  * As long as pushed a led ring counts down depending on the audio duration.
  * When released, the button sends a signal to the hub to stop the audio.

**Listen hub**
  * Single hub (or multiple if you want them on multiple places), powered by 5V.
  * Listens to incoming IDs and plays the associated audio file (and send back duration to the sending button).
  * These audiofiles are prerecorded and stored on an sd card (DFplayer mini).
  * The audio can be listened to through headphones.
  * The listen hub has an enticing and stimulating design.

## Parts list
| Part     | Link    | Cost   | Amount  |
| -------- | ------- |------- | ------- |
| **Independent Push Buttons** (N=4)|
| XIAO ESP32S3  | [Seeed](https://wiki.seeedstudio.com/XIAO_ESP32S3_Getting_Started/)   | €10 | 4 |
| Stainless steel button | [Conrad](https://www.conrad.be/nl/p/tru-components-tc-9563704-druktoets-12-v-0-005-a-1x-uit-aan-contact-element-o-x-h-24-7-mm-x-14-5-mm-ip65-1-stuk-s-bulk-2390926.html?searchType=SearchRedirect)          | €7 | 4 |
| LED ring | [Adafruit](https://www.adafruit.com/product/1463) | €16 | 4 |
| Diode | [Digikey](https://www.digikey.be/en/products/detail/stmicroelectronics/1N5819/1037326)| €0.3 | 4 |
| DCBarrel jack connector | [Digikey](https://www.digikey.be/en/products/detail/schurter-inc/4840-2201/2644239) | €3| 4 |
| Acryl sheet | experimentation| | |
| 5V power adapter | | | | 
| Custom PCB (N=5) | [JLCPCB](https://jlcpcb.com/) | €30 |  1 |
| M3 x 4 x 5 treaded inserts | | | |
| Stand-offs | | | |
| M3 screws | | | |
| **Listen hub** |
| XIAO ESP32S3  | [Seeed](https://wiki.seeedstudio.com/XIAO_ESP32S3_Getting_Started/)   | €10 | 1 |
| DFPlayer Mini    | [DFRobot](https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299)      | €6 | 1 |
| Female 3.5mm jack | [Allekabels](https://www.onlinekabelshop.nl/35mm-jack-v-stereo-open-eind-audiokabel-zwart-030.html) | €2 | 1 |
| Headphone | [JBL Wired](https://www.bol.com/be/nl/p/jbl-tune-500-on-ear-koptelefoon-zwart/9200000098045179/) | €26 | 1 |
| **Total** |  | **€76** |

## Construction
<div align="left">  
<img src="img/schematic.png" width="600">
</div>

## Tests
**Input** (touch - Button) 
* Push: [Pull-up button](test/push.ino)
* Touch: [Continuous reading of capacitive touch pads](tests/capacitive.ino)   
(curious towards the 9 capacitive touchpads on the XIAO ESP32S3)   

**Output** (audio - DFplayer)   
* [Play sound file](tests/audio.ino)   

**Output** (led ring - Neopixel)
* [LED ring simple chase animation](tests/ledring.ino)   
* [LED ring countdown](tests\ledring_countdown.ino)

**Interations**   
* [Hold to listen](tests/holdtolisten.ino)
* [Push & start led countdown](tests/push_and_count.ino)

**Communication** (ESP-NOW)   
* [Step 1 - Get MAC adress of the devices](tests/mac.ino)
* [Step 2 - send data & listen to response (device MAC D8:3B:DA:73:C6:74)](tests/device1.ino)
* [step 3 - listen to response and answer when message received (device MAC D8:3B:DA:73:C4:58)](tests/device2.ino)


## Actions
- [ ] Assemble button
- [ ] Soft fade led ring
- [ ] Mounting system button
- [ ] Technical test listen hub
- [ ] Concept listen hub
- [ ] Buy final components

## Background
### Create a custom PCB
* For this case, I'm creating a custom PCB to illustrate the flow on a simple example
* If you're new to this, it might be easer to work in [easyEDA](easyeda.com), then import the STEP file in you CAD program
* [Good tutorial](https://www.youtube.com/watch?v=MsdJgEinb34&t=23s)
* If tou want to step up your game: work with the intergrated PCB modules in CAD (eg. in Gerber is intergrated in Fusion)

**workflow**
1. **test**: Test circuit and code on a breadboard
2. **parts**: Research the components you need (tip: note down part numbers)
3. **place**: Open an easyEDA project, search and place your components (cfr part numbers) using the library module
4. **connect***: Wire your components together (circruit design)
5. **position**: Position components on the two sides of the breadboard
6. **refine**: Optimize wires & add text
7. **Export**: Export step file for import in CAD (crosscheck & go back if corrections are needed)
8. **Order**: EasyEDA allows direct orders on PCBWay

**parts**   
When adding parts   
* Part are built out of 3 components: (1) schematic (2) footprint (3) 3D model
* Check if there is a version in the library (crosscheck foorptint)
* If not: create your own schematic & footprint (start from [datasheet](https://www.schurter.com/en/datasheet/typ_4840.2201.pdf) and [available models](https://www.digikey.be/nl/models/2644239))
* In some software you can import this directly (for easyEDA, this is a payed feature)
* Create you own component (or crosscheck user contributed part):
  * Create new schematic
    * Schematic representation
    * Functional: add pins (figure out: name versus number)
    * Add metadata (data sheet, url, manufacturer, ...)
    * Use correct prefix (e.g. for a jack connection, this is J?)  
  * Create new footprint
    * Start with reference point and use x-y coordinates
    * Add pads (number them correctly)
    * Use silk layer for things you want to print on the PCB
    * Go back to schematic and add footprint
  * Add 3D Model
    * Look for wrl file online
    * Open Tools 3D model manager
    * Go to footprint and add 3D model
* Typical width of the ring: 0.15 mm to 0.2 mm

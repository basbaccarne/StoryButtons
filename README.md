# Listenmodule
small XIAO set-up that supports "touch to listen" interactions

*by: Bas Baccarne*

**components**
| Part     | Link    | Cost   |
| -------- | ------- |------- |
| XIAO ESP32S3  | [Seeed](https://wiki.seeedstudio.com/XIAO_ESP32S3_Getting_Started/)   | €7 |
| Stainless steel button | [Conrad](https://www.conrad.be/nl/p/tru-components-tc-9563704-druktoets-12-v-0-005-a-1x-uit-aan-contact-element-o-x-h-24-7-mm-x-14-5-mm-ip65-1-stuk-s-bulk-2390926.html?searchType=SearchRedirect)          | €7 |
| Capacitive Touch Sensor | [Seeed](https://wiki.seeedstudio.com/Grove-Touch_Sensor/) | €4|
| DFPlayer Mini    | [DFRobot](https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299)      | €6 |
| LED ring | [Adafruit](https://www.adafruit.com/product/1463) | €24 |
| Female 3.5mm jack | [Allekabels](https://www.onlinekabelshop.nl/35mm-jack-v-stereo-open-eind-audiokabel-zwart-030.html) | €2 |
| Headphone | [JBL Wired](https://www.bol.com/be/nl/p/jbl-tune-500-on-ear-koptelefoon-zwart/9200000098045179/) | €26 |
| **Total** |  | **€76** |

**schematic**
<div align="left">  
<img src="img/schematic.png" width="600">
</div>


**tests**   
* [Button Push](tests/touch.ino)
* [Play sound file](tests/sound.ino)
* [Hold to listen](tests/holdtolisten.ino)

**Actions**
- [ ] LED Ring for Duration
- [ ] Technical Concept multiple stories & in situ installation (power etc)
- [ ] Housing (& custom PCB?)

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

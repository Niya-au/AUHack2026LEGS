The reposity contains code the drinkable leg project for AUHack2026. This is a 3D moulded leg fitted with internal
tubing that allows for drinking any beverage through it. When a user wants to "drink a leg", they can register their
information via an app and this will create a new entry for them in the database. The leg is fitted with water level and water flow sensors
that allow for the logging of the time taken to finish a leg and the average flow rate to be recorded and registered for the user.

**auHackLegRegistration.html** and **regPageTrims.css** contains code for formatting the mobile application that registers a new user and 
can create a new entry in the database using buttons. **ESP32_final.ino** has the microcontroller code to read data from sensors and send it to the database when the whole leg is finished. 
**ESP32_water_level** contains test code to test elapsed time and average flowrate on the hardware setup. 

Folders **/examples** and **/espSupabaseExample** contain code to test the HTML interface and wifi-based connection to the ESP32 respectively. 

## Setup instuctions
The ESP32 requires Wifi, HTTPClient and ArduinoJSON libraries. The board is to be flashed through ArduinoIDE via USB-Serial.

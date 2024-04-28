# Overview

This is the code for the locking device microcontroller created for my ESET Capstone team's project. The code has a few goals. Firstly, it needs to ensure that it is still on the correct Wi-Fi channel for ESP-NOW communication. It will send "alive" status checking packets to the other ESP32 and wait for a response to signify that the connection is still "alive". Secondly, it  is waits for an "unlock" packet that tells it to activate the Solenoid that it's connected to, unlocking the door for a period of time. Once this time is up, the Solenoid is deactivated, locking the door once again. One final thing to note is that in order to improve the robustness of the code, numerous redundancy checks and logging statements were added.

The code for the microncontroller is thoroughly documented and thus I recommend reading through it to get a better understanding of what all is going on.

## Notes
This project was programmed in VS Code with the ESP-IDF extension. The device utilized was an ESP32-S3-DevKitC-1-N8R2.

# Embedded-System-Project
In this project, I implement an embedded system with these attributes:

+ Server should send with MQTT protocol information like CPU load, CPU temperature, differences in face detected, and change in audio to the user when it request

+ Server should save essential data in a database implemented with MySQL.

+ Server should get an HTTP get request and answer two requests: 1. capture a photo 2. give the last n row of the database to the user

In this project, I used different libraries like:

* Boost/beast
* OpenCV
* Alsa

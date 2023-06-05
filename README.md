# Local Network IP and MAC Scanner
This **Local Network IP and MAC Scanner Project** is scans ip and mac addresses 
in the local network as fast as possible using threads.

## Setup
```cmd
g++ -o LocalScanner.exe main.cpp -lws2_32 -liphlpapi
```

## Usage
LocalScanner.exe -s startIP -e endIP -m <br />
-m => mac address scan <br />
no inputs => default scan 192.168.1.1 - 192.168.1.255 <br />
```
LocalScanner.exe -s 192.168.1.4 -e 192.168.1.25 -m
```
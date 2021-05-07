# Ground station core (GSC)

All-in-one open-source utility for SDR-based satellite tracking

Tested with weather NOAA satellites and the International Space station (ISS).

## Features

1. Compatible with all common SDRs
2. Compatible with all common antenna controllers
3. Completely autonomous: doesn't need internet connection to operate
4. Flexible: uses networking to operate with different parts
5. All Unix-based systems supported
6. JSON-based configuration
7. Doppler compensation

## Installation

Install `libpredict`:
```
git clone https://github.com/la1k/libpredict.git
cd libpredict
mkdir build
cd build
cmake ..
sudo make install
```

Install prerequisites
```
sudo apt-get install cmake gcc make predict libjson-c-dev hamlib-utils
```

Install GSC:
```
git clone https://github.com/skypodolsky/isu_ground_station.git
cd isu_ground_station
mkdir build
cd build
cmake ..
make
```

## Launch

Start `rotctld` antenna controlling daemon(-s):

```
screen sudo rotctld -s 9600 -m 1003 -r /dev/ttyUSB0 -T 127.0.0.1 -t 8080 -vvvvv
```

Download list of active satellites:
```
wget http://celestrak.com/NORAD/elements/active.txt ./build/
```

Start GSC:
```
screen sudo ./gsc --verbosity 3 --remote-addr 127.0.0.1 --azimuth-port 8080 --elevation-port 8081 --request-port 25565 --latitude=48.31237 --longitude=7.44126
```

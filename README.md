# CpuUsageTracker
Script calculate load for every visible core, and prints it in formatted way to the console with 1Hz rate. \
Technologies used:
- cmake
- pthreads
- GTest
- C
- GCC/CLang

## Structure
```
├── CMakeLists.txt
├── README.md
├── src
│   ├── Analyzer
│   │   ├── Analyzer.c
│   │   ├── Analyzer.h
│   │   └── CMakeLists.txt
│   ├── CMakeLists.txt
│   ├── Logger
│   │   ├── CMakeLists.txt
│   │   ├── Logger.c
│   │   └── Logger.h
│   ├── Printer
│   │   ├── CMakeLists.txt
│   │   ├── Printer.c
│   │   └── Printer.h
│   ├── Queue
│   │   ├── CMakeLists.txt
│   │   ├── Queue.c
│   │   └── Queue.h
│   ├── Reader
│   │   ├── CMakeLists.txt
│   │   ├── Reader.c
│   │   └── Reader.h
│   ├── Watchdog
│   │   ├── CMakeLists.txt
│   │   ├── Watchdog.c
│   │   └── Watchdog.h
│   └── main.c
└── ut
    ├── CMakeLists.txt
    └── QueueTest.cpp
```

## Build
Build system is entirely based on CMake, user can choose between using GCC or Clang compiler during build process. To do so user need to declare proper CC flag before cmake execution. \
In order to build project navigate to root project folder and type:

```
cmake -S ./ -B ./build
cd build
make
```

## Run
To run application from root folder:
```
./build/src/CUT
```
Example:
```
CPU core usage: CORE  0:  10%   CORE  1:   5%   CORE  2:   6%   CORE  3:   3%
```

For gracefull application shutdown use Ctrl+C, following prints should be visible:
```
Cleaning resources...
CUT app closed. 
```

## How does it work?
Program is based on 5 threads running simultaniously, which are:
1.  Reader
2.  Analyzer
3.  Printer
4.  Watchdog
5.  Logger

#### Ad.1 Reader
Reader thread responsible is for reading raw data from /proc/stat file, encapsulate it to a structure, and push it to Analyzer thread

#### Ad.2 Analyzer
Analyzer thread responsible is for calculating load per every available core in running system based on data received from Reader thread. Calculated cores load is then send to Printer thread.

#### Ad.3 Printer
Printer thread is responsible for printing formatted core load data to the console.

#### Ad.4 Watchdog
Watchdog checks if every thread is working properly. If it detects that given thread is not responsive program will exit with failure.

#### Ad.5 Logger
Main job for logger thread is to save every received debug print for other threads in to the DebugData.txt file.


Threads are communicating witch each other through dedicated fifo queues.
Application have implementation for signal handling in case of SIGTERM and SIGINT. When signal is received, script should deallocate all dynamic memory that was allocated during runtime, and close every thread.  


## Debug logs
Program save debug messages during runtime that can be later checked by user. Every debug print should have timestamp in the beggining and name of the function that paricular message originates from.

Messages are saved in:
```
build/src/LoggerData.txt
```

Example prints:
```
Thu Jan 12 10:36:22 2023 FUNC:PushRawCpuTimesToLogger Msg: cpu=0 user=2547 nice=0 system=1078 idle=381327 iowait=798 irq=0 softirq=572 steal=0 guest=0
Thu Jan 12 10:36:22 2023 FUNC:PushRawCpuTimesToLogger Msg: cpu=1 user=2053 nice=0 system=750 idle=383176 iowait=142 irq=0 softirq=224 steal=0 guest=0
Thu Jan 12 10:36:22 2023 FUNC:PushRawCpuTimesToLogger Msg: cpu=2 user=1949 nice=0 system=979 idle=382656 iowait=257 irq=0 softirq=165 steal=0 guest=0
Thu Jan 12 10:36:22 2023 FUNC:PushRawCpuTimesToLogger Msg: cpu=3 user=2049 nice=0 system=781 idle=383151 iowait=163 irq=0 softirq=114 steal=0 guest=0
Thu Jan 12 10:36:22 2023 FUNC:PushLoadDataToLogger Msg: Core:  0, Load:   0%
Thu Jan 12 10:36:22 2023 FUNC:PushLoadDataToLogger Msg: Core:  1, Load:   0%
Thu Jan 12 10:36:22 2023 FUNC:PushLoadDataToLogger Msg: Core:  2, Load:   0%
Thu Jan 12 10:36:22 2023 FUNC:PushLoadDataToLogger Msg: Core:  3, Load:   0%
Thu Jan 12 10:36:22 2023 FUNC:CleanUp_Handler Msg: Cleaning resources
Thu Jan 12 10:36:22 2023 FUNC:Analyzer Msg: Analyzer thread closed
Thu Jan 12 10:36:23 2023 FUNC:Watchdog Msg: Watchdog OK
Thu Jan 12 10:36:23 2023 FUNC:Watchdog MSG: Watchod thread closed
Thu Jan 12 10:36:23 2023 FUNC:Printer Msg: Printer thread closed
Thu Jan 12 10:36:23 2023 FUNC:Reader Msg: Reader thread closed
```
## Known limitations
1. Script works only in Linux based systems
2. Max number of cores that program can measure is limitted to 32
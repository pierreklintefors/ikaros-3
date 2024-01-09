# ikaros-3

Ikaros is an open framework for system-level brain modeling and real-time robot control. Ikaros supports the design and implementation of large-scale computation models using a flow programming paradigm. 

More than 100 persons  contributed to the code base of earlier versions and over [100 scientific publications](http://www.ikaros-project.org/publications/) report on work that has used Ikaros for simulations or robot control.

Ikaros 2 is described in the article [Ikaros: A framework for controlling robots with system-level brain models](https://journals.sagepub.com/doi/full/10.1177/1729881420925002).


Version 3 is completely rewritten in modern C++ and includes a number of new features. 
This version is currently in development and not ready for use. 

For now, please use the [previous version](https://github.com/ikaros-project/ikaros) instead.

## Ikaros 3 Status - January, 2024

| Component | State | Comments |
| ----|----|----| 
| CMake             |<div style="color:green">🟢 OK |   |
| Matrices          |<div style="color:green">🟢 OK | not optimized |
| Ranges            |<div style="color:green">🟢 OK | no enumerated ranges |
| Dictionary        |<div style="color:green">🟢 OK |  |
| Options           |<div style="color:green">🟢 OK |
| Maths             |<div style="color:#c60">🟡 Partial | only essential functions |
| Parameters        |<div style="color:green">🟢 OK | some conversions missing |
| Expressions       |<div style="color:green">🟢 OK |  |
| XML               |<div style="color:green">🟢 OK | some std::string functions missing |
| Kernel            |<div style="color:#c60">🟡 Partial |  |
| Shared Dict       |<div style="color:green">🟢  OK |  |
| Scheduler         |<div style="color:#c60">🟡 Minimal | no zero-delays, single thread |
| SetSizes    |     <div style="color:green">🟢 OK |
| Input resizing    |<div style="color:green">🟢 OK |     |  |
| Delays            |<div style="color:#green">🟢 OK | copy not completed |
| Task Sorting      |<div style="color:#e00">🔴 Pending |  |
| Real Time         |<div style="color:green">🟢 OK |  |
| WebUI             |<div style="color:green">🟢 OK |  |
| BrainStudio       |<div style="color:#e00">🔴 Pending | templates & editing |
| Message queue     |<div style="color:#c60">🟡 Partial |  |
| Log               |<div style="color:#c60">🟡 Partial |  |
| Encryption        |<div style="color:#e00">🔴 Pending |  |
| Editing           |<div style="color:#c60">🟡 Minimal |
| Sockets           |<div style="color:green">🟢 OK |  |
| UtilityModules    |<div style="color:#c60">🟡 Partial | 4 modules |
| Named Dimensions  |<div style="color:green">🟢 OK |  |


# Timing

Timing is set by the *real-time* flag together with the desired *tickduation*. Tickduation defaults to 1 s.

All times are set in seconds as a double.

On Mac OS the timing error is below 0.2 microseconds.

The following functions should be used for all timing calculations and work both in real-time and simulated time.

 Function | return type |Real-time | Simulated time  |
| ----|----|----|----|
| GetTick() | tick_count  |tick | tick | 
| GetTickDuration() | double  | tickduration | tickduration 
| GetTime() | double  | GetRealtime() | tick * tickduration 
| GetRealTime() | double  | timer.GetTime() | tick * tickduration 
| GetLag()  | double  | tick * tickduration - timer.GetTime() | 0 

GetLag() resturns the lag at the time of calling and can be used to adjust for jitter within a module. It is not necessarily the same as the *lag* value in the kernel.

# Basic Start-up Parameters

filename :   ikg-file to load

-w: wait for command from WebUI; default if no filename is given

-r: real-time mode

-d# : tick duration (in seconds)

All parameters have can be set in the top element of the ikg-file as well.
# ikaros-3

Ikaros is an open framework for system-level brain modeling and real-time robot control. Ikaros supports the design and implementation of large-scale computation models using a flow programming paradigm. 

More than 100 persons  contributed to the code base of earlier versions and over [100 scientific publications](http://www.ikaros-project.org/publications/) report on work that has used Ikaros for simulations or robot control.

Ikaros 2 is described in the article [Ikaros: A framework for controlling robots with system-level brain models](https://journals.sagepub.com/doi/full/10.1177/1729881420925002).


Version 3 is completely rewritten in modern C++ and includes a number of new features. 
This version is currently in development and not ready for use. 

For now, please use the [previous version](https://github.com/ikaros-project/ikaros) instead.

## Ikaros 3 Status - July, 2023

| Component | State | Comments |
| ----|----|----| 
| CMake             |<div style="color:green">OK |   |
| Matrices          |<div style="color:green">OK | not optimized |
| Ranges            |<div style="color:green">OK | no enumerated ranges |
| Dictionary        |<div style="color:green">OK |  |
| Options           |<div style="color:green">OK |
| Maths             |<div style="color:#c60">Partial | only essential functions |
| Parameters        |<div style="color:green">OK | some conversions missing |
| Expressions       |<div style="color:green">OK |  |
| XML               |<div style="color:green">OK | some std::string functions missing |
| Kernel            |<div style="color:#c60">Partial |  |
| Scheduler         |<div style="color:#c60">Minimal | no zero-delays, single thread |
| SetSizes    |     <div style="color:green">OK | 
| Input resizing    |<div style="color:green">OK     |  |
| Delays            |<div style="color:#c60">Partial | copy not completed |
| Task Sorting      |<div style="color:#e00">Pending |  |
| Real Time         |<div style="color:#e00">Pending |  |
| WebUI             |<div style="color:#e00">Pending |  |
| BrainStudio       |<div style="color:#e00">Pending | templates & editing |
| Message queue     |<div style="color:#e00">Pending |  |
| Log               |<div style="color:#c60">Partial |  |
| Encryption        |<div style="color:#e00">Pending |  |
| Editing           |<div style="color:#e00">Pending |  |
| Sockets           |<div style="color:#c60">Partial | std::string not supported |
| UtilityModules    |<div style="color:#c60">Partial | 4 modules |
| Named Dimensions  |<div style="color:#c60">Partial | only in matrices |


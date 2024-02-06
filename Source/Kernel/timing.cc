// timing.cc		Timing utilities for the IKAROS project
//
//    Copyright (C) 2006-2023  Christian Balkenius
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//    See http://www.ikaros-project.org/ for more information.
//
//	This file implements a timer class using the POSIX timing functions
//  TODO: Use only chrono functions
//  std::this_thread::sleep_for(std::chrono::milliseconds(x));

#include "timing.h"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

using namespace std::chrono;

std::string TimeString(double time)
{
    int days = time / 86400;
    time -= (double(days)*86400.0);

    int hours = time / 3600;
    time -= (double(hours)*3600.0);

    int minutes = time / 60;
    double seconds = time - (double(minutes)*60.0);

    if(days>0) 
        return std::to_string(days) + " " + std::to_string(hours) + ":" + std::to_string(minutes) + ":" + std::to_string(seconds);
    else
        return std::to_string(hours) + ":" + std::to_string(minutes) + ":" + std::to_string(seconds);

}



void
Sleep(double time)
{
    Timer t;
    t.WaitUntil(t.GetTime()+time);
}


long
GetTimeStamp()
{
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}



std::string GetClockTimeString()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}




void
Timer::Restart()
{
    start_time = std::chrono::steady_clock::now();
}


double
Timer::GetTime()
{
    return 0.001 * std::chrono::duration_cast<milliseconds>(steady_clock::now() - start_time).count();

}



double
Timer::WaitUntil(double time)
{
    float dt;
	while ((dt = GetTime()-time) < -0.128){ std::this_thread::sleep_for(std::chrono::microseconds(127000)); };
	while ((dt = GetTime()-time) < -0.004){ std::this_thread::sleep_for(std::chrono::microseconds(3000)); };
	while ((dt = GetTime()-time) < -0.001){ std::this_thread::sleep_for(std::chrono::microseconds(100)); };
    while(GetTime() < time)
    {} // burn some cycles to get this as accurate as possible
    return GetTime() - time;
}



Timer::Timer()
{
    Restart();
}



std::string
Timer::GetTimeString()
{
    return TimeString(GetTime());
}

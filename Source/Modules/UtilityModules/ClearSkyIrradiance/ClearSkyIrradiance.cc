#include "ikaros.h"
#include <math.h>
#include <iostream>
#include <cmath>
#include <ctime>

using namespace ikaros;


// Function to calculate the day of the year
int 
calculateDayOfYear(int year, int month, int day) 
{
    std::tm tm = {};
    tm.tm_year = year - 1900; // tm_year is years since 1900
    tm.tm_mon = month - 1;    // tm_mon is 0-based
    tm.tm_mday = day;

    std::time_t time = std::mktime(&tm);
    std::tm *start_of_year_tm = std::gmtime(&time);
    start_of_year_tm->tm_mon = 0;
    start_of_year_tm->tm_mday = 1;

    std::time_t start_of_year_time = std::mktime(start_of_year_tm);
    return std::difftime(time, start_of_year_time) / (60 * 60 * 24) + 1;
}

// Function to calculate the solar declination
double 
calculateSolarDeclination(int day_of_year) 
{
    return 23.44 * std::sin((360.0 / 365.24) * (day_of_year - 81) * M_PI / 180.0);
}

// Function to calculate the equation of time
double 
calculateEquationOfTime(int day_of_year) 
{
    double B = 2 * M_PI * (day_of_year - 81) / 364;
    return 9.87 * std::sin(2 * B) - 7.53 * std::cos(B) - 1.5 * std::sin(B);
}

// Function to calculate the time correction
double calculateTimeCorrection(double longitude, double equation_of_time) 
{
    return 4 * (longitude - 15 * std::floor(longitude / 15)) + equation_of_time;
}

// Function to calculate the solar hour angle
double 
calculateSolarHourAngle(double hour, double time_correction) 
{
    double solar_time = hour + time_correction / 60.0;
    return (solar_time - 12) * 15;
}

// Function to calculate the solar altitude
double 
calculateSolarAltitude(double latitude, double solar_declination, double solar_hour_angle) 
{
    double latitude_rad = latitude * M_PI / 180.0;
    double solar_declination_rad = solar_declination * M_PI / 180.0;
    double solar_hour_angle_rad = solar_hour_angle * M_PI / 180.0;

    return std::asin(std::sin(latitude_rad) * std::sin(solar_declination_rad) +
                     std::cos(latitude_rad) * std::cos(solar_declination_rad) * std::cos(solar_hour_angle_rad)) * 180.0 / M_PI;
}

// Function to calculate the clear sky irradiance
double 
calculateClearSkyIrradiance(double latitude, double longitude, int year, int month, int day, int hour, int minute) 
{
    int day_of_year = calculateDayOfYear(year, month, day);
    double solar_declination = calculateSolarDeclination(day_of_year);
    double equation_of_time = calculateEquationOfTime(day_of_year);
    double time_correction = calculateTimeCorrection(longitude, equation_of_time);
    double solar_hour_angle = calculateSolarHourAngle(hour + minute / 60.0, time_correction);
    double solar_altitude = calculateSolarAltitude(latitude, solar_declination, solar_hour_angle);
    if (solar_altitude > 0) 
        return 1361 * std::sin(solar_altitude * M_PI / 180.0);  // 1361 W/m^2 is the solar constant
    else 
        return 0;
}

int main() {
    // Location coordinates
    double latitude = 40.7128;  // New York City latitude
    double longitude = -74.0060;  // New York City longitude

    // Date and time
    int year = 2023;
    int month = 7;
    int day = 13;
    int hour = 12;
    int minute = 0;

    // Calculate clear sky irradiance
    double clearSkyIrradiance = calculateClearSkyIrradiance(latitude, longitude, year, month, day, hour, minute);

    // Print result
    std::cout << "Clear Sky Irradiance: " << clearSkyIrradiance << " W/m^2" << std::endl;

    return 0;
}



class ClearSkyIrradiance: public Module
{
    parameter   longitude;
    parameter   latitue;

    parameter   year;
    parameter   month;
    parameter   date;


    matrix      output;

    void Init()
    {
        Bind(osc_type, "type");
        Bind(frequency, "frequency");
        Bind(output, "OUTPUT");
    }


    float func(float time, float freq)
    {
        //std::cout << "   " << time << "  " << << std::endl;
        switch(osc_type.as_int())
        {
            case 0: return sin(2*M_PI*time*freq);
            case 1: return sin(2*M_PI*time*freq) > 0 ? 1 : 0;
            default: return 0;
        }
    }

    void Tick()
    {
        float time = kernel().GetTime();

        output.apply(frequency, [=](float x, float f) {return func(time, f);});
    }
};

INSTALL_CLASS(ClearSkyIrradiance)


/*
	Copyright 2016 Golikov Vitaliy

	This file is part of wlswitch.

	wlswitch is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	wlswitch is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with wlswitch. If not, see <http://www.gnu.org/licenses/>.
*/
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include "marker.h"

Marker::Marker(std::string name, double r, double g, double b)
{
    markerName = name;
    setMarker((std::size_t)r, (std::size_t)g, (std::size_t)b);
}

std::string Marker::getName()
{
    return markerName;
}
std::string Marker::threeIntsToHexString(std::size_t r, std::size_t g, std::size_t b)
{
    //Using for converting int to hex string
    std::stringstream convertStream;
    convertStream << std::setw(2) << std::setfill('0') << std::hex << r;
    convertStream << std::setw(2) << std::setfill('0') << std::hex << g;
    convertStream << std::setw(2) << std::setfill('0') << std::hex << b;
    return ((std::string)"#" + convertStream.str());
}

std::size_t Marker::convertToColorByte(std::size_t a)
{
    if (a > 255)
        return 255;
    else if (a < 0)
        return 0;
    return a;
}

void Marker::setMarker (double r, double g, double b)
{
    std::size_t tempR = (std::size_t)(r * 1.8);
    std::size_t tempG = (std::size_t)(g * 1.8);
    std::size_t tempB = (std::size_t)(b * 1.8);
    tempR = convertToColorByte(tempR);
    tempG = convertToColorByte(tempG);
    tempB = convertToColorByte(tempB);
    markerStringLightLight = threeIntsToHexString(tempR, tempG, tempB);

    tempR = (std::size_t)(r * 1.5);
    tempG = (std::size_t)(g * 1.5);
    tempB = (std::size_t)(b * 1.5);
    tempR = convertToColorByte(tempR);
    tempG = convertToColorByte(tempG);
    tempB = convertToColorByte(tempB);
    markerStringLight = threeIntsToHexString(tempR, tempG, tempB);

    markerString = threeIntsToHexString((std::size_t)r, (std::size_t)g, (std::size_t)b);

    tempR = (std::size_t)(r * 0.7);
    tempG = (std::size_t)(g * 0.7);
    tempB = (std::size_t)(b * 0.7);
    tempR = convertToColorByte(tempR);
    tempG = convertToColorByte(tempG);
    tempB = convertToColorByte(tempB);
    markerStringDark = threeIntsToHexString(tempR, tempG, tempB);

    tempR = (std::size_t)(r * 0.4);
    tempG = (std::size_t)(g * 0.4);
    tempB = (std::size_t)(b * 0.4);
    tempR = convertToColorByte(tempR);
    tempG = convertToColorByte(tempG);
    tempB = convertToColorByte(tempB);
    markerStringDarkDark = threeIntsToHexString(tempR, tempG, tempB);
}

std::string Marker::getSelectedString(std::string src)
{
    std::cerr << "Get: " << src << ". from: " << markerName << std::endl;
    if (src == markerName + (std::string)"DD")
        return markerStringDarkDark;
    if (src == markerName + (std::string)"D")
        return markerStringDark;
    if (src == markerName + (std::string)"LL")
        return markerStringLightLight;
    if (src == markerName + (std::string)"L")
        return markerStringLight;
    if (src == markerName)
        return markerString;
    return (std::string)"";
}
Marker::~Marker()
{

}
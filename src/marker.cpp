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
    markerLightLight = threeIntsToHexString(tempR, tempG, tempB);
    markerInvertLight = threeIntsToHexString(255 - tempR, 255 - tempG, 255 - tempB);

    tempR = (std::size_t)(r * 1.5);
    tempG = (std::size_t)(g * 1.5);
    tempB = (std::size_t)(b * 1.5);
    tempR = convertToColorByte(tempR);
    tempG = convertToColorByte(tempG);
    tempB = convertToColorByte(tempB);
    markerLight = threeIntsToHexString(tempR, tempG, tempB);
    markerInvertLight = threeIntsToHexString(255 - tempR, 255 - tempG, 255 - tempB);

    marker = threeIntsToHexString((std::size_t)r, (std::size_t)g, (std::size_t)b);
    markerInvertLight = threeIntsToHexString(255 - (std::size_t)r, 255 - (std::size_t)g, 255 - (std::size_t)b);

    tempR = (std::size_t)(r * 0.7);
    tempG = (std::size_t)(g * 0.7);
    tempB = (std::size_t)(b * 0.7);
    tempR = convertToColorByte(tempR);
    tempG = convertToColorByte(tempG);
    tempB = convertToColorByte(tempB);
    markerDark = threeIntsToHexString(tempR, tempG, tempB);
    markerInvertDark = threeIntsToHexString(255 - tempR, 255 - tempG, 255 - tempB);

    tempR = (std::size_t)(r * 0.4);
    tempG = (std::size_t)(g * 0.4);
    tempB = (std::size_t)(b * 0.4);
    tempR = convertToColorByte(tempR);
    tempG = convertToColorByte(tempG);
    tempB = convertToColorByte(tempB);
    markerDarkDark = threeIntsToHexString(tempR, tempG, tempB);
    markerInvertDarkDark = threeIntsToHexString(255 - tempR, 255 - tempG, 255 - tempB);
}

std::string Marker::getSelectedString(std::string src)
{
    if (src == markerName + (std::string)"DD")
        return markerDarkDark;
    if (src == markerName + (std::string)"D")
        return markerDark;
    if (src == markerName + (std::string)"LL")
        return markerLightLight;
    if (src == markerName + (std::string)"L")
        return markerLight;
    if (src == markerName)
        return marker;
    if (src == markerName + (std::string)"InvertDD")
        return markerInvertDarkDark;
    if (src == markerName + (std::string)"InvertD")
        return markerInvertDark;
    if (src == markerName + (std::string)"InvertLL")
        return markerInvertLightLight;
    if (src == markerName + (std::string)"InvertL")
        return markerInvertLight;
    if (src == markerName + (std::string)"Invert")
        return markerInvert;
    return (std::string)"";
}
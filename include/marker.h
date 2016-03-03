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
#ifndef WLSWITCH_MARKER_H
#define WLSWITCH_MARKER_H
#include <string>

class Marker
{
private:
    std::string markerStringLightLight;
    std::string markerStringLight;
    std::string markerString;
    std::string markerStringDark;
    std::string markerStringDarkDark;
    std::string markerName;

    std::size_t convertToColorByte(std::size_t a);
    std::string threeIntsToHexString(std::size_t r, std::size_t g, std::size_t b);
public:
    Marker(std::string name, double r = 0.0, double g = 0.0, double b = 0.0);

    void setMarker (double r, double g, double b);
    std::string getName();
    std::string getSelectedString(std::string src);

    ~Marker();
};
#endif //WLSWITCH_MARKER_H

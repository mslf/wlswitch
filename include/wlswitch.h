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
#ifndef WLSWITCH_H_INCLUDED
#define WLSWITCH_H_INCLUDED
#include <string>
#include <unordered_map>
#include <Magick++.h>

class Wlswitch
{
private:
    std::string homePath;
    std::string currentDir;
    std::string switcherProgram;
    std::string switcherArguments;
    std::string delay;
    std::string currentDependConfig;
    std::string currentWallpaper;
    //Script runs after depend configs updating by shellProgram
    std::string updateScript;
    std::string shellProgram;

    std::unordered_map<std::string, Magick::Image::ImageStatistics> statisticsContainer;
    std::string meanRColor, meanGColor, meanBColor, meanWColor;
    std::string meanRColorInvert, meanGColorInvert, meanBColorInvert, meanWColorInvert;
    std::string avgMarker;
    std::string avgInvertMarker;

    std::string threeIntsToHexString(unsigned int a, unsigned int b, unsigned int c);
    std::string deleteExtraSpaces(std::string src);
    std::size_t countSpacesBeforeFind(std::string src, std::string findSrc);
    void loadConfig();
    void parseConfig(std::string line);
    void replaceMarker(std::string oldMarker, std::string newMarker);
    void calculateMarkers();


public:
    Wlswitch(std::string path = "", std::string newDelay = "180");
    ~Wlswitch();

    void switchWallpaper();
    void updateDependConfigs();
    unsigned int waitDelay();
};
#endif // WLSWITCH_H_INCLUDED

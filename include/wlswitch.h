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
#include "marker.h"
typedef struct
{
    Magick::Image::ImageStatistics all;
    Magick::Image::ImageStatistics leftSide;
    Magick::Image::ImageStatistics center;
    Magick::Image::ImageStatistics rightSide;
    Magick::Image::ImageStatistics upSide;
    Magick::Image::ImageStatistics downSide;
}ImagePartsStatistics;

class Wlswitch
{
private:
    std::string homePath;
    std::string configPath;
    std::string currentDir;
    std::string switcherProgram;
    std::string switcherArguments;
    std::string delay;
    std::string currentDependConfig;
    std::string currentWallpaper;
    std::vector<std::string> imageFormats;
    bool usingSubDirs;
    bool wallpaperChanged;
    //Error message strings
    std::string messageCreatedConfigDir;
    std::string messageCreatedCleanConfig;
    std::string errCreatingCleanConfig;
    std::string errEmptyDir;
    std::string errWrongDir;
    std::string errDirNotSeted;
    std::string errNoClosingQuote;
    std::string errNoEqualSymbol;
    std::string errOpeningDependConfig;
    std::string warnWrongNewMask;
    std::string errOpeningWallpaper;

    //Config file tag strings
    std::string autoBlockStartString;
    std::string autoBlockEndString;
    std::string maskConfigString;
    std::string autoConfigLineOnesString;
    std::string autoConfigLineString;

    //Script runs after depend configs updating by shellProgram
    std::string updateBeforeScript;
    std::string updateAfterScript;
    std::string shellProgram;

    std::unordered_map<std::string, ImagePartsStatistics> statisticsContainer;
    std::string meanRColor, meanGColor, meanBColor, meanWColor;
    std::string meanRColorInvert, meanGColorInvert, meanBColorInvert, meanWColorInvert;
    std::vector<Marker> markers;

    std::string deleteExtraSpaces(std::string src);
    std::size_t countSpacesBeforeFind(std::string src, std::string findSrc);
    std::vector<std::string> readImagesFromDir(std::string path);
    void loadConfig();
    void parseConfig(std::string line);
    void replaceMarker(std::string oldMarker, std::string newMarker);
    void calculateMarkers();
public:
    Wlswitch(std::string path = "", std::string newDelay = "180");

    void switchWallpaper();
    void updateDependConfigs();
    std::size_t waitDelay();
};
#endif // WLSWITCH_H_INCLUDED

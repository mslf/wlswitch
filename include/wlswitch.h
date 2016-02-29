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
using namespace std;

class Wlswitch
{
private:
    string homePath;
    string currentDir;
    string switcherProgram;
    string switcherArguments;
    string delay;
    string avgMarker;
    string avgInvertMarker;
    string currentWallpaper;
    string updateScript;
    string shellProgram;
    //Script runs after depend configs updating
    string meanRColor, meanGColor, meanBColor, meanWColor;
    string meanRColorInvert, meanGColorInvert, meanBColorInvert, meanWColorInvert;
    string currentDependConfig;

    void loadConfig();
    void parseConfig(string line);
    void replaceMarker(string oldMarker, string newMarker);
    void calculateMarkers();

public:
    Wlswitch(string path = "", string newDelay = "180");
    ~Wlswitch();

    void switchWallpaper();
    void updateDependConfigs();
    unsigned int waitDelay();
};
#endif // WLSWITCH_H_INCLUDED

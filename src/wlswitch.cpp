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
#include "wlswitch.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

using namespace std;

Wlswitch::Wlswitch(string path, string newDelay)
{
    currentDir = path;
    delay = newDelay;
    loadConfig();
    if(!configLoaded)
        cerr << "Error while config loading! Put the config file  to the ~/.config/wlswitch/wlswitch.conf!\n";

}

Wlswitch::~Wlswitch() {}

void Wlswitch::loadConfig()
{
    ifstream fin;
    string word[3];
    int k = 0;

    configLoaded = 0;
    string configPath = ((string)getenv("HOME") + "/.config/wlswitch/wlswitch.conf");

    fin.open(configPath.c_str());
    if(!fin.is_open())
        return;
    configLoaded = 1;

    while(!fin.eof()) {

        fin >> word[k];
        k++;
        ///Every three word puts to parser.
        if(3 == k) {

            k = 0;
            parseConfig(word);
        }
    }


}

void Wlswitch::switchWallpaper()
{
    string s;
    string countFile = (currentDir + "wallpapersCount.txt");
    string listFile = (currentDir + "wallpapersList.txt");
    string fileName;///Result wallpaper name

    int count = 0;
    ifstream fin;
    /*

        In current directory we get the list of images (jpg, png, etc) and puts it to
        wallpapersList.txt (which in current directory).
        After that we count them and puts count to the wallpapersCount.txt.
        Next stage is a randoming number between 1 and count, reading N lines from wallpapersList.txt (N = count).
        fileName will have path of the picture to switching.

    */
    srand (time (NULL));

    s = "find " + currentDir + " -type f > " + currentDir + "wallpapersList.txt";
    system(s.c_str());

    s = "grep jpg " + currentDir + "wallpapersList.txt > " + currentDir + "wl_jpg.txt";///Jpg pictures
    system(s.c_str());

    s = "grep png " + currentDir + "wallpapersList.txt > " + currentDir + "wl_png.txt";///Png pictures
    system(s.c_str());

    s = "rm " + currentDir + "wallpapersList.txt";
    system(s.c_str());

    s = "cat " + currentDir + "wl_jpg.txt " + currentDir + "wl_png.txt > " + currentDir + "wallpapersList.txt";
    system(s.c_str());

    s = "rm " + currentDir + "wl_jpg.txt";
    system(s.c_str());

    s = "rm " + currentDir + "wl_png.txt";
    system(s.c_str());

    s = "cat " + currentDir + "wallpapersList.txt | wc -l > " + currentDir + "wallpapersCount.txt";
    system(s.c_str());


    fin.open(countFile.c_str());
    if(!fin.is_open())
        return;
    fin >> count;

    fin.close();


    fin.open(listFile.c_str());
    if(!fin.is_open())
        return;

    count = rand() % count + 1;

    for(int i = 0; i < count; i++)
        fin >> fileName;

    fin.close();

    s = switcherProgram + " " + switcherArguments + " " + fileName;
    system(s.c_str());


}

void Wlswitch::updateDependConfigs()
{
    loadConfig();
}

void Wlswitch::parseConfig(string* words)
{


    if(words[2] == ";") {
        ///Path setting
        if(words[0] == "path") {
            ///Simple path validating. Path should begin and ends with "/" symbol.
            if(words[1].at(0) == words[1].at(words[1].length() - 1) && words[1][0] == '/')
                currentDir = words[1];
        }
        else
            ///Wallpaper switcher program setting
            if(words[0] == "switcher") {
                switcherProgram = words[1];
            }
        else
            ///Delay setting
            if(words[0] == "delay") {

                delay = words[1];
            }

        else
            ///Switcher program argument setting
            //if(words[0].find("key", 0) != string::npos){
            if(words[0] == "argument") {
                switcherArguments += words[1] + " ";
            }

        else

            if(words[1] == "avg") {

                replaceMarker(words[0], avgMarker);
            }
    }
}

unsigned int Wlswitch::waitDelay()
{

    return sleep (stoi (delay));
}

void Wlswitch::replaceMarker(string oldMarker, string newMarker) {} ///TODO

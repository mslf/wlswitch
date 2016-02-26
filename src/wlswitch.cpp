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
#include <sstream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <dirent.h>
#include <regex>
#include <Magick++.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>


using namespace std;
using namespace Magick;

Wlswitch::Wlswitch(string path, string newDelay)
{
    homePath = (string)getenv("HOME");
    currentDir = path;
    delay = newDelay;
    loadConfig();
    if (!configLoaded)
        cerr << "Error while config loading! Put the config file  to the ~/.config/wlswitch/wlswitch.conf!" << endl;

}

Wlswitch::~Wlswitch() {}

void Wlswitch::loadConfig()
{
    ifstream fin;
    string word[3];
    int k = 0;

    switcherArguments = "";
    configLoaded = false;
    string configPath = (homePath + (string)"/.config/wlswitch/wlswitch.conf");

    fin.open(configPath);
    if (!fin.is_open()){

        string tempConfigDirPath = homePath + (string)"/.config/wlswitch/";
        if (mkdir(tempConfigDirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
            cerr << "Created config directory in " << homePath << "/.config/" << endl;
        ofstream fout;
        fout.open(configPath);
       if (fout.is_open()){

            cerr << "Created clean config " << homePath << "/.config/wlswitch" << endl;
            fout.close();
        }
        else
           cerr << "Error while creating clean config! " << homePath << "/.config/wlswitch" << endl;
    }
    fin.close();
    fin.open(configPath);

    if (!fin.is_open())
        return;

    while (!fin.eof()) {

        fin >> word[k];
        k++;
        //Every three word puts to parser.
        if (3 == k) {

            k = 0;
            parseConfig(word);
        }
    }

    configLoaded = true;
}

void Wlswitch::switchWallpaper()
{
    if (currentDir != ""){

        vector<string> namesList;
        string resultFilename;
        DIR *d = NULL;
        struct dirent *dentry = NULL;
        string tempFilename;
        /*
            In current directory we get the list of images (jpg, png) and puts it to
            namesList string and count them.
            Then we random number between 0 and count of files and set the resultFilename to namesList[randNum].
        */
        if ((d = opendir(currentDir.c_str())) != NULL){

            while ((dentry = readdir(d)) != NULL){

                tempFilename = (string)dentry->d_name;
                if (tempFilename.find(".jpg", 0) != string::npos || tempFilename.find(".png", 0) != string::npos){

                    namesList.push_back(tempFilename);
                }
            }
            srand (time (NULL));
            unsigned long int randNum = rand() % namesList.size();
            resultFilename = namesList[randNum];

        } else
            cerr << "Error while wallpapers directory opening! Correct this path in config file!" << endl << (string)"Wrong path: " + currentDir << endl;

        currentWallpaper = currentDir + resultFilename;
        string temp = switcherProgram + (string)" " + switcherArguments + (string)" " + currentWallpaper;
        system(temp.c_str());

        //Computing wallpaper's characteristics.
        getMean();

        closedir(d);
        free(dentry);
    }
    else
        cerr << "Error while wallpaper switching! Path is not set in the config file!" << endl;
}

void Wlswitch::updateDependConfigs()
{
    loadConfig();
    if (shellProgram != "" && updateScript != "" && configLoaded){

        string temp = shellProgram + (string)" " + updateScript;
        system(temp.c_str());
    }
}

void Wlswitch::parseConfig(string* words)
{
    if(words[2] == ";") {
        //Path setting
        if (words[0] == "path") {
            //Simple path validating. Path should begin and ends with "/" symbol.
            if (words[1].at(0) == words[1].at(words[1].length() - 1) && words[1][0] == '/')
                currentDir = words[1];
            else
                cerr << "Error while wallpapers directory opening! Path should begin and ends with / symbol!" << endl;
        }
        else
            //Wallpaper switcher program setting
            if (words[0] == "switcher") {

                switcherProgram = words[1];
            }
        else
            //Delay setting
            if (words[0] == "delay") {

                delay = words[1];
            }

        else
            //Switcher program argument setting
            if (words[0] == "argument") {

                switcherArguments += words[1] + (string)" ";
            }

        else
            //Update script path
            if (words[0] == "updateScript") {

                updateScript = words[1];
            }

        else
            //Shell program
            if (words[0] == "shellProgram") {

                shellProgram = words[1];
            }

        else
            //Current depend config path setting
            if (words[0] == "dependConfig") {

                currentDependConfig = words[1];
            }

        else
            //Average color of the wallpaper marker
            if (words[1] == "avg") {

                replaceMarker(words[0], avgMarker);
            }

        else
            //Average color of the wallpaper marker
            if (words[1] == "avgInvert") {

                replaceMarker(words[0], avgInvertMarker);
            }
    }
}

unsigned int Wlswitch::waitDelay()
{
    return sleep (stoi (delay));
}

void Wlswitch::replaceMarker(string oldMarker, string newMarker)
{
    fstream fio;
    string maskConfigString = (string)"###<MASK_CONFIG_LINE> " + oldMarker + (string)" ";
    string errorMessage = "Error while depend config opening! Correct the depend config path in ~/.config/wlswitch/wlswitch.conf!\n";

    char temp_strLine[1000];
    //Yes,in  lines bigger than 1000 chars only 1000 will be processed.

    string strLine;
    string fileContain;
    string oldMask;
    string newMask;

    bool inAutoBlock = false;
    bool maskConfigured = false;

    fio.open(currentDependConfig, ios_base::in);
    if (!fio.is_open())
        cerr << errorMessage;

    while (!fio.eof()){

        fio.getline(temp_strLine, 1000);
        strLine = (string)temp_strLine;

        if (strLine.find("###<WLSWITCH_AUTO>", 0) != string::npos)
            inAutoBlock = true;
        if (strLine.find("###</WLSWITCH_AUTO>", 0) != string::npos)
            inAutoBlock = false;
        if (strLine.find(maskConfigString, 0) != string::npos && inAutoBlock){

            int position = strLine.find(maskConfigString, 0);

            position = strLine.find("#", position + maskConfigString.length());
            oldMask = strLine.substr(position + 1, strLine.find("#", position + 1) - position - 1);
            position = strLine.find("#", strLine.find("#", position + 1) + 1);
            newMask = strLine.substr(position + 1, strLine.find("#", position + 1) - position - 1);
            if (newMask.find("%", 0) == string::npos)
                cerr << "Warning! New mask has no % symbol. Is this error?\nLine: " << strLine << endl;
            else
                newMask.replace(newMask.find("%", 0), 1, newMarker);

            maskConfigured = true;
        }

        if (strLine.find((string)"###<AUTO_CONFIG_LINE_ONES> " + oldMarker, 0) != string::npos && inAutoBlock && maskConfigured){

            regex pattern (oldMask, regex::ECMAScript);
            smatch m;
            //For matching result
            fileContain += strLine + (string)"\n";
            //Next line after ###<AUTO_CONFIG_LINE_ONES> will be modified
            fio.getline(temp_strLine, 1000);
            strLine = (string)temp_strLine;

            if (newMarker != ""){

                //Replacing all pattern sequences to oldMarker
                while (regex_search(strLine, m, pattern)){

                    strLine.replace(m.position(), m.str().length(), oldMarker);
                }
                //Replacing all oldMarker sequences to newMarker
                while (strLine.find(oldMarker, 0) != string::npos){

                    strLine.replace(strLine.find(oldMarker, 0), oldMarker.length(), newMask);
                }
            }
        }
        /*
        if (strLine.find("###<AUTO_CONFIG_LINE_ONE>", 0) != string::npos && inAutoBlock && maskConfigured){

            regex pattern (oldMask, regex::ECMAScript);
            smatch m;
            //For matching result
            strLine.t
            fileContain += strLine + "\n";
            //Next line after ###<AUTO_CONFIG_LINE_ONES> will be modified
            fio.getline(temp_strLine, 1000);
            strLine = (string)temp_strLine;

            if (newMarker != ""){

                //Replacing all pattern sequences to oldMarker
                while (regex_search(strLine, m, pattern)){

                    strLine.replace(m.position(), m.str().length(), oldMarker);
                }
                //Replacing all oldMarker sequences to newMarker
                while (strLine.find(oldMarker, 0) != string::npos){

                    strLine.replace(strLine.find(oldMarker, 0), oldMarker.length(), newMask);
                }
            }
        }*///TODO
        fileContain += strLine + (string)"\n";
    }

    fio.close();
    //Saving to file
    fio.open(currentDependConfig, ios_base::out);
    if (!fio.is_open())
        cerr << errorMessage;
    fio.seekp(0, ios_base::beg);
    fileContain.erase(fileContain.length() - 1, 1);//Deleting \n symbol.
    fio << fileContain;
    fio.close();
    /*
        Needs to open currentDependConfig and search ###<WLSWITCH_AUTO>
        for e.g. conky's config

            #////////////////////////
            double_buffer yes
            TEXT
            ###<WLSWITCH_AUTO>
            ###<MASK_CONFIG_LINE> color_1 #${color *}# #${color %}#
            ###<MASK_CONFIG_LINE> color_2 #${color *}# #${color %}#
            ###<MASK_CONFIG_LINE> color_3 #${color *}# #${color %}#

            ###<AUTO_CONFIG_LINE> color_1 color_2 color_3
            CPU0: ${color 2E2E2E}${cpu cpu0} CPU1: ${color 2A2A2A}${cpu cpu1} CPU3: ${color 1E2E3E}${cpu cpu3}
            ###</WLSWITCH_AUTO>

            ###<WLSWITCH_AUTO>
            ###<MASK_CONFIG_LINE> color_1 #${color *}# #${color %}#
            ###<AUTO_CONFIG_LINE_ONES> color_1
            RAM: ${color }$mem / $memmax  ${alignr}${color #5588EE}
            ###</WLSWITCH_AUTO>
            #////////////////////////

            Then:
                the ${color 2E2E2E} will be replaced with color_1 marker (for e.g. avg)
                the ${color 2A2A2A} will be replaced with color_2 marker (for e.g. avg_w)
                the ${color 1E2E3E} will be replaced with color_3 marker (for e.g. avg_b)
    */
}

void Wlswitch::getMean()
{
    /*
        Calculating some characteristics of the wallpaper picture for e.g. average color (avg) e.t.c
    */

    //Using for converting int to hex string.
    if (configLoaded){

        stringstream convertStream;

        Image wallpaperImage;
        Image::ImageStatistics* wallpaperImageStats = new Image::ImageStatistics;

        try
        {
            wallpaperImage.read(currentWallpaper);
        }
        catch(Exception &error_ )
        {
            cerr << "Error while opening wallpaper file! Please check your config file!" << endl;
        }

        try
        {
            wallpaperImage.statistics(wallpaperImageStats);
        }
        catch(Exception &error_ )
        {
            cerr << "Error while getting wallpaper statistics! Please check your config file!" << endl;
        }

        unsigned int r, g, b;
        //Adduction the (0.0; 65535.0) numbers to (0; 255) format using magic number (65535).
        //In specification sayed that it must match (0.0; 1.0) format, but on my machine is not so.
        r = (unsigned int)(wallpaperImageStats->red.mean / 65535 * 255);
        g = (unsigned int)(wallpaperImageStats->green.mean / 65535 * 255);
        b = (unsigned int)(wallpaperImageStats->blue.mean / 65535 * 255);

        convertStream << setw(2) << setfill('0') << hex << r << setw(2) << setfill('0') << hex << g << setw(2) << setfill('0') << hex << b;
        avgMarker = (string)"#" + convertStream.str();
        convertStream.str("0");

        convertStream << setw(2) << setfill('0') << hex << (255 - r) << setw(2) << setfill('0') << hex << (255 - g) << setw(2) << setfill('0') << hex << (255 - b);
        avgInvertMarker = (string)"#" + convertStream.str();
        convertStream.str("0");

        delete wallpaperImageStats;
    }
}

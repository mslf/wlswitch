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

Wlswitch::Wlswitch(std::string path, std::string newDelay)
{
    homePath = (std::string)getenv("HOME");
    currentDir = path;
    delay = newDelay;
    loadConfig();
}

Wlswitch::~Wlswitch() {}

void Wlswitch::loadConfig()
{
    std::ifstream fin;
    std::string word[3];
    int k = 0;

    switcherArguments = "";
    std::string configPath = (homePath + (std::string)"/.config/wlswitch/wlswitch.conf");

    fin.open(configPath);
    if (!fin.is_open()){

        std::string tempConfigDirPath = homePath + (std::string)"/.config/wlswitch/";
        if (mkdir(tempConfigDirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
            std::cerr << "Created config directory in " << homePath << "/.config/" << std::endl;
        std::ofstream fout;
        fout.open(configPath);
       if (fout.is_open()){

           std::cerr << "Created clean config " << homePath << "/.config/wlswitch" << std::endl;
            fout.close();
        }
        else
           std::cerr << "Error while creating clean config! " << homePath << "/.config/wlswitch" << std::endl;
    }
    fin.close();
    fin.open(configPath);

    if (!fin.is_open())
        return;

    char tempString[1000];

    while (!fin.eof()) {

        fin.getline(tempString, 1000);
        parseConfig((std::string)tempString);
    }
}

void Wlswitch::switchWallpaper()
{
    if (currentDir != "" && switcherProgram != ""){

        std::vector<std::string> namesList;
        std::string resultFilename;
        DIR *d = NULL;
        struct dirent* dentry = NULL;
        std::string tempFilename;
        /*
            In current directory we get the list of images (jpg, png) and puts it to
            namesList string and count them.
            Then we random number between 0 and count of files and set the resultFilename to namesList[randNum].
        */
        if ((d = opendir(currentDir.c_str())) != NULL){

            while ((dentry = readdir(d)) != NULL){

                tempFilename = (std::string)dentry->d_name;
                if (tempFilename.find(".jpg", 0) != std::string::npos || tempFilename.find(".png", 0) != std::string::npos){

                    namesList.push_back(tempFilename);
                }
            }
            srand (time (NULL));
            if (namesList.size() != 0) {

                unsigned long int randNum = rand() % namesList.size();
                resultFilename = namesList[randNum];
            }else
                std::cerr << "Current wallpaper directory has no images!" << std::endl;
            closedir(d);
            free(dentry);
        } else
            std::cerr << "Error while wallpapers directory opening! Correct this path in config file!" << std::endl << (std::string)"Wrong path: " + currentDir << std::endl;
        if (resultFilename != ""){

            currentWallpaper = currentDir + resultFilename;
            std::string temp = switcherProgram + (std::string)" " + switcherArguments + (std::string)" " + currentWallpaper;
            system(temp.c_str());

            //Computing wallpaper's characteristics.
            calculateMarkers();
        }
    }
    else
        std::cerr << "Error while wallpaper switching! Path or switcher program is not set in the config file!" << std::endl;
}

void Wlswitch::updateDependConfigs()
{
    loadConfig();
    if (shellProgram != "" && updateScript != ""){

        std::string temp = shellProgram + (std::string)" " + updateScript;
        system(temp.c_str());
    }
}

void Wlswitch::parseConfig(std::string line)
{
    //Ignoring lines which contain '#' symbol
    if (line.find('#', 0) != std::string::npos)
        return;
    //Ignoring empty lines
    if (line.length() == 0)
        return;
    if (line.find('=', 0) != std::string::npos){

        std::string leftOperand = line.substr(0, line.find('=', 0));
        std::string rightOperand = line.substr(line.find('=', 0) + 1, line.length() - line.find('=', 0));
        unsigned int i = 0;
        bool inQuoteFlag = false;
        bool parityQuotesCountFlag = true;

        //Checking quotes count
        for (i = 0; i <rightOperand.length(); i++)
            if(rightOperand[i] == '"')
                parityQuotesCountFlag = !parityQuotesCountFlag;
        i = 0;
        if (!parityQuotesCountFlag){

            std::cerr << "Error while parsing config line! There is no closing quote!" << std::endl << "Line: <" << line << ">" << std::endl;
            return;
        }
        //Deleting spaces and tabulations from leftOperand string
        i = 0;
        while (i <= leftOperand.length()){

            if (leftOperand[i] == ' ' || leftOperand[i] == '\t')
                leftOperand.erase(i, 1);
            else
                i++;
        }
        //Deleting spaces and tabulations from non-quoted rightOperand string parts
        i = 0;
        while (i <= rightOperand.length()){

            if(rightOperand[i] == '\"')
                inQuoteFlag = !inQuoteFlag;
            if (!inQuoteFlag && (rightOperand[i] == ' ' || rightOperand[i] == '\t'))
                rightOperand.erase(i, 1);
            else
                i++;
        }
        //Deleting quotes from rightOperand string.
        i = 0;
        while (i <= rightOperand.length()){

            if(rightOperand[i] == '\"')
                rightOperand.erase(i, 1);
            i++;
        }

        //Checking leftOperand for markers or config flags
        //Path setting
        if (leftOperand == "path")
            currentDir = rightOperand;
        //Wallpaper switcher program setting
        if (leftOperand == "switcher")
            switcherProgram = rightOperand;
        //Delay setting
        if (leftOperand == "delay")
            delay = rightOperand;
        //Switcher program argument setting
        if (leftOperand == "argument")
            switcherArguments += rightOperand + (std::string)" ";
        //Update script path setting
        if (leftOperand == "updateScript")
            updateScript = rightOperand;
        //Shell program setting
        if (leftOperand == "shellProgram")
            shellProgram = rightOperand;
        //Current depend config path setting
        if (leftOperand == "dependConfig")
            currentDependConfig = rightOperand;

        //Average color of the wallpaper marker
        if (rightOperand == "avg")
            replaceMarker(leftOperand, avgMarker);
        //Invert average color of the wallpaper marker
        if (rightOperand == "avgInvert")
            replaceMarker(leftOperand, avgInvertMarker);
    } else
        std::cerr << "Error while parsing config line! There is no \'=\' symbol!" << std::endl << "Line: <" << line << ">" << std::endl;
}

unsigned int Wlswitch::waitDelay()
{
    return sleep (stoi (delay));
}

void Wlswitch::replaceMarker(std::string oldMarker, std::string newMarker)
{
    std::fstream fio;
    std::string maskConfigString = (std::string)"###<MASK_CONFIG_LINE> " + oldMarker + (std::string)" ";
    std::string errorMessage = "Error while depend config opening! Correct the depend config path in ~/.config/wlswitch/wlswitch.conf!\n";

    char temp_strLine[1000];
    //Yes,in  lines bigger than 1000 chars only 1000 will be processed.

    std::string strLine;
    std::string fileContain;
    std::string oldMask;
    std::string newMask;

    bool inAutoBlock = false;
    bool maskConfigured = false;

    fio.open(currentDependConfig, std::ios_base::in);
    if (!fio.is_open())
        std::cerr << errorMessage;

    while (!fio.eof()){

        fio.getline(temp_strLine, 1000);
        strLine = (std::string)temp_strLine;

        if (strLine.find("###<WLSWITCH_AUTO>", 0) != std::string::npos)
            inAutoBlock = true;
        if (strLine.find("###</WLSWITCH_AUTO>", 0) != std::string::npos)
            inAutoBlock = false;
        if (strLine.find(maskConfigString, 0) != std::string::npos && inAutoBlock){

            int position = strLine.find(maskConfigString, 0);

            position = strLine.find("#", position + maskConfigString.length());
            oldMask = strLine.substr(position + 1, strLine.find("#", position + 1) - position - 1);
            position = strLine.find("#", strLine.find("#", position + 1) + 1);
            newMask = strLine.substr(position + 1, strLine.find("#", position + 1) - position - 1);
            if (newMask.find("%", 0) == std::string::npos)
                std::cerr << "Warning! New mask has no % symbol. Is this error?\nLine: " << strLine << std::endl;
            else
                newMask.replace(newMask.find("%", 0), 1, newMarker);

            maskConfigured = true;
        }

        if (strLine.find((std::string)"###<AUTO_CONFIG_LINE_ONES> " + oldMarker, 0) != std::string::npos && inAutoBlock && maskConfigured){

            std::regex pattern (oldMask, std::regex::ECMAScript);
            std::smatch m;
            //For matching result
            fileContain += strLine + (std::string)"\n";
            //Next line after ###<AUTO_CONFIG_LINE_ONES> will be modified
            fio.getline(temp_strLine, 1000);
            strLine = (std::string)temp_strLine;

            if (newMarker != ""){

                //Replacing all pattern sequences to oldMarker
                while (regex_search(strLine, m, pattern)){

                    strLine.replace(m.position(), m.str().length(), oldMarker);
                }
                //Replacing all oldMarker sequences to newMarker
                while (strLine.find(oldMarker, 0) != std::string::npos){

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
        fileContain += strLine + (std::string)"\n";
    }

    fio.close();
    //Saving to file
    fio.open(currentDependConfig, std::ios_base::out);
    if (!fio.is_open())
        std::cerr << errorMessage;
    fio.seekp(0, std::ios_base::beg);
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
    *///TODO into doc
}

void Wlswitch::calculateMarkers()
{
    /*
        Calculating some characteristics of the wallpaper picture for e.g. average color (avg) e.t.c
    */
    if (currentWallpaper != ""){

        std::stringstream convertStream;
        //Using for converting int to hex string.
        Magick::Image* wallpaperImage = new Magick::Image;
        Magick::Image::ImageStatistics wallpaperImageStats;
        try
        {
            wallpaperImage->read(currentWallpaper);
        }
        catch(Magick::Exception &error_ )
        {
            std::cerr << "Error while opening wallpaper file! Please check your config file!" << std::endl;
            return;
        }
        std::unordered_map<std::string, Magick::Image::ImageStatistics>::const_iterator got = statisticsContainer.find(currentWallpaper);

        if (got == statisticsContainer.end()){

            wallpaperImage->statistics(&wallpaperImageStats);
            std::pair<std::string, Magick::Image::ImageStatistics> pairToAdd (currentWallpaper, wallpaperImageStats);
            statisticsContainer.insert(pairToAdd);
            std::cout << "Added: " << currentWallpaper << " to statistics container." << std::endl;
        }
        else{

            wallpaperImageStats = got->second;
            std::cout << "Readed: " << currentWallpaper << " from statistics container." << std::endl;
        }


        delete wallpaperImage;


        unsigned int r, g, b;
        //Adduction the (0.0; 65535.0) numbers to (0; 255) format using magic number (65535).
        //In specification sayed that it must match (0.0; 1.0) format, but on my machine is not so.
        r = (unsigned int)(wallpaperImageStats.red.mean / 65535 * 255);
        g = (unsigned int)(wallpaperImageStats.green.mean / 65535 * 255);
        b = (unsigned int)(wallpaperImageStats.blue.mean / 65535 * 255);

        convertStream << std::setw(2) << std::setfill('0') << std::hex << r;
        convertStream << std::setw(2) << std::setfill('0') << std::hex << g;
        convertStream << std::setw(2) << std::setfill('0') << std::hex << b;
        avgMarker = (std::string)"#" + convertStream.str();
        convertStream.str("0");

        convertStream << std::setw(2) << std::setfill('0') << std::hex << (255 - r);
        convertStream << std::setw(2) << std::setfill('0') << std::hex << (255 - g);
        convertStream << std::setw(2) << std::setfill('0') << std::hex << (255 - b);
        avgInvertMarker = (std::string)"#" + convertStream.str();
        convertStream.str("0");
    }
}

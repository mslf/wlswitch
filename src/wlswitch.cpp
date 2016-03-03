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
            Then we random number between 0 and count of files and set the resultFilename to namesList[randNum]
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

            //Computing wallpaper's characteristics
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
        //Deleting quotes from rightOperand string
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
    std::string autoBlockStartString = "###<WLSWITCH_AUTO>";
    std::string autoBlockEndString = "###</WLSWITCH_AUTO>";
    std::string maskConfigString = "###<MASK_CONFIG_LINE>";
    std::string autoConfigLineOnesString = "###<AUTO_CONFIG_LINE_ONES>";
    std::string autoConfigLineString = "###<AUTO_CONFIG_LINE>";

    std::fstream fio;
    std::string errorMessage = "Error while depend config opening! Correct the depend config path in ~/.config/wlswitch/wlswitch.conf!\n";

    char temp_strLine[1000];
    //Yes,in  lines bigger than 1000 chars only 1000 will be processed

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
        //Auto block start and end
        if (strLine.find(autoBlockStartString, 0) != std::string::npos)
            inAutoBlock = true;
        if (strLine.find(autoBlockEndString, 0) != std::string::npos)
            inAutoBlock = false;
        //Mask configuring
        if (strLine.find(maskConfigString, 0) != std::string::npos && inAutoBlock){

            strLine = deleteExtraSpaces(strLine);
            if (strLine.find(oldMarker, 0) != std::string::npos){

                int position = strLine.find(oldMarker, 0);
                position = strLine.find("#", position + oldMarker.length());
                oldMask = strLine.substr(position + 1, strLine.find("#", position + 1) - position - 1);
                position = strLine.find("#", strLine.find("#", position + 1) + 1);
                newMask = strLine.substr(position + 1, strLine.find("#", position + 1) - position - 1);
                if (newMask.find("%", 0) == std::string::npos)
                    std::cerr << "Warning! New mask has no % symbol. Is this error?\nLine: " << strLine << std::endl;
                else
                    newMask.replace(newMask.find("%", 0), 1, newMarker);

                maskConfigured = true;
            }
        }
        //One-colored string's markers replacing
        if (strLine.find(autoConfigLineOnesString, 0) != std::string::npos && inAutoBlock && maskConfigured)
            if (strLine.find(oldMarker, strLine.find(autoConfigLineOnesString, 0)) != std::string::npos){
                std::regex pattern (oldMask, std::regex::ECMAScript);
                //For matching result
                std::smatch m;
                //Next line after ###<AUTO_CONFIG_LINE_ONES> will be modified
                fileContain += strLine + (std::string)"\n";
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
        //Multi-colored string's markers replacing
        if (strLine.find(autoConfigLineString, 0) != std::string::npos && inAutoBlock && maskConfigured){

            std::regex pattern (oldMask, std::regex::ECMAScript);
            //For matching result
            std::smatch m;
            std::string tempMarkerBase = "tempMarker";
            std::vector<std::string> tempMarkers;
            std::vector<std::string> tempMarkersSaved;
            strLine = deleteExtraSpaces(strLine);
            std::string markersString = strLine.substr(strLine.find(autoConfigLineString, 0) + autoConfigLineString.length() + 1, strLine.length() - autoConfigLineString.length());
            //Deleting first space symbol if exist
            markersString = deleteExtraSpaces(markersString);
            //Counting spaces in string without oldMarker (spaceCount + 1 == oldMarker position in next line)
            std::size_t spacesCount =  countSpacesBeforeFind(markersString, oldMarker);
            //Next line after ###<AUTO_CONFIG_LINE> will be modified
            fileContain += strLine + "\n";
            fio.getline(temp_strLine, 1000);
            strLine = (std::string)temp_strLine;

            if (newMarker != ""){

                //Replacing only one pattern sequences, which have the same number as 'spaceCount + 1' to oldMarker
                std::size_t currentMarkerPosition = 0;

                while (regex_search(strLine, m, pattern)){

                    currentMarkerPosition++;
                    if (currentMarkerPosition == spacesCount + 1){

                        strLine.replace(m.position(), m.str().length(), oldMarker);
                        //Delete oldMarker from markerString and recount spacesCount (for each oldMarker in string)
                        if (markersString.find(oldMarker, 0) + oldMarker.length() + 1 < markersString.length())
                            markersString = markersString.substr(markersString.find(oldMarker, 0) + oldMarker.length() + 1, markersString.length() - oldMarker.length());
                        if (markersString.find(oldMarker, 0) != std::string::npos)
                            spacesCount +=  countSpacesBeforeFind(markersString, oldMarker) + 1;
                    }
                    else{

                        tempMarkersSaved.push_back(strLine.substr(m.position(), m.str().length()));
                        strLine.replace(m.position(), m.str().length(), tempMarkerBase + (char)tempMarkersSaved.size());
                        tempMarkers.push_back(tempMarkerBase + (char)tempMarkersSaved.size());
                    }

                }
                //Replacing all oldMarker sequences to newMarker
                while (strLine.find(oldMarker, 0) != std::string::npos){

                    strLine.replace(strLine.find(oldMarker, 0), oldMarker.length(), newMask);
                }
                //Replacing all tempMarker sequences to saved tempMarker
                for (std::size_t i = 0; i < tempMarkers.size(); i++)
                    strLine.replace(strLine.find(tempMarkers[i], 0), tempMarkers[i].length(), tempMarkersSaved[i]);
            }
        }//TODO
        fileContain += strLine + (std::string)"\n";
    }

    fio.close();
    //Saving to file
    fio.open(currentDependConfig, std::ios_base::out);
    if (!fio.is_open())
        std::cerr << errorMessage;
    fio.seekp(0, std::ios_base::beg);
    fileContain.erase(fileContain.length() - 1, 1);//Deleting \n symbol
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
            ###<AUTO_CONFIG_LINE>color_1 color_2 color_3
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
        }
        else
            wallpaperImageStats = got->second;

        delete wallpaperImage;


        unsigned int r, g, b;
        //Adduction the (0.0; 65535.0) numbers to (0; 255) format using magic number (65535)
        //In specification sayed that it must match (0.0; 1.0) format, but on my machine is not so
        r = (unsigned int)(wallpaperImageStats.red.mean / 65535 * 255);
        g = (unsigned int)(wallpaperImageStats.green.mean / 65535 * 255);
        b = (unsigned int)(wallpaperImageStats.blue.mean / 65535 * 255);

        avgMarker = threeIntsToHexString(r, g, b);
        avgInvertMarker = threeIntsToHexString(255 - r, 255 - g, 255 - b);
    }
}

std::string Wlswitch::threeIntsToHexString(unsigned int a, unsigned int b, unsigned int c)
{
    std::stringstream convertStream;
    //Using for converting int to hex string
    convertStream << std::setw(2) << std::setfill('0') << std::hex << a;
    convertStream << std::setw(2) << std::setfill('0') << std::hex << b;
    convertStream << std::setw(2) << std::setfill('0') << std::hex << c;
    return ((std::string)"#" + convertStream.str());
}

std::string Wlswitch::deleteExtraSpaces(std::string src)
{
    std::size_t i = 0;
    std::size_t j = 0;

    //Replace all tabulations by space symbols

    while (src.find('\t', 0) != std::string::npos)
        src[src.find('\t', 0)] =  ' ';
    //Delete all spaces before text begining
    while (src[0] == ' ')
        src.erase(0, 1);
    //Replace all sequences with spaces by first symbol
    i = 0;
    while (src.find(' ', i) != std::string::npos){

        j = src.find(' ', i);
        while (src[j + 1] == ' ' && j < src.length() - 1)
            src.erase(j + 1, 1);
        i = j + 1;
    }

    //Deleting space at the end of string if exist
    if (src[src.length() - 1] == ' ')
        src.erase(src.length() - 1, 1);

    return src;
}

std::size_t Wlswitch::countSpacesBeforeFind(std::string src, std::string findSrc)
{
    std::size_t spacesCount = 0;
    std::size_t i = 0;
    src = src.substr(0, src.find(findSrc, 0));
    while (src.find(' ', i) != std::string::npos){

        spacesCount++;
        i = src.find(' ', i) + 1;
    }
    return spacesCount;
}
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
#include "marker.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

#include <fstream>
#include <unistd.h>
#include <dirent.h>
#include <regex>
#include <Magick++.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <wlswitch.h>

Wlswitch::Wlswitch(std::string path, std::string newDelay)
{
    homePath = std::string(getenv("HOME"));
    configPath = (homePath + std::string("/.config/wlswitch/wlswitch.conf"));
    currentDir = path;
    delay = newDelay;
    wallpaperChanged = false;
    usingSubDirs = false;

    //Error message strings init
    messageCreatedConfigDir = "Created config directory in ";
    messageCreatedCleanConfig = std::string("Created clean config ") + configPath;
    errCreatingCleanConfig = std::string("Error while creating clean config! ") + configPath;
    errEmptyDir = "Current wallpaper directory has no images!";
    errWrongDir = std::string("Error while wallpapers directory opening!"
                                      " Correct this path in config file!\nWrong path: ") + currentDir;
    errDirNotSeted = std::string("Error while wallpaper switching! "
                                         "Path or switcher program is not set in the config file!");
    errNoClosingQuote = std::string("Error while parsing config line! There is no closing quote!\nLine: <");
    errNoEqualSymbol = std::string("Error while parsing config line! There is no \'=\' symbol!\nLine: <");
    errOpeningDependConfig = std::string("Error while depend config opening! "
                                                 "Correct the depend config path in ~/.config/wlswitch/wlswitch.conf!\n "
                                                 "Depend config: ");
    warnWrongNewMask = "Warning! New mask has no % symbol. Is this error?\nLine: ";
    errOpeningWallpaper = "Error while opening wallpaper file! Please check your config file!";

    //Config file tag strings init
    autoBlockStartString = std::string("###<WLSWITCH_AUTO>");
    autoBlockEndString = std::string("###</WLSWITCH_AUTO>");
    maskConfigString = std::string("###<MASK_CONFIG_LINE>");
    autoConfigLineOnesString = std::string("###<AUTO_CONFIG_LINE_ONES>");
    autoConfigLineString = std::string("###<AUTO_CONFIG_LINE>");

    loadConfig();

    markers.push_back(Marker("avg"));
    markers.push_back(Marker("smart"));
    markers.push_back(Marker("leftSide"));
    markers.push_back(Marker("rightSide"));
    markers.push_back(Marker("upSide"));
    markers.push_back(Marker("downSide"));
    markers.push_back(Marker("center"));
}

void Wlswitch::loadConfig()
{
    std::ifstream fin(configPath);

    if (!fin.is_open()){
        std::string tempConfigDirPath = homePath + std::string("/.config/wlswitch/");
        if (mkdir(tempConfigDirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
            std::cerr << messageCreatedConfigDir << homePath << "/.config/" << std::endl;
        std::ofstream fout(configPath);
       if (fout.is_open()){
           std::cerr << messageCreatedCleanConfig << std::endl;
            fout.close();
        }
        else
           std::cerr << errCreatingCleanConfig << std::endl;
    }
    fin.close();
    fin.open(configPath);
    if (!fin.is_open())
        return;
    switcherArguments = "";
    imageFormats.clear();
    char tempString[MAX_LINE_LENGTH];
    while (!fin.eof()) {
        fin.getline(tempString, MAX_LINE_LENGTH);
        parseConfig((std::string)tempString);
    }
}

void Wlswitch::switchWallpaper()
{
    if (!isLocked())
        if (!currentDir.empty() && !switcherProgram.empty()){
            std::vector<std::string> namesList;
            std::string resultFilename;
            /*
                In current directory we get the list of images (jpg, png) and puts it to
                namesList string and count them.
                Then we random number between 0 and count of files and set the resultFilename to namesList[randNum]
            */
                namesList = readImagesFromDir(currentDir);
                srand (time (NULL));
                if (namesList.size() != 0) {
                    unsigned long int randNum = rand() % namesList.size();
                    if (currentWallpaper != currentDir + namesList[randNum]){
                        resultFilename = namesList[randNum];
                        wallpaperChanged = true;
                    }
                    else
                        wallpaperChanged = false;
                }else
                    std::cerr << errEmptyDir << std::endl;
            if (!resultFilename.empty() && wallpaperChanged){
                currentWallpaper = currentDir + resultFilename;
                std::string temp = switcherProgram + std::string(" ") +
                        switcherArguments + std::string(" \"") +
                        currentWallpaper + std::string("\"\n");
                system(temp.c_str());

                //Computing wallpaper's characteristics
                calculateMarkers();
            }
        }
        else
            std::cerr << errDirNotSeted << std::endl;
}

void Wlswitch::updateDependConfigs()
{
    if (!shellProgram.empty() && !updateBeforeScript.empty() && wallpaperChanged && !isLocked()){
        std::string temp = shellProgram + std::string(" ") + updateBeforeScript;
        system(temp.c_str());
    }
    loadConfig();
    if (!shellProgram.empty() && !updateAfterScript.empty() && wallpaperChanged && !isLocked()){
        std::string temp = shellProgram + std::string(" ") + updateAfterScript;
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
        std::size_t i = 0;
        bool inQuoteFlag = false;
        bool parityQuotesCountFlag = true;
        //Checking quotes count
        for (i = 0; i < rightOperand.length(); i++)
            if('\"' == rightOperand[i])
                parityQuotesCountFlag = !parityQuotesCountFlag;
        i = 0;
        if (!parityQuotesCountFlag){
            std::cerr << errNoClosingQuote << line << ">" << std::endl;
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
            if(rightOperand[i] == '"')
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
        //Lock-file path setting
        if (leftOperand == "lockFilePath")
            lockFilePath = rightOperand;
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
            switcherArguments += rightOperand + std::string(" ");
        //usingRecursiveDirs flag setting
        if(leftOperand == "usingSubDirs"){
            if (rightOperand == "1" || rightOperand == "true" || rightOperand == "yes")
                usingSubDirs = true;
            else
                usingSubDirs = false;
        }
        //Image formats setting
        if (leftOperand == "format"){
            for (i = 0; i < imageFormats.size(); i++)
                if (imageFormats[i] == rightOperand)
                    return;
            imageFormats.push_back(rightOperand);
        }
        //Update script path setting
        if (leftOperand == "updateBeforeScript")
            updateBeforeScript = rightOperand;
        if (leftOperand == "updateAfterScript")
            updateAfterScript = rightOperand;
        //Shell program setting
        if (leftOperand == "shellProgram")
            shellProgram = rightOperand;
        //Current depend config path setting
        if (leftOperand == "dependConfig")
            currentDependConfig = rightOperand;

        //Reading markers
        if (wallpaperChanged) {
            for (i = 0; i < markers.size(); i++)
                if (!markers[i].getSelectedString(rightOperand).empty())
                    replaceMarker(leftOperand, markers[i].getSelectedString(rightOperand));

            if (rightOperand == "currentWallpaperPath")
                replaceMarker(leftOperand, currentWallpaper);
        }
    } else
        std::cerr << errNoEqualSymbol << line << ">" << std::endl;
}

std::size_t Wlswitch::waitDelay()
{
    return sleep(stoi(delay));
}

void Wlswitch::replaceMarker(std::string oldMarker, std::string newMarker)
{
    std::fstream fio;
    //Yes,in  lines bigger than MAX_LINE_LENGTH chars only MAX_LINE_LENGTH will be processed
    char temp_strLine[MAX_LINE_LENGTH];
    std::string strLine;
    std::string fileContain;
    std::string oldMask;
    std::string newMask;
    bool inAutoBlock = false;
    bool maskConfigured = false;

    fio.open(currentDependConfig, std::ios_base::in);
    if (!fio.is_open())
        std::cerr << errOpeningDependConfig << currentDependConfig << std::endl;
    while (!fio.eof()){
        fio.getline(temp_strLine, MAX_LINE_LENGTH);
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
                if (newMask.find("%", 0) == std::string::npos) {
                    std::cerr << warnWrongNewMask << strLine << std::endl;
                } else {
                    newMask.replace(newMask.find("%", 0), 1, newMarker);
                }
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
                fio.getline(temp_strLine, MAX_LINE_LENGTH);
                strLine = (std::string)temp_strLine;
                if (newMarker != ""){
                    //Replacing all pattern sequences to oldMarker
                    while (regex_search(strLine, m, pattern))
                        strLine.replace(m.position(), m.str().length(), oldMarker);
                    //Replacing all oldMarker sequences to newMarker
                    while (strLine.find(oldMarker, 0) != std::string::npos)
                        strLine.replace(strLine.find(oldMarker, 0), oldMarker.length(), newMask);
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
            //Making string without 'autoConfigLineString' (only markers with spaces)
            std::string markersString = strLine.substr(strLine.find(autoConfigLineString, 0) +
                                                               autoConfigLineString.length() + 1,
                                                       strLine.length() - autoConfigLineString.length());
            //Deleting first space symbol if exist
            markersString = deleteExtraSpaces(markersString);
            //Counting spaces in string without oldMarker (spaceCount  == oldMarker position in next line)
            std::size_t spacesCount = countSpacesBeforeFound(markersString, oldMarker);
            //Read next line after ###<AUTO_CONFIG_LINE> which will be modified
            fileContain += strLine + "\n";
            fio.getline(temp_strLine, MAX_LINE_LENGTH);
            strLine = (std::string)temp_strLine;
            if (newMarker != "" && markersString.find(oldMarker, 0) != std::string::npos){
                //Replacing only one pattern sequences, which have the same number as 'spaceCount + 1' to oldMarker
                //If number not match it will be replaced with tempMarker (different for each sequence)
                std::size_t currentMarkerPosition = 0;
                while (regex_search(strLine, m, pattern)){
                    if (currentMarkerPosition == spacesCount){
                        strLine.replace(m.position(), m.str().length(), oldMarker);
                        //Delete oldMarker from markerString and recount spacesCount (for each oldMarker in string)
                        if (markersString.find(oldMarker, 0) + oldMarker.length() < markersString.length())
                            markersString = markersString.substr(markersString.find(oldMarker, 0) + oldMarker.length() + 1,
                                                                 markersString.length() - oldMarker.length());
                        if (markersString.find(oldMarker, 0) != std::string::npos)
                            spacesCount += countSpacesBeforeFound(markersString, oldMarker) + 1;
                    }
                    else{
                        tempMarkersSaved.push_back(strLine.substr(m.position(), m.str().length()));
                        strLine.replace(m.position(), m.str().length(), tempMarkerBase + (char)tempMarkersSaved.size());
                        tempMarkers.push_back(tempMarkerBase + (char)tempMarkersSaved.size());
                    }
                    currentMarkerPosition++;
                }
                //Replacing all oldMarker sequences to newMarker
                while (strLine.find(oldMarker, 0) != std::string::npos)
                    strLine.replace(strLine.find(oldMarker, 0), oldMarker.length(), newMask);
                //Replacing all tempMarker sequences to saved tempMarker
                for (std::size_t i = 0; i < tempMarkers.size(); i++)
                    strLine.replace(strLine.find(tempMarkers[i], 0), tempMarkers[i].length(), tempMarkersSaved[i]);
            }
        }
        fileContain += strLine + std::string("\n");
    }
    fio.close();
    //Saving to file
    fio.open(currentDependConfig, std::ios_base::out);
    if (!fio.is_open())
        std::cerr << errOpeningDependConfig << currentDependConfig << std::endl;
    fio.seekp(0, std::ios_base::beg);
    fileContain.erase(fileContain.length() - 1, 1);//Deleting \n symbol
    fio << fileContain;
    fio.close();
}

void Wlswitch::calculateMarkers()
{
    /*
        Calculating some characteristics of the wallpaper picture for e.g. average color (avg) e.t.c
    */
    if (!currentWallpaper.empty()){
        Magick::Image wallpaperImage;
        Magick::Image cropImage;
        ImagePartsStatistics wallpaperImageStats;
        try
        {
            wallpaperImage.read(currentWallpaper);
        }
        catch(Magick::Exception &error_ )
        {
            std::cerr << errOpeningWallpaper << std::endl;
            return;
        }
        std::unordered_map<std::string, ImagePartsStatistics>::const_iterator got =
                statisticsContainer.find(currentWallpaper);
        if (got == statisticsContainer.end()){
            wallpaperImage.statistics(&wallpaperImageStats.all);
            size_t imageHeight = wallpaperImage.size().height();
            size_t imageWidth = wallpaperImage.size().width();
            //Calculating upSide image statistics
            cropImage = wallpaperImage;
            cropImage.crop(Magick::Geometry(imageWidth,
                                            imageHeight / 10,
                                            0,
                                            0));
            cropImage.statistics(&wallpaperImageStats.upSide);

            //Calculating downSide image statistics
            cropImage = wallpaperImage;
            cropImage.crop(Magick::Geometry(imageWidth,
                                            imageHeight / 10,
                                            0,
                                            imageHeight - imageHeight / 10));
            cropImage.statistics(&wallpaperImageStats.downSide);

            //Calculating leftSide image statistics
            cropImage = wallpaperImage;
            cropImage.crop(Magick::Geometry(imageWidth / 2,
                                            imageHeight,
                                            0,
                                            0));
            cropImage.statistics(&wallpaperImageStats.leftSide);

            //Calculating rightSide image statistics
            cropImage = wallpaperImage;
            cropImage.crop(Magick::Geometry(imageWidth / 2,
                                            imageHeight,
                                            imageWidth - imageWidth / 2 ,
                                            0));
            cropImage.statistics(&wallpaperImageStats.rightSide);

            //Calculating center image statistics
            cropImage = wallpaperImage;
            cropImage.crop(Magick::Geometry(imageWidth / 3,
                                            imageHeight / 3,
                                            (imageWidth - imageWidth / 3) / 2,
                                            (imageHeight - imageHeight / 3) / 2));
            cropImage.statistics(&wallpaperImageStats.center);

            std::pair<std::string, ImagePartsStatistics> pairToAdd (currentWallpaper, wallpaperImageStats);
            statisticsContainer.insert(pairToAdd);
        }
        else
            wallpaperImageStats = got->second;
        double r, g, b;
        //Adduction the (0.0; 65535.0) numbers to (0; 255) format using magic number (65535)
        //In specification sayed that it must match (0.0; 1.0) format, but on my machine is not so
        //Markers for 'all' zone
        r = (wallpaperImageStats.all.red.mean / 65535 * 255);
        g = (wallpaperImageStats.all.green.mean / 65535 * 255);
        b = (wallpaperImageStats.all.blue.mean / 65535 * 255);

        for (std::size_t i = 0; i < markers.size(); i++){
            if (markers[i].getName() == "avg")
                markers[i].setMarker(r, g, b);
            if (markers[i].getName() == "smart"){
                if ((r + g + b) / 3 > (255 - r + 255 - g + 255 - b) / 3)
                    markers[i].setMarker(255 - r, 255 - g, 255 - b);
                else
                    markers[i].setMarker(r, g, b);
            }
        }
        //Markers for 'center' zone
        r = (wallpaperImageStats.center.red.mean / 65535 * 255);
        g = (wallpaperImageStats.center.green.mean / 65535 * 255);
        b = (wallpaperImageStats.center.blue.mean / 65535 * 255);

        for (std::size_t i = 0; i < markers.size(); i++)
            if (markers[i].getName() == "center")
                markers[i].setMarker(r, g, b);
        //Markers for 'leftSide' zone
        r = (wallpaperImageStats.leftSide.red.mean / 65535 * 255);
        g = (wallpaperImageStats.leftSide.green.mean / 65535 * 255);
        b = (wallpaperImageStats.leftSide.blue.mean / 65535 * 255);

        for (std::size_t i = 0; i < markers.size(); i++)
            if (markers[i].getName() == "leftSide")
                markers[i].setMarker(r, g, b);
        //Markers for 'rightSide' zone
        r = (wallpaperImageStats.rightSide.red.mean / 65535 * 255);
        g = (wallpaperImageStats.rightSide.green.mean / 65535 * 255);
        b = (wallpaperImageStats.rightSide.blue.mean / 65535 * 255);

        for (std::size_t i = 0; i < markers.size(); i++)
            if (markers[i].getName() == "rightSide")
                markers[i].setMarker(r, g, b);
        //Markers for 'upSide' zone
        r = (wallpaperImageStats.upSide.red.mean / 65535 * 255);
        g = (wallpaperImageStats.upSide.green.mean / 65535 * 255);
        b = (wallpaperImageStats.upSide.blue.mean / 65535 * 255);

        for (std::size_t i = 0; i < markers.size(); i++)
            if (markers[i].getName() == "upSide")
                markers[i].setMarker(r, g, b);
        //Markers for 'downSide' zone
        r = (wallpaperImageStats.downSide.red.mean / 65535 * 255);
        g = (wallpaperImageStats.downSide.green.mean / 65535 * 255);
        b = (wallpaperImageStats.downSide.blue.mean / 65535 * 255);

        for (std::size_t i = 0; i < markers.size(); i++)
            if (markers[i].getName() == "downSide")
                markers[i].setMarker(r, g, b);

    }
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

std::size_t Wlswitch::countSpacesBeforeFound(std::string src, std::string findSrc)
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

std::vector<std::string> Wlswitch::readImagesFromDir(std::string path)
{
    std::vector<std::string> namesList;
    if (!path.empty()){
        DIR *d = NULL;
        DIR *tempD = NULL;
        struct dirent* dentry = NULL;
        std::vector<std::string> tempNamesList;
        std::string tempFilename;
        std::string tempDirName;
        /*
            In current directory we get the list of images (jpg, png) and puts it to
            namesList string and count them.
            Then we random number between 0 and count of files and set the resultFilename to namesList[randNum]
        */
        if ((d = opendir(path.c_str())) != NULL){
            while ((dentry = readdir(d)) != NULL){
                tempFilename = (std::string)dentry->d_name;
                if (tempFilename != "." && tempFilename != ".."){
                    if (usingSubDirs){
                        tempDirName = path + tempFilename + (std::string)"/";
                        if ((tempD = opendir(tempDirName.c_str())) != NULL){
                            closedir(tempD);
                            tempNamesList = readImagesFromDir(tempDirName);
                            for (std::size_t i = 0; i < tempNamesList.size(); i++)
                                namesList.push_back(tempFilename + (std::string)"/" + tempNamesList[i]);
                            tempNamesList.clear();
                        }
                    }
                    for (std::size_t i = 0; i < imageFormats.size(); i++)
                        if (tempFilename.find((std::string)"." + imageFormats[i], 0) != std::string::npos)
                            if(tempFilename.find((std::string)"." + imageFormats[i], 0) ==
                                    tempFilename.length() - imageFormats[i].length() - 1){
                                namesList.push_back(tempFilename);
                                break;
                            }
                }
            }
            closedir(d);
            free(dentry);
        } else
            std::cerr << errWrongDir << path << std::endl;
    }
    return namesList;
}

bool Wlswitch::isLocked()
{
    std::ifstream fin;
    fin.open(lockFilePath);
    return fin.is_open();
}
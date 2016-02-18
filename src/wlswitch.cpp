#include "../include/wlswitch.h"
#include <cstdlib>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

Wlswitch::Wlswitch(string path, int newDelay)
{
    currentDir = path;
    delay = newDelay;
    loadConfig();
}

Wlswitch::~Wlswitch(){}

void Wlswitch::loadConfig()
{
    fstream fin;
    string word[3];
    int k = 0;

    configLoaded = 0;
    string configPath = (currentDir + (string)"wlswitch.conf");

    fin.open(configPath.c_str(), ios_base::in);
    if(!fin.is_open())
        return;
    configLoaded = 1;

    while(!fin.eof()){

        fin >> word[k];
        k++;
        ///Каждые 3 считанные слова передаем парсеру
        if(3 == k){

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
    string fileName;

    int count = 0;
    fstream fin;

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


    fin.open(countFile.c_str(), ios_base::in);
    if(!fin.is_open())
        return;
    fin >> count;

    fin.close();

    fin.open(listFile.c_str(), ios_base::in);
    if(!fin.is_open())
        return;
    count = rand() % count + 1;

    for(int i = 0; i < count; i++)
        fin >> fileName;

    fin.close();

    s = switcherProgram + " " + switcherArguments + " " + fileName;
    system(s.c_str());


}

void Wlswitch::updateChildConfigs()
{
    loadConfig();
}

void Wlswitch::parseConfig(string* words)
{


    if(words[2].compare(";") == 0){
        ///Path setting
        if(words[0].compare("path") == 0){
            ///Проверка на валидность пути. Валидны только пути начинающиеся и заканчивающиеся на "/"
            if(words[1].at(0) == words[1].at(words[1].length() - 1) && words[1][0] == '/')
                currentDir = words[1];
        }
        else
            ///Wallpaper switcher program setting
            if(words[0].compare("switcher") == 0){
                switcherProgram = words[1];
            }
        else
            ///Delay setting
            if(words[0].compare("delay") == 0){

                delay = words[1];
            }

        else
            ///Switcher program argument setting
            //if(words[0].find("key", 0) != string::npos){
            if(words[0].compare("argument") == 0){
               switcherArguments += words[1] + " ";
            }

        else

            if(words[1].compare("avg") == 0){

                replaceMarker(words[0], avgMarker);
            }
    }
}

size_t Wlswitch::waitDelay()
{
    string s;

    s = "sleep " + delay;
    return system(s.c_str());
}

void Wlswitch::replaceMarker(string oldMarker, string newMarker){}///TODO

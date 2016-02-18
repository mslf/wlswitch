#ifndef WLSWITCH_H_INCLUDED
#define WLSWITCH_H_INCLUDED
#include <string>
using namespace std;

class Wlswitch
{
private:
    string currentDir;
    string avgMarker;
    string switcherProgram;
    string switcherArguments;
    string delay;

    bool configLoaded;


    void loadConfig();
    void parseConfig(string* words);
    void replaceMarker(string oldMarker, string newMarker);

public:


    Wlswitch(string path = "~/Pictures/", string newDelay = "180");
    ~Wlswitch();

    void switchWallpaper();
    void updateChildConfigs();
    size_t waitDelay();

};


#endif // WLSWITCH_H_INCLUDED

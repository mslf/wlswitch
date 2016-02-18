#include <iostream>
#include <cstdlib>
#include <string>
#include "../include/wlswitch.h"
using namespace std;

int main()
{
    Wlswitch wl;

    while(1){

        wl.switchWallpaper();
        if(wl.waitDelay() != 0)
            return 0;

    }

    return 0;
}

##wlswitch
This is a wallpaper switcher with config auto modifying for openbox (or another WM). Can use different programs for switching wallpapers (e.g. nitrogen, feh).
Can compute a lot of characteristics of wallpaper's image. (e.g. avg, invert_avg)
Also it is can change for e.g. conky configs at the markers for color changing.
##config
Loads the default config from ~/.config/wlswitch/wlswitch.conf
See the example config (wlswitch_sample.conf)
##building
Needs to install libmagick++ and libmagick++-dev packages.
(
For Debian:
	$ sudo apt-get install libmagick++-6.q16-5v5 libgraphicsmagick++-q16-12 libmagick++-6.q16-dev libmagick++-dev
)

$ cmake CMakeLists.txt
$ make
##install
Use install.sh for creating wlswitch directory in your configs directory.
##usage
wlswitch depends on Magick++ libary, pleasy install it in your system.
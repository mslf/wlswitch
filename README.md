## wlswitch
This is a wallpaper switcher with config auto modifying for openbox (or another WM). It can use different programs for switching wallpapers. (e.g. nitrogen, feh)
`wlswitch` can compute a lot of characteristics of wallpaper's image. (e.g. mean color, inverted mean color for image's top side e.t.c)
For more information see `doc/WlswitchConfigDoc`.
Also it is can change for e.g. conky configs at the markers for color changing. (see `doc/DependConfigDoc`)
## Config
Loads the default config from `~/.config/wlswitch/wlswitch.conf`
See the example config (`doc/wlswitch_sample.conf`)
## Building
Needs to install `libmagick++` and `libmagick++-dev` packages.

For Debian:
```
$ apt-get install libmagick++-6.q16-5v5 libgraphicsmagick++-q16-12 libmagick++-6.q16-dev libmagick++-dev
```
Exute folowing commands in `wlswitch` download directory:
```
$ mkdir build
$ cd ./build
$ cmake ..
$ make
```

## Usage
`wlswitch` depends on `Magick++` libary, please, install it in your system.
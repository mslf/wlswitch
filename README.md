## wlswitch
This is a wallpaper switcher with config auto modifying for openbox (or another WM). It can use different programs for switching wallpapers (e.g. nitrogen, feh).
`wlswitch` can compute a lot of characteristics of wallpaper's image. (e.g. avg, invert_avg)
Also it is can change for e.g. conky configs at the markers for color changing.
## Config
Loads the default config from `~/.config/wlswitch/wlswitch.conf`
See the example config (`wlswitch_sample.conf`)
## Building
Needs to install `libmagick++` and `libmagick++-dev` packages.

For Debian:
```
$ apt-get install libmagick++-6.q16-5v5 libgraphicsmagick++-q16-12 libmagick++-6.q16-dev libmagick++-dev
```
And make:
```
$ cmake CMakeLists.txt
$ make
```

## Usage
`wlswitch` depends on `Magick++` libary, please, install it in your system.
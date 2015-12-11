HEVCESBrowser
==========

HEVCESBrowser is a tool for analyzing HEVC(h265) bitstreams.


This application displays syntax elements of hevc elementary streams. Input file for HEVCESBrowser should be unmuxed elementary stream. If analysis of the video in any type of container (e.g. in .ts or .mp4 files) is needed, preliminary demuxing is required. For example, you can use ffmpeg for this: ffmpeg -i your_file -vcodec copy -an video.h265. There are gui and cli applications in this product.


Current building status: [![Build Status](https://travis-ci.org/virinext/hevcesbrowser.svg)](https://travis-ci.org/virinext/hevcesbrowser)

Prebuilt binaries
-----

Prebuilt binaries for windows are available.

[Latest Release](https://github.com/virinext/hevcesbrowser/releases/latest)



Gui
-----

![alt tag](https://cloud.githubusercontent.com/assets/10683398/6995983/2f0a3974-db20-11e4-8d8f-cd6db7a954c4.png)


Building requirements:
-----

* boost (for testing only)

* qt



Building:
-----

cd hevcesbrowser

git submodule init && git submodule update

mkdir project

cd project

cmake ../

make 

cd ../utils/hevc_es_browser_gui/

qmake hevc_es_browser_gui.pro

make gitinfo

make


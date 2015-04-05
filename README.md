HEVCESBrowser
==========

HEVCESBrowser is a tool for analyzing HEVC(h265) bitstreams.


This application displays syntax elements of hevc elementary streams. Input file for HEVCESBrowser should be unmuxed elementary stream. If analysis of the video in any type of container (e.g. in .ts or .mp4 files) is needed, preliminary demuxing is required. For example, you can use ffmpeg for this: ffmpeg -i your_file -vcodec copy -an video.h265. There are gui and cli applications in this product.



Gui
-----

![alt tag](https://cloud.githubusercontent.com/assets/10683398/6668595/2699a5c4-cc1e-11e4-95f9-1db911c98dce.png)


Building requirements:
-----

* boost (for testing only)

* qt



Building:
-----

cd hevcesbrowser

git submodule foreach git pull

mkdir project

cd project

cmake ../

make 

cd ../utils/hevc_es_browser_gui/

qmake hevc_es_browser_gui.pro

make gitinfo

make


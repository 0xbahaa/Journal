[![Stories in Ready](https://badge.waffle.io/lordadamson/Journal.png?label=ready&title=Ready)](https://waffle.io/lordadamson/Journal)
# Journal
a note taking application that is heavily inspired by OneNote.

## Setting up development environment
This project is based on the wonderful Qt framework.

If you're on Windows download and install it here: https://www.qt.io/download-open-source/
If you're on Linux use your package manager.
### Ubuntu
`sudo apt install sudo apt-get install qt5-default qttools5-dev-tools` (I don't have ubuntu, let me know if there are more packages the needs to be installed please).
### Fedora
`sudo dnf install qt5-qt*`
### Arch
I don't know! If you have arch send a pull request to let the world know what commands to do!

## opening the project using the Terminal
after making sure that Qt has been downloaded go to the project directory `cd Journal` and type the following commands :
*  `qmake`
*  `make`
*  `./Jounral`
## opening the proeject using [Qt creator](https://www.qt.io/ide/)
I use qtcreator to develop and I recommend using it.

The project uses qmake, so import the project to qtcreator and it should be fine as long as you have a recent version of qt.

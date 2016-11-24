# Image processing apps
A set of QT apps for image processing, based on dense labeling from a very sparse user input.

## Compiling the apps 

The following must be installed into your computer:
* [git] (https://git-scm.com/) for downloading the source code.
* A C++11 capable compiler (gcc, clang...) for your operating system.
* [CMake](https://cmake.org/) for cross-compilation.
* [OpenCV](http://opencv.org/) for image processing.
* [Qt 5] (https://www.qt.io/) for the user interface.
* Optionally, [Boost](http://www.boost.org/), for compiling command line executables for superpixel segmentation and dense labeling.

You can clone the repository and compile everything (git + cmake + g++):
```
git clone https://github.com/anacambra/app_lensblur.git
cd app_lensblur
git branch -u origin/filters filters
git checkout filters 
mkdir build
cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make
```

This will compile all the aps and store the executable files in `app_lensblur/bin`. In order to execute them please you can either set that folder as part of the `PATH` environment variable, or move to the folder and execute there. None of the three  

Using the `build` is recommended in order to avoid poluting the folder with the code with all the compilation files.

Notice that `cmake` has many generators, it is supposed to help cross-plattform compilation. Some of them generate different kinds of makefiles, while some others generate specific IDE projects (such as Visual Studio or XCode projects). Find more information [here](https://cmake.org/cmake/help/v3.0/manual/cmake-generators.7.html).

The parameter `-DCMAKE_BUILD_TYPE=Release` is required for optimization purposes on command-line generators, and can be replaced by `-DCMAKE_BUILD_TYPE=Debug` for debug purposes. IDE generators will include both build types into the corresponding projects. 

There are two lines that are necesary because this is a branch, and not the master of the repository, and might eventually dissappear:
```
git branch -u origin/filters filters
git checkout filters 
``` 


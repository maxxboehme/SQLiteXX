# SQLiteXX
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/maxxboehme/SQLiteXX/blob/master/LICENSE.txt)
[![Travis CI Linux/Mac Build Status](https://travis-ci.org/maxxboehme/SQLiteXX.svg?branch=master)](https://travis-ci.org/maxxboehme/SQLiteXX)
[![AppVeyor Windows Build status](https://ci.appveyor.com/api/projects/status/wkrlgfv2p5mm5cgg/branch/master?svg=true)](https://ci.appveyor.com/project/maxxboehme/sqlitexx/branch/master)
[![Coverage Status](https://coveralls.io/repos/github/maxxboehme/SQLiteXX/badge.svg)](https://coveralls.io/github/maxxboehme/SQLiteXX)

## What is SQLiteXX
An object oriented designed C++ wrapper for [sqlite3](https://www.sqlite.org) that uses features in C++14.

## How to use it
The following links will direct you to helpful documents on how to use SQLiteXX.

* [Tutorial](docs/tutorial.md) - Getting Started
* [Doxygen](https://maxxboehme.github.io/SQLiteXX/doxygen/html) - Documentation for the API
* [Reference](docs/ReadMe.md) - all the details

## How to build it
You will need a compiler that supports C++14. The Travis-CI YAML file shows some of the supported and tested compilers.

Using the following commands should allow you to build SQLiteXX on Windows, Linux, and Mac.
```Shell
git clone https://github.com/maxxboehme/SQLiteXX.git
cd SQLiteXX
mkdir build
cd build

# Generate Visual Studio project, XCode project, Makefiles or any ther build systems CMake supports.
cmake ..

# Build
cmake --build .
```

### Dependencies
* An STL implementation that supports C++14 featurs.
* The SQLite library either by linking statically or dynamically. (The CMake script files will either find the library if there is a version installed on your system or will download and build it during the build process.)
* Catch which is a automated test framework for C++ and is only needed if building the automated tests. (The CMake script files will either find the library if there there is a version installed on your system or will download and build it during the build process.)

## Multithreading
This wrapper does not add any other thead-safety mechanism above what the SQLite library itself provides. You can read what SQLite
already provides in their page, [Using SQLite In Multi-Threaded Applications](http://www.sqlite.org/threadsafe.html).

## More
* Issues and bugs can be raised on the [Issue tracker on Github](https://github.com/maxxboehme/SQLiteXX/issues)


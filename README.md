# Object Tracker
## Description
Some description
## Pre-requirements
First you need to install `conan` with python:
```bash
> sudo python3 -m pip install conan
```
And `opencv` with your package manager

## Installation
You can open project with `CLion` and it automatically would create the `cmake-build-debug` directory.
Or you could create it manually.

Then you should to `cd` into this directory: 
```bash
> cd cmake-build-debug
```
And run(it will download, compile and install all required libraries):
```bash
> conan install .. --build=missing
```
Then you just run:
```bash
> cmake ..
```
Then compile application:
```bash
> cmake --build .
```

## Destination
You could then find the application into `cmake-build-debug` directory with shared library (`*.so` file)
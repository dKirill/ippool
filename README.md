# Intro
Tested on macOS 10.13.2 + XCode9

# Preparation
Do not forget to 
```
git submodule update --init --recursive
```
after clone, since Catch2 unit test framework is included as submodule

# How to build&run tests
## makefile
```
mkdir build-make && cd build-make
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
make
./tests
```

## XCode
```
mkdir build-xcode && cd build-xcode
cmake .. -GXcode
xcodebuild -configuration Release
./Release/tests
```

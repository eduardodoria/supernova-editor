# supernova-editor
Editor of Supernova Engine

## Install GLFW

### Linux (Ubuntu)

`sudo apt install cmake xorg-dev libglfw3 libglfw3-dev`

### macOS

#### Using brew

`brew install glfw`

#### Build from Source (Optional)
```
git clone https://github.com/glfw/glfw.git
cd glfw
cmake .
make
sudo make install
```

### Windows

#### Download GLFW

1. Go to the GLFW website.
2. Download the Windows binaries.
3. Extract Files Extract the downloaded ZIP file to a folder of your choice.

#### Configure Your Project

1. Add the include directory to your compiler's include directories.
2. Link against glfw3.lib found in the lib-vc2022 (or similar) directory.
3. Ensure glfw3.dll is in your executable's directory or in your system's PATH.

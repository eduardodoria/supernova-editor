# supernova-editor
Editor of Supernova Engine

## Install dependencies

### Linux (Ubuntu)

#### GLFW

`sudo apt install cmake xorg-dev libglfw3 libglfw3-dev`

#### SDL

`sudo apt install cmake xorg-dev libsdl2-dev`

#### GTK-3 or Portal

Depending of NFD_PORTAL config.

`sudo apt install libgtk-3-dev`
`sudo apt install libdbus-1-dev`

### macOS

#### GLFW

##### Using brew

`brew install glfw`

##### Build from Source (Optional)
```
git clone https://github.com/glfw/glfw.git
cd glfw
cmake .
make
sudo make install
```

#### SDL

##### Using brew

`brew install sdl2`

##### Build from Source (Optional)
```
git clone https://github.com/libsdl-org/SDL.git
cd SDL
cmake .
make
sudo make install
```

### Windows

#### GLFW

##### From binaries

1. Go to the GLFW website.
2. Download the Windows binaries.
3. Extract Files Extract the downloaded ZIP file to a folder of your choice.
4. Add environment variable "GLFW_DIR" with path of binaries.

#### SDL

##### From binaries

1. Go to the SDL2 download page.
2. Download the precompiled binaries for Windows.
3. Extract the downloaded ZIP file to a folder of your choice.
4. Add an environment variable SDL2_DIR with the path to the binaries.

## Assets

 - https://opengameart.org/content/sky-box-sunny-day
 - https://www.freepik.com/icon/file_2521594
 - https://www.freepik.com/icon/folder_12003793

 ## Third party libraries

* imgui - https://github.com/ocornut/imgui - MIT license
* nativefiledialog-extended (nfd) - https://github.com/btzy/nativefiledialog-extended - Zlib license
* ImGuiColorTextEdit - https://github.com/santaclose/ImGuiColorTextEdit - MIT license
* yaml-cpp - https://github.com/jbeder/yaml-cpp - MIT license

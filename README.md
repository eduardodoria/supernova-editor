# supernova-editor
Editor of Supernova Engine

## Install wxWidgets

### Linux (Ubuntu)

`sudo apt install libwxgtk3.2-dev`

### macOS

#### Using brew

`brew install wxwidgets`

#### Download wxWidgets source
```
mkdir build
cd build
../configure --enable-debug
make
sudo make install
```

### Windows

Download wxWidgets binaries (https://www.wxwidgets.org/downloads):
* Development files
* Header files

Extract both to same directory. Ex: `C:\wxWidgets-<version>`

Create `WXWIN` environment variable with this directory

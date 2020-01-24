# PortAudio Device List as JSON object
## Building/Compiling
Building requires [Docker](https://www.docker.com/) installed.

Simply run the following command with a [dockcross](https://github.com/dockcross/dockcross#cross-compilers) image name :
```bash
./bin/build.sh <dockcross-image-name>
```
When done, the binary will be in the `./build/$CROSS_TRIPLE/` directory.

## Supported Platforms
Currently, the supported/working platforms ([dockcross](https://github.com/dockcross/dockcross#cross-compilers) image names) are :
- linux-x64
- linux-x86
- windows-static-x86
- windows-static-x64

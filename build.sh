~!/bin/bash
set -x

which cmake 1>/dev/null 2>/dev/null || sudo apt install cmake
which cmake 1>/dev/null 2>/dev/null || sudo apt install g++


mkdir build;
cd build;
cmake ..;
make;

cmake -S . -B build/ -G "Visual Studio 17 2022"
cd build
msbuild test_runner.sln
Debug\test_runner.exe


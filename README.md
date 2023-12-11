# TestCmakelist
# This is a demo about how to use cmake to develop in VScode with C++.

1. 安装VSCODE，CMAKE，编译器， googletest, boost
   - googletest: https://github.com/google/googletest
   - boost: https://www.boost.org/
   - VSCODE: cmake插件, c++ 插件等
2. 根据cmakelists修改具体boost库和googletest库的位置
3. mkdir build
4. cd build; cmake ..; cmake --build . --config Debug/Release

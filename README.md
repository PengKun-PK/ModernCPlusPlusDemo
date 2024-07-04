# Modern C++ Demo
# This is a demo about how to use cmake to develop in VScode with C++.
持续更新中

功能:
1. 基础的vscode + cmake 开发c++
2. 增加了懒汉模式的单例模板类
3. 增加了GTest，后续持续增加单元测试保证覆盖率
4. 增加了SPLog日志模块，持续更新
5. 增加了基于boost库的状态机模板类
6. 增加了线程池类
7. 基于c++20，持续优化以上所有模块

开发步骤:
1. 安装VSCODE，CMAKE，编译器， googletest, boost
   - googletest: https://github.com/google/googletest
   - boost: https://www.boost.org/
   - VSCODE: cmake插件, c++ 插件等
2. 根据cmakelists修改具体boost库和googletest库的位置
3. p.s. 最新更新中把googletest作为子库加进来(软链接), git clone --recursive !!!
4. mkdir build
5. cd build; cmake ..; cmake --build . --config Debug/Release

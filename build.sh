#!/bin/bash
# 构建脚本 - 在 Docker 容器内运行
set -e  # 遇到错误立即退出

echo "=== C++ Project Build Script ==="

# 默认构建类型
BUILD_TYPE="Release"

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug|-d)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release|-r)
            BUILD_TYPE="Release"
            shift
            ;;
        --build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --help|-h)
            echo "用法: $0 [选项]"
            echo "选项:"
            echo "  -d, --debug         构建Debug版本"
            echo "  -r, --release       构建Release版本（默认）"
            echo "  --build-type TYPE   指定构建类型（Release/Debug/RelWithDebInfo/MinSizeRel）"
            echo "  -h, --help          显示帮助信息"
            exit 0
            ;;
        *)
            echo "未知参数: $1"
            echo "使用 --help 查看可用选项"
            exit 1
            ;;
    esac
done

echo "构建类型: $BUILD_TYPE"

# 检查是否在正确的目录
if [ ! -f "CMakeLists.txt" ]; then
    echo "错误: 找不到 CMakeLists.txt 文件"
    echo "请确保在项目根目录运行此脚本"
    exit 1
fi

# 清理旧的构建目录（解决路径冲突问题）
echo "清理旧的构建缓存..."
if [ -d "build" ]; then
    rm -rf build
    echo "已删除旧的 build 目录"
fi

# 创建构建目录
echo "创建构建目录..."
mkdir -p build
cd build

# 验证 Boost 安装
echo "验证 Boost 安装..."
echo "BOOST_ROOT: $BOOST_ROOT"
echo "BOOST_LIBRARYDIR: $BOOST_LIBRARYDIR"

# 检查 Boost 头文件
if [ -d "$BOOST_ROOT/include/boost" ]; then
    echo "✅ Boost 头文件找到"
    ls -la $BOOST_ROOT/include/boost/ | head -5
else
    echo "❌ Boost 头文件未找到"
fi

# 检查 Boost 库文件（动态库）
if [ -d "$BOOST_LIBRARYDIR" ]; then
    echo "✅ Boost 库目录找到"
    echo "Boost 动态库："
    ls -la $BOOST_LIBRARYDIR/libboost*.so* 2>/dev/null | head -10 || echo "动态库文件列表获取失败"
else
    echo "❌ Boost 库目录未找到"
fi

# 显示已安装的 Boost 包
echo ""
echo "已安装的 Boost 包："
dpkg -l | grep libboost | head -10

# 验证 OpenCL 安装
echo ""
echo "验证 OpenCL 安装..."
if [ -f "/usr/include/CL/cl.h" ]; then
    echo "✅ OpenCL 头文件找到"
else
    echo "❌ OpenCL 头文件未找到"
fi

# 显示已安装的 OpenCL 包
echo "已安装的 OpenCL 包："
dpkg -l | grep -i opencl

# 检查 OpenCL 运行时
echo ""
echo "检查 OpenCL 设备..."
which clinfo >/dev/null 2>&1 && clinfo 2>/dev/null | grep "Number of platforms" || echo "⚠️  未检测到 OpenCL 设备（这在容器中是正常的）"

# 配置 CMake（使用动态构建类型）
echo ""
echo "配置 CMake (构建类型: $BUILD_TYPE)..."

# 根据构建类型设置不同的编译器标志
CMAKE_EXTRA_FLAGS=""
if [ "$BUILD_TYPE" = "Debug" ]; then
    CMAKE_EXTRA_FLAGS="-DCMAKE_CXX_FLAGS_DEBUG=\"-g -O0 -Wall -Wextra -DDEBUG\""
elif [ "$BUILD_TYPE" = "Release" ]; then
    CMAKE_EXTRA_FLAGS="-DCMAKE_CXX_FLAGS_RELEASE=\"-O3 -DNDEBUG\""
fi

eval cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBOOST_ROOT="${BOOST_ROOT}" \
    -DBOOST_LIBRARYDIR="${BOOST_LIBRARYDIR}" \
    -DBoost_DEBUG=ON \
    -DBoost_DETAILED_FAILURE_MSG=ON \
    -DUSE_OPENCL=ON \
    $CMAKE_EXTRA_FLAGS || {
        echo "❌ CMake 配置失败"
        echo "查看 CMake 错误日志："
        [ -f "CMakeFiles/CMakeError.log" ] && tail -50 CMakeFiles/CMakeError.log
        exit 1
    }

# 构建项目
echo ""
echo "构建项目 ($BUILD_TYPE)..."
cmake --build . --config "$BUILD_TYPE" -j$(nproc) || {
    echo "❌ 构建失败"
    exit 1
}

echo ""
echo "=== 构建完成 ($BUILD_TYPE) ==="

# 检查可执行文件
if [ -f "ModernCPlusPlusDemo" ]; then
    echo "✅ 主程序构建成功: ModernCPlusPlusDemo"
    ls -lah ModernCPlusPlusDemo

    # 显示二进制文件信息
    echo "文件信息:"
    file ModernCPlusPlusDemo

    # 如果是Debug版本，显示调试信息
    if [ "$BUILD_TYPE" = "Debug" ]; then
        echo "包含调试符号: $(objdump -h ModernCPlusPlusDemo | grep -c debug || echo "未知")"
    fi
else
    echo "❌ 主程序构建失败"
fi

if [ -f "gtest_unitTest" ]; then
    echo "✅ 测试程序构建成功: gtest_unitTest"
    ls -lah gtest_unitTest
else
    echo "❌ 测试程序构建失败"
fi

# 检查子项目库
echo ""
echo "检查子项目库:"
for lib in StateCharts SPDLogEx StateMachine DesignModes ThreadPool Subscriber OpenCL; do
    if [ -f "${lib}/lib${lib}.a" ] || [ -f "${lib}/lib${lib}.so" ]; then
        echo "✅ ${lib} 库构建成功"
    else
        echo "⚠️  ${lib} 库未找到（可能是直接链接到主程序）"
    fi
done

echo ""
echo "可用命令:"
echo "  ./ModernCPlusPlusDemo     # 运行主程序"
echo "  ./gtest_unitTest          # 运行单元测试"

if [ "$BUILD_TYPE" = "Debug" ]; then
    echo "  gdb ./ModernCPlusPlusDemo # 使用GDB调试主程序"
    echo "  valgrind --tool=memcheck --leak-check=full ./ModernCPlusPlusDemo  # 内存检查"
fi

echo ""
echo "注意: 在 Docker 容器中可能无法访问真实的 GPU/OpenCL 设备"
echo "当前构建类型: $BUILD_TYPE"

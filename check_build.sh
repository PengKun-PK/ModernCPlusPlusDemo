#!/bin/bash
echo "=== 检查当前构建配置 ==="

# 检查当前目录
if [ -f "CMakeCache.txt" ]; then
    echo "✅ 在构建目录中"
elif [ -f "build/CMakeCache.txt" ]; then
    echo "切换到构建目录..."
    cd build
else
    echo "❌ 未找到构建目录，请先运行构建脚本"
    exit 1
fi

echo ""
echo "1. CMake 构建类型配置:"
grep -E "CMAKE_BUILD_TYPE|CMAKE_CONFIGURATION_TYPES" CMakeCache.txt || echo "未找到构建类型配置"

echo ""
echo "2. 环境变量:"
echo "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE:-未设置}"
echo "BUILD_TYPE: ${BUILD_TYPE:-未设置}"

echo ""
echo "3. 生成的目录结构:"
ls -la | grep -E "Debug|Release|RelWithDebInfo|MinSizeRel" || echo "未找到构建类型相关目录"

echo ""
echo "4. 可执行文件编译信息:"
if [ -f "ModernCPlusPlusDemo" ]; then
    echo "主程序:"
    file ModernCPlusPlusDemo

    # 检查是否包含调试符号
    if objdump -h ModernCPlusPlusDemo 2>/dev/null | grep -q debug; then
        echo "✅ 包含调试符号 (Debug 构建)"
    else
        echo "❌ 不包含调试符号 (Release 构建)"
    fi

    # 检查优化级别（如果可能）
    echo "二进制文件大小: $(ls -lh ModernCPlusPlusDemo | awk '{print $5}')"
else
    echo "未找到可执行文件"
fi

echo ""
echo "5. CMake 生成器信息:"
grep CMAKE_GENERATOR CMakeCache.txt || echo "未找到生成器信息"

echo ""
echo "6. 编译器标志:"
grep -E "CMAKE_CXX_FLAGS|CMAKE_C_FLAGS" CMakeCache.txt | head -5

# 尝试使用 Ubuntu 24.10 (代号 oracular)
FROM ubuntu:oracular

# 设置非交互式安装，避免安装过程中的提示
ENV DEBIAN_FRONTEND=noninteractive

# 更新包列表并安装必要的工具和依赖
RUN apt-get update && apt-get install -y \
    # 基础开发工具
    build-essential \
    git \
    wget \
    curl \
    pkg-config \
    sudo \
    ca-certificates \
    gnupg \
    lsb-release \
    unzip \
    # C++编译器和工具
    g++ \
    gdb \
    # CMake
    cmake \
    # OpenCL 依赖（与 CI 保持一致）
    opencl-headers \
    ocl-icd-opencl-dev \
    pocl-opencl-icd \
    # Boost 库（与 CI 保持一致，使用 apt 安装）
    libboost-all-dev \
    # 清理缓存
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 设置 Boost 环境变量（与 CI 保持一致）
ENV BOOST_ROOT=/usr
ENV BOOST_LIBRARYDIR=/usr/lib/x86_64-linux-gnu
ENV LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH

# 验证安装的版本
RUN echo "===== 系统信息 =====" && \
    lsb_release -a && \
    echo "\n===== CMake 版本 =====" && \
    cmake --version && \
    echo "\n===== GCC 版本 =====" && \
    g++ --version | head -1 && \
    echo "\n===== Boost 版本 =====" && \
    dpkg -l | grep libboost-all-dev && \
    echo "\n===== OpenCL 包 =====" && \
    dpkg -l | grep -i opencl

# 创建工作目录
RUN mkdir -p /workspace

# 设置工作目录
WORKDIR /workspace

# 设置默认命令
CMD ["/bin/bash"]

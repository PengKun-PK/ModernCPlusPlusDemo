# Modern C++ Demo

![C++](https://img.shields.io/badge/C++-20-blue.svg)
![CMake](https://img.shields.io/badge/CMake-3.10+-green.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)

📚 [View Full Documentation](https://pengkun-pk.github.io/ModernCPlusPlusDemo/)

## 目录

- [项目简介](#项目简介)
- [主要特性](#-主要特性)
- [技术栈](#️-技术栈)
- [快速开始](#-快速开始)
  - [前置要求](#前置要求)
  - [安装步骤](#安装步骤)
- [使用指南](#-使用指南)
- [运行测试](#-运行测试)
- [持续更新计划](#-持续更新计划)
- [贡献](#-贡献)
- [许可证](#-许可证)
- [联系方式](#-联系方式)

## 项目简介

Modern C++ Demo 是一个展示现代C++开发实践的示例项目。本项目旨在演示如何在VSCode中使用CMake进行C++开发,并整合了多种先进的C++特性和常用库。无论您是C++新手还是经验丰富的开发者,这个项目都能为您提供宝贵的学习资源和参考。

## 🌟 主要特性

- 基于VSCode和CMake的现代C++开发环境
- 单例模式模板类实现(线程安全）
- event-bus事件总线模式模板类实现及demo
- 观察者模式模板类实现
- 工厂模式(CRTP派生于单例)模板类实现
- 集成Google Test框架,支持单元测试
- 使用SPDLog实现高效日志记录
- 基于Boost库的状态机实现
- 自定义线程池模板类实现
- 自用事件，属性订阅模式模板类实现
- 充分利用C++20新特性
- 持续优化和更新的各个模块

## 🛠️ 技术栈

- C++20
- CMake 3.10+
- Google Test
- Boost库
- SPDLog
- VSCode
  
## 🚀 快速开始

### 前置要求

确保您的系统已安装以下工具:

- VSCode
- CMake (3.10+)
- 现代C++编译器 (支持C++20)
- Git
- boost (1.83.0)

### 安装步骤

1. 克隆仓库:
git clone --recursive https://github.com/PengKun-PK/ModernCPlusPlusDemo.git

2. 根据.vscode目录下文件开始编译

3. task.json, launch.json, setting.json均已配置好，有问题，可查看json文件里路径是否正确，或者cmakelist里路径是否正确

## 📘 使用指南

本项目包含多个模块,每个模块都有其特定的用途和示例。以下是一些主要模块的简要说明:

- **单例模式**: `DesignModes/Singleton.hpp` 展示了线程安全的懒汉式单例模式实现。
- **工厂模式**: `DesignModes/ObjectFactory.hpp` 展示了工厂模式模板类实现
- **事件总线模式**: `DesignModes/EventBus.hpp` 展示了事件总线模式实现
- **观察者模式**: `DesignModes/Observer.hpp` 展示了观察者模式实现
- **日志系统**: `SPDLogEx/` 目录下包含了SPDLog的集成和使用示例。
- **状态机**: `StateMachine/` 展示了如何使用Boost库实现状态机。
- **线程池**: `ThreadPool/` 提供了一个高效的线程池实现。
- **自定义订阅模式**: `Subscriber/` 展示了我自用的订阅模式实现

更多详细信息,请参阅各模块的文档和源代码。

## 🧪 运行测试

本项目使用Google Test进行单元测试。要运行测试,请在构建目录中执行(Run Google Tests)，目前只加了个别模块的单元测试，后续会慢慢更新

## 📈 持续更新计划

我们正在持续改进和扩展这个项目。以下是一些计划中的更新:

- 增加更多设计模式示例
- 改进线程池性能
- 添加并发编程示例
- 集成更多现代C++库(C++23/26)

## 🤝 贡献

我们欢迎任何形式的贡献!如果您有任何改进意见或新功能建议,请随时提出issue或pull request。

## 📄 许可证

本项目采用MIT许可证。详情请见 [LICENSE](LICENSE) 文件。

## 📮 联系方式

如有任何问题或建议,请通过GitHub Issues与我们联系。

---

感谢您对Modern C++ Demo的关注。让我们一起探索现代C++的魅力!

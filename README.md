# <a id="chinese-version"></a>面向对象编程（C++）大作业

[English Version](#english-version)

本仓库是东北大学机器人科学与工程学院（**Faculty of Robot Science and Engineering, Northeastern University**）《面向对象编程（C++）》课程的大作业代码仓库。

## 开发环境

- 操作系统：Ubuntu 22.04
- 编程语言：C++
- 编译器：`g++`

## 项目内容

仓库包含以下几个 C++ 课程项目：

- **Blackjack Game**：一个简单的 Blackjack（二十一点）游戏。
- **Currency Exchange**：一个简单的货币兑换程序。
- **Employee Management System**：一个简单的员工管理系统。
- **Unsigned Big Integer Calculator**：一个无符号大整数计算器。

各项目目录中包含对应的源代码和项目说明文档，`screenshots` 目录保存部分运行截图。

## 使用方法

进入需要运行的项目目录，使用 `g++` 编译源文件。例如：

```bash
cd Blackjack_Game
g++ -std=c++17 -Wall -Wextra blackjack.cpp -o blackjack
./blackjack
```

其他项目可以按照相同方式编译运行，只需将源文件名和生成的可执行文件名替换为对应项目中的名称：

```bash
cd Currency_Exchange
g++ -std=c++17 -Wall -Wextra currency_exchange.cpp -o currency_exchange
./currency_exchange
```

```bash
cd Employee_Management_System
g++ -std=c++17 -Wall -Wextra employee_system.cpp -o employee_system
./employee_system
```

```bash
cd Unsigned_Big_Integer_Calculator
g++ -std=c++17 -Wall -Wextra calculator.cpp -o calculator
./calculator
```

编译生成的可执行文件属于临时编译产物，已通过 `.gitignore` 配置为不纳入 Git 版本管理。

---

# <a id="english-version"></a>Object-Oriented Programming (C++) Course Project

[中文版](#chinese-version)

This repository contains the course projects for **Object-Oriented Programming (C++)** at the **Faculty of Robot Science and Engineering, Northeastern University**.

## Development Environment

- Operating system: Ubuntu 22.04
- Programming language: C++
- Compiler: `g++`

## Projects

This repository contains the following C++ course projects:

- **Blackjack Game**: A simple Blackjack game.
- **Currency Exchange**: A simple currency exchange program.
- **Employee Management System**: A simple employee management system.
- **Unsigned Big Integer Calculator**: A calculator for unsigned big integers.

Each project directory contains its source code and project documentation. The `screenshots` directory contains selected screenshots of the programs in use.

## Usage

Enter the directory of the project you want to run and compile its source file with `g++`. For example:

```bash
cd Blackjack_Game
g++ -std=c++17 -Wall -Wextra blackjack.cpp -o blackjack
./blackjack
```

The other projects can be compiled and run in the same way by replacing the source file and executable names:

```bash
cd Currency_Exchange
g++ -std=c++17 -Wall -Wextra currency_exchange.cpp -o currency_exchange
./currency_exchange
```

```bash
cd Employee_Management_System
g++ -std=c++17 -Wall -Wextra employee_system.cpp -o employee_system
./employee_system
```

```bash
cd Unsigned_Big_Integer_Calculator
g++ -std=c++17 -Wall -Wextra calculator.cpp -o calculator
./calculator
```

Compiled executables are temporary build artifacts and are excluded from Git tracking through `.gitignore`.

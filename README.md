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

以下四组命令分别从仓库根目录执行（每组运行后回到根目录再执行下一组）。使用 `g++` 编译源文件。例如：

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

Run each command block separately from the repository root. Return to the root before trying another project. For example:

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


## 审查与测试

- [综合实验报告](./report.md)
- [课程要求逐项核对、修复依据及验证限制](./REVIEW.md)
- 离线回归：在根目录运行 `python3 tests/run_tests.py`（需要 Python 3 与 g++）。
- 外币选择币种后，选择自动或手动汇率；自动失败依次尝试 24 小时内缓存、手动输入。过期缓存直接拒绝，不询问。
- 21 点按课程特殊规则处理双方爆牌平局。

## Visual Studio / MSVC（C++17）

课程要求以 VS 为主。以下命令在 VS 的“Developer Command Prompt”中从仓库根目录执行；源码保存为 UTF-8。外币运行另需命令行 `curl`。本次仅实测 Linux/g++，下列 MSVC 命令未实机验证。

```bat
cl /std:c++17 /EHsc /utf-8 /W4 Blackjack_Game\blackjack.cpp /Fe:Blackjack_Game\blackjack.exe
cl /std:c++17 /EHsc /utf-8 /W4 Currency_Exchange\currency_exchange.cpp /Fe:Currency_Exchange\currency_exchange.exe
cl /std:c++17 /EHsc /utf-8 /W4 Employee_Management_System\employee_system.cpp /Fe:Employee_Management_System\employee_system.exe
cl /std:c++17 /EHsc /utf-8 /W4 Unsigned_Big_Integer_Calculator\calculator.cpp /Fe:Unsigned_Big_Integer_Calculator\calculator.exe
```

在 VS 图形界面中，每个程序建立单独控制台项目，加入对应 `.cpp`，设置 C++17 和 `/utf-8`；四个文件各有 `main`，不可放进同一个可执行项目。Windows 终端可执行 `chcp 65001` 使用 UTF-8。

# YSU-Compiler-Principles-Experiment-Course
本仓库是燕山大学（YSU）编译原理课程的实验配套代码与文档，涵盖编译原理核心实验环节（词法分析、语法分析、语义分析、中间代码生成、代码优化、目标代码生成等），旨在帮助学习者理解编译程序的构造原理与实现方法。

## 项目介绍
编译原理是计算机专业核心课程，本项目以**可运行的代码+实验指导**的形式，拆解编译程序的各个核心阶段，从基础的词法分析器实现，到完整的小型编译器构建，逐步引导完成编译原理课程实验。

### 实验涵盖内容
| 实验阶段 | 核心内容 | 技术要点 |
|----------|----------|----------|
| 词法分析 | 设计实现词法分析器，识别关键字、标识符、常量、运算符等 | 正则表达式、有限自动机、Flex（可选） |
| 语法分析 | 实现自上而下/自下而上的语法分析器（LL(1)/SLR(1)等） | 上下文无关文法、语法分析表、递归下降分析 |
| 语义分析 | 类型检查、符号表管理、语义规则实现 | 符号表设计、属性文法、语义动作 |
| 中间代码生成 | 生成三地址码/四元式等中间表示 | 中间代码结构、语法制导翻译 |
| 代码优化（可选） | 基本块优化、常量传播、删除公共子表达式 | 数据流分析、优化算法 |
| 目标代码生成（可选） | 生成简单机器码/汇编代码 | 寄存器分配、目标指令映射 |

## 环境要求
- 操作系统：Windows/Linux/macOS（推荐Linux/macOS，编译工具链更友好）
- 编译工具：GCC/G++（C/C++版本≥11）、Make（可选）
- 辅助工具（可选）：Flex（词法分析生成器）、Bison（语法分析生成器）
- 开发环境：VSCode、CLion、Vim等（任选）

### 环境安装（Linux/macOS）
```bash
# 安装GCC/Make
sudo apt update && sudo apt install gcc g++ make -y  # Ubuntu/Debian
# brew install gcc make  # macOS（需先安装Homebrew）

# 安装Flex/Bison（可选）
sudo apt install flex bison -y  # Ubuntu/Debian
# brew install flex bison  # macOS
```

### 环境安装（Windows）
- 安装MinGW-w64（提供GCC编译环境），配置环境变量
- 或使用WSL2（推荐），在Windows子系统中安装Linux环境
- Flex/Bison可通过MinGW-w64安装，或使用WinFlexBison

## 快速开始
### 1. 克隆仓库
```bash
git clone https://github.com/asgudao/YSU-Compiler-Principles-Experiment-Course.git
cd YSU-Compiler-Principles-Experiment-Course
```

### 2. 编译运行（以词法分析实验为例）
```bash
# 进入对应实验目录
cd src/lexical_analysis
# 编译代码
make  # 若有Makefile，直接执行；无则执行：gcc -o lexer lexer.c
# 运行程序（输入测试用例）
./lexer test.c
```

## 目录结构
```
YSU-Compiler-Principles-Experiment-Course/
├── doc/                # 实验文档（实验指导书、报告模板、文法说明等）
│   ├── exp1_lexical_analysis.md  # 词法分析实验指导
│   ├── exp2_syntax_analysis.md   # 语法分析实验指导
│   └── report_template.md        # 实验报告模板
├── src/                # 实验源代码
│   ├── lexical_analysis/  # 词法分析实验代码
│   │   ├── lexer.c        # 手动实现的词法分析器
│   │   ├── lexer.l        # Flex生成词法分析器的规则文件
│   │   └── Makefile       # 编译脚本
│   ├── syntax_analysis/   # 语法分析实验代码
│   │   ├── parser.c       # 递归下降分析器
│   │   ├── parser.y       # Bison生成语法分析器的规则文件
│   │   └── test_case.txt  # 测试用例
│   ├── semantic_analysis/ # 语义分析实验代码
│   └── utils/             # 通用工具（符号表、错误处理等）
├── test/               # 测试用例集合
│   ├── test_lex.txt    # 词法分析测试用例
│   ├── test_syntax.txt # 语法分析测试用例
│   └── test_semantic.txt # 语义分析测试用例
└── README.md           # 项目说明（本文档）
```

## 实验完成指南
1. 阅读 `doc/` 下对应实验的指导文档，理解实验目标与实现思路；
2. 参考 `src/` 下的基础代码，完成核心功能的编写/修改；
3. 使用 `test/` 下的测试用例验证程序正确性；
4. 按照 `doc/report_template.md` 完成实验报告。

### 示例：词法分析器测试
输入测试用例（`test/test_lex.txt`）：
```c
int main() {
    int a = 10 + 20;
}
```
运行词法分析器后，预期输出（示例）：
```
关键字: int
标识符: main
界符: (
界符: )
界符: {
关键字: int
标识符: a
运算符: =
常量: 10
运算符: +
常量: 20
界符: ;
界符: }
```

## 常见问题
1. **Flex/Bison编译报错**：检查Flex/Bison版本是否兼容，确保生成的C文件与代码中函数调用匹配；
2. **语法分析器逻辑错误**：先验证文法的LL(1)/SLR(1)特性，检查分析表是否正确；
3. **符号表管理异常**：确认符号表的插入/查询/删除逻辑，避免重复定义、未定义标识符等错误。

## 贡献说明
本仓库为燕山大学编译原理课程实验的参考实现，欢迎：
- 提交Issue反馈代码Bug或实验文档问题；
- 提交PR完善代码注释、补充测试用例、优化实验文档；
- 新增不同实现方式的代码（如纯Python实现的编译器、LLVM后端等）。

## 免责声明
本项目仅用于燕山大学编译原理课程学习与实验参考，请勿直接抄袭代码完成实验报告，建议在理解原理的基础上独立实现。

## 许可证
本项目采用 [MIT License](https://opensource.org/licenses/MIT) 开源许可证，可自由学习、修改、分发。

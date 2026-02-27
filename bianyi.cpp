#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>
#include <cctype>

using namespace std;

// 错误信息结构
struct Error {
    int line;
    char type;
    string description;
    
    Error(int l, char t, const string& d) : line(l), type(t), description(d) {}
    
    bool operator<(const Error& other) const {
        return line < other.line;
    }
};

// 符号表项
struct Symbol {
    string name;
    string type;
    bool isConst;
    bool isFunction;
    int paramCount;
    int lineDeclared;
    
    Symbol() : isConst(false), isFunction(false), paramCount(0), lineDeclared(0) {}
};

class ErrorHandlerFixed {
private:
    vector<Error> errors;
    map<string, Symbol> symbolTable;
    set<string> keywords;
    int currentLine;
    
public:
    ErrorHandlerFixed() {
        currentLine = 1;
        initKeywords();
    }
    
    void initKeywords() {
        keywords = {"int", "char", "void", "const", "if", "else", "while", 
                   "for", "switch", "case", "default", "return", "printf", "scanf"};
    }
    
    void processFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "无法打开文件: " << filename << endl;
            return;
        }
        
        string line;
        vector<string> lines;
        
        // 读取所有行
        while (getline(file, line)) {
            lines.push_back(line);
            currentLine++;
        }
        
        file.close();
        currentLine = 1;
        
        // 第一遍：建立符号表
        for (const auto& line : lines) {
            buildSymbolTable(line);
            currentLine++;
        }
        
        currentLine = 1;
        
        // 第二遍：检查错误
        for (const auto& line : lines) {
            checkErrors(line);
            currentLine++;
        }
        
        // 排序并输出错误
        sort(errors.begin(), errors.end());
        outputErrors();
    }
    
    void buildSymbolTable(const string& line) {
        string trimmed = trim(line);
        if (trimmed.empty()) return;
        
        // 检查函数定义
        if (isFunctionDefinition(trimmed)) {
            processFunctionDefinition(trimmed);
        }
        // 检查变量声明
        else if (isVariableDeclaration(trimmed)) {
            processVariableDeclaration(trimmed);
        }
    }
    
    void checkErrors(const string& line) {
        string trimmed = trim(line);
        if (trimmed.empty()) return;
        
        checkIllegalSymbols(trimmed);
        checkMissingSemicolon(trimmed);
        checkMissingParentheses(trimmed);
        checkMissingBrackets(trimmed);
        checkConstModification(trimmed);
        checkArrayIndex(trimmed);
        checkConditionTypes(trimmed);
        checkFunctionCalls(trimmed);
        checkReturnStatements(trimmed);
        checkSwitchStatements(trimmed);
        checkArrayInitialization(trimmed);
        checkConstTypeMismatch(trimmed);
    }
    
    void checkIllegalSymbols(const string& line) {
        // 检查字符常量
        size_t pos = 0;
        while ((pos = line.find("'", pos)) != string::npos) {
            size_t endPos = line.find("'", pos + 1);
            if (endPos != string::npos) {
                string content = line.substr(pos + 1, endPos - pos - 1);
                if (content.empty()) {
                    addError(currentLine, 'a', "空字符常量");
                    break;
                }
                if (content.length() > 1 && content[0] != '\\') {
                    addError(currentLine, 'a', "字符常量中包含多个字符");
                    break;
                }
            }
            pos = endPos + 1;
        }
        
        // 检查字符串常量
        pos = 0;
        while ((pos = line.find("\"", pos)) != string::npos) {
            size_t endPos = line.find("\"", pos + 1);
            if (endPos != string::npos) {
                string content = line.substr(pos + 1, endPos - pos - 1);
                if (content.empty()) {
                    addError(currentLine, 'a', "空字符串常量");
                    break;
                }
            }
            pos = endPos + 1;
        }
    }
    
    void checkMissingSemicolon(const string& line) {
        // 检查return语句
        if (line.find("return") != string::npos && line.find(";") == string::npos) {
            if (line.find("(") != string::npos && line.find(")") != string::npos) {
                addError(currentLine, 'k', "return语句缺少分号");
            }
        }
        
        // 检查变量声明和赋值
        bool isDecl = (line.find("int") == 0 || line.find("char") == 0);
        bool hasAssign = (line.find("=") != string::npos);
        bool hasSemi = (line.find(";") != string::npos);
        
        if ((isDecl || hasAssign) && !hasSemi && 
            line.find("(") == string::npos && line.find(")") == string::npos) {
            addError(currentLine, 'k', "语句缺少分号");
        }
    }
    
    void checkMissingParentheses(const string& line) {
        int openCount = count(line.begin(), line.end(), '(');
        int closeCount = count(line.begin(), line.end(), ')');
        
        if (openCount > closeCount) {
            addError(currentLine, 'l', "缺少右小括号");
        }
    }
    
    void checkMissingBrackets(const string& line) {
        int openCount = count(line.begin(), line.end(), '[');
        int closeCount = count(line.begin(), line.end(), ']');
        
        if (openCount > closeCount) {
            addError(currentLine, 'm', "缺少右中括号");
        }
    }
    
    void checkConstModification(const string& line) {
        // 更精确地检查常量修改
        for (const auto& entry : symbolTable) {
            if (entry.second.isConst) {
                string name = entry.first;
                
                // 检查是否是赋值语句（不是声明，不是比较）
                size_t namePos = line.find(name);
                if (namePos != string::npos) {
                    // 检查前面是否是声明
                    bool isDeclaration = false;
                    string beforeName = line.substr(0, namePos);
                    if (beforeName.find("const") != string::npos || 
                        beforeName.find("int") != string::npos || 
                        beforeName.find("char") != string::npos) {
                        isDeclaration = true;
                    }
                    
                    if (!isDeclaration) {
                        // 检查后面是否是赋值
                        string afterName = line.substr(namePos + name.length());
                        size_t eqPos = afterName.find("=");
                        if (eqPos != string::npos) {
                            // 确保不是比较操作符
                            if (eqPos == 0 || (eqPos > 0 && afterName[eqPos-1] != '=' && afterName[eqPos-1] != '!')) {
                                // 检查是否是函数调用参数
                                bool isFunctionParam = false;
                                if (line.find("(") != string::npos && line.find(")") != string::npos) {
                                    // 简单检查：如果在括号内，可能是函数参数
                                    size_t openParen = line.find("(");
                                    size_t closeParen = line.find(")");
                                    if (namePos > openParen && namePos < closeParen) {
                                        isFunctionParam = true;
                                    }
                                }
                                
                                if (!isFunctionParam) {
                                    addError(currentLine, 'j', "试图修改常量: " + name);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    void checkArrayIndex(const string& line) {
        size_t pos = 0;
        while ((pos = line.find("[", pos)) != string::npos) {
            size_t endPos = line.find("]", pos);
            if (endPos != string::npos) {
                string index = line.substr(pos + 1, endPos - pos - 1);
                index = trim(index);
                
                // 检查是否是字符常量
                if (index.length() >= 2 && index[0] == '\'' && index[index.length()-1] == '\'') {
                    addError(currentLine, 'i', "数组下标为字符类型");
                }
            }
            pos = endPos + 1;
        }
    }
    
    void checkConditionTypes(const string& line) {
        // 检查==和!=表达式
        size_t eqPos = line.find("==");
        if (eqPos == string::npos) eqPos = line.find("!=");
        
        if (eqPos != string::npos) {
            string left = line.substr(0, eqPos);
            string right = line.substr(eqPos + 2);
            
            bool leftHasChar = (left.find("'") != string::npos);
            bool rightHasChar = (right.find("'") != string::npos);
            bool leftHasInt = (left.find_first_of("0123456789") != string::npos);
            bool rightHasInt = (right.find_first_of("0123456789") != string::npos);
            
            if ((leftHasChar && rightHasInt) || (rightHasChar && leftHasInt)) {
                addError(currentLine, 'f', "条件判断中字符类型与整型比较");
            }
        }
    }
    
    void checkFunctionCalls(const string& line) {
        size_t parenPos = line.find("(");
        if (parenPos != string::npos) {
            string beforeParen = line.substr(0, parenPos);
            
            // 提取函数名
            size_t spacePos = beforeParen.find_last_of(" \t");
            string funcName;
            if (spacePos != string::npos) {
                funcName = beforeParen.substr(spacePos + 1);
            } else {
                funcName = beforeParen;
            }
            
            if (!funcName.empty() && keywords.find(funcName) == keywords.end()) {
                // 检查函数是否定义
                if (symbolTable.find(funcName) == symbolTable.end()) {
                    addError(currentLine, 'c', "未定义的函数: " + funcName);
                } else {
                    // 检查参数个数
                    Symbol func = symbolTable[funcName];
                    int actualParams = countParameters(line);
                    if (actualParams != func.paramCount) {
                        addError(currentLine, 'd', "函数参数个数不匹配: " + funcName);
                    }
                }
            }
        }
    }
    
    void checkReturnStatements(const string& line) {
        if (line.find("return") != string::npos) {
            // 查找当前函数类型
            string funcType = "void"; // 默认
            for (const auto& entry : symbolTable) {
                if (entry.second.lineDeclared <= currentLine && 
                    entry.second.isFunction) {
                    funcType = entry.second.type;
                    break;
                }
            }
            
            if (funcType == "void") {
                // 无返回值函数不应有返回值
                if (line.find("return(") != string::npos || 
                    (line.find("return") != string::npos && line.find(";") != string::npos && 
                     line.find("(") != string::npos)) {
                    addError(currentLine, 'g', "无返回值函数不应有返回值");
                }
            } else {
                // 有返回值函数应有返回值
                if (line.find("return;") != string::npos) {
                    addError(currentLine, 'h', "有返回值函数缺少返回值");
                }
            }
        }
    }
    
    void checkSwitchStatements(const string& line) {
        if (line.find("switch") != string::npos && line.find("default") == string::npos) {
            addError(currentLine, 'p', "switch语句缺少default分支");
        }
    }
    
    void checkArrayInitialization(const string& line) {
        if (line.find("{") != string::npos && line.find("[") != string::npos) {
            addError(currentLine, 'n', "数组初始化不匹配");
        }
    }
    
    void checkConstTypeMismatch(const string& line) {
        // 检查int类型初始化为字符常量
        if (line.find("int") != string::npos && line.find("'") != string::npos) {
            size_t eqPos = line.find("=");
            if (eqPos != string::npos) {
                string init = line.substr(eqPos + 1);
                if (init.find("'") != string::npos && init.find("\\") == string::npos) {
                    addError(currentLine, 'o', "int类型初始化为字符常量");
                }
            }
        }
        
        // 检查switch语句中的case常量
        if (line.find("case") != string::npos && line.find("'") != string::npos) {
            addError(currentLine, 'o', "case常量类型不匹配");
        }
    }
    
    bool isFunctionDefinition(const string& line) {
        return ((line.find("int") == 0 || line.find("char") == 0 || line.find("void") == 0) &&
                line.find("(") != string::npos && line.find(")") != string::npos &&
                line.find("{") != string::npos);
    }
    
    bool isVariableDeclaration(const string& line) {
        return ((line.find("int") == 0 || line.find("char") == 0 || 
                 line.find("const int") == 0 || line.find("const char") == 0) &&
                line.find("(") == string::npos);
    }
    
    void processFunctionDefinition(const string& line) {
        // 提取函数信息
        istringstream iss(line);
        string type, name;
        
        iss >> type;
        if (type == "const") iss >> type;
        
        size_t parenPos = line.find("(");
        if (parenPos != string::npos) {
            string beforeParen = line.substr(0, parenPos);
            istringstream nameStream(beforeParen);
            string temp;
            while (nameStream >> temp) {
                name = temp;
            }
            
            if (!name.empty()) {
                // 计算参数个数
                int paramCount = 0;
                string params = line.substr(line.find("(") + 1, line.find(")") - line.find("(") - 1);
                istringstream paramStream(params);
                string param;
                
                while (paramStream >> param) {
                    if (param == "int" || param == "char") {
                        paramCount++;
                    }
                }
                
                Symbol func;
                func.name = name;
                func.type = type;
                func.isFunction = true;
                func.paramCount = paramCount;
                func.lineDeclared = currentLine;
                
                // 检查重定义
                if (symbolTable.find(name) != symbolTable.end()) {
                    addError(currentLine, 'b', "函数重定义: " + name);
                } else {
                    symbolTable[name] = func;
                }
            }
        }
    }
    
    void processVariableDeclaration(const string& line) {
        // 提取变量信息
        istringstream iss(line);
        string type, name;
        bool isConst = false;
        
        string first;
        iss >> first;
        
        if (first == "const") {
            isConst = true;
            iss >> type;
        } else {
            type = first;
        }
        
        // 处理多个变量
        string rest;
        getline(iss, rest, ';');
        istringstream varStream(rest);
        string varDecl;
        
        while (getline(varStream, varDecl, ',')) {
            istringstream nameStream(varDecl);
            string tempName;
            
            while (nameStream >> tempName) {
                if (tempName != "=" && tempName != type && tempName != "const" && !tempName.empty()) {
                    name = tempName;
                    
                    // 检查重定义
                    if (symbolTable.find(name) != symbolTable.end()) {
                        addError(currentLine, 'b', "变量重定义: " + name);
                    } else {
                        Symbol var;
                        var.name = name;
                        var.type = type;
                        var.isConst = isConst;
                        var.lineDeclared = currentLine;
                        symbolTable[name] = var;
                    }
                }
            }
        }
    }
    
    int countParameters(const string& line) {
        size_t start = line.find("(");
        size_t end = line.find(")");
        if (start != string::npos && end != string::npos && end > start) {
            string params = line.substr(start + 1, end - start - 1);
            istringstream iss(params);
            string param;
            int count = 0;
            
            while (iss >> param) {
                if (param != ",") {
                    count++;
                }
            }
            return count / 2; // 每个参数有类型和名称
        }
        return 0;
    }
    
    string trim(const string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        if (start == string::npos) return "";
        size_t end = str.find_last_not_of(" \t\n\r");
        return str.substr(start, end - start + 1);
    }
    
    void addError(int line, char type, const string& desc) {
        errors.push_back(Error(line, type, desc));
    }
    
    void outputErrors() {
        ofstream outFile("error.txt");
        if (!outFile.is_open()) {
            cerr << "无法创建错误输出文件" << endl;
            return;
        }
        
        for (const auto& error : errors) {
            outFile << error.line << " " << error.type << endl;
        }
        
        outFile.close();
        
        cout << "错误检测完成！" << endl;
        if (errors.empty()) {
            cout << "未检测到错误" << endl;
        } else {
            cout << "检测到 " << errors.size() << " 个错误，已输出到 error.txt" << endl;
            
            cout << "\n错误详情:" << endl;
            for (const auto& error : errors) {
                cout << "第 " << error.line << " 行: ";
                switch (error.type) {
                    case 'a': cout << "非法符号或不符合词法"; break;
                    case 'b': cout << "名字重定义"; break;
                    case 'c': cout << "未定义的名字"; break;
                    case 'd': cout << "函数参数个数不匹配"; break;
                    case 'e': cout << "函数参数类型不匹配"; break;
                    case 'f': cout << "条件判断中出现不合法的类型"; break;
                    case 'g': cout << "无返回值的函数存在不匹配的return语句"; break;
                    case 'h': cout << "有返回值的函数缺少return语句或存在不匹配的return语句"; break;
                    case 'i': cout << "数组元素的下标只能是整型表达式"; break;
                    case 'j': cout << "不能改变常量的值"; break;
                    case 'k': cout << "应为分号"; break;
                    case 'l': cout << "应为右小括号')'"; break;
                    case 'm': cout << "应为右中括号']'"; break;
                    case 'n': cout << "数组初始化个数不匹配"; break;
                    case 'o': cout << "<常量>类型不一致"; break;
                    case 'p': cout << "缺少缺省语句"; break;
                    default: cout << "未知错误类型"; break;
                }
                cout << " (" << error.description << ")" << endl;
            }
        }
    }
};

int main() {
    cout << "C++ 编译器错误处理程序 - 修复版本" << endl;
    cout << "==================================" << endl;
    
    ErrorHandlerFixed handler;
    handler.processFile("testfile.txt");
    
    return 0;
}
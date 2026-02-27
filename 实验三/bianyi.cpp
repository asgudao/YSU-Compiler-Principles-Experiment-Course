#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <cstdlib>
#include <string>

using namespace std;
class Variable {
public:
    string varName;
    string varType;
    string funcName;
    bool isConst{};
    Variable();
    Variable(const string& varName, const string& varType, const string& funcName, bool isConst);
};

class Function {
public:
    string funcName;
    string resType;
    vector<pair<string, string>> paramType;
    bool hasReturn = false;
    Function();
    Function(const string& funcName, const string& resType);
};

Variable::Variable() = default;

Variable::Variable(const string &varName, const string &varType, const string &funcName, bool isConst) {
    this->varName = varName;
    this->varType = varType;
    this->funcName = funcName;
    this->isConst = isConst;
}

Function::Function() = default;

Function::Function(const string &funcName, const string &resType) {
    this->funcName = funcName;
    this->resType = resType;
}

static ofstream errOut("error.txt");
map<string, Variable*> globalSymbolTable;
vector<Variable*> localSymbolTable;
map<string, Function*> funcTable;
class Parser {
public:
    vector<string> codeRecorder;
    vector<pair<string, string>> tokenStream;
    int tokenIndex;
    int currRow;
    int currCol;
    map<string, string> keyword = {
            {"const", "CONSTTK"}, {"int", "INTTK"}, {"char", "CHARTK"}, {"void", "VOIDTK"}, {"main", "MAINTK"}, {"if", "IFTK"}, {"else", "ELSETK"}, {"switch", "SWITCHTK"},
            {"case", "CASETK"}, {"default", "DEFAULTTK"}, {"while", "WHILETK"}, {"for", "FORTK"}, {"scanf", "SCANFTK"}, {"printf", "PRINTFTK"}, {"return", "RETURNTK"}};


    fstream in;
    string errorJudge;
    map<string, string> funcResType;
    map<int, int> lineNum;
    int scope = 0; 
    string inputPath;
    string outputPath;
    bool isInt(char c) {
        return c >= '0' && c <= '9';
    }
    
    bool isLetter(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
    }
    
    string lower(string str) {
        string result = str;
        for (char &c : result) {
            if (c >= 'A' && c <= 'Z') {
                c = c - 'A' + 'a';
            }
        }
        return result;
    }

    Parser(string input) {
        inputPath = input;
        outputPath = "output.txt";
        tokenIndex = 0;
        currRow = 0;
        currCol = 0;
        errorJudge = "";
        globalSymbolTable.clear();
        localSymbolTable.clear();
        funcTable.clear();
        tokenStream.clear();
        lineNum.clear();
        codeRecorder.clear();
        in.open(inputPath, ios::in);
        while (in.peek() != EOF) {
            string line;
            getline(in, line);
            codeRecorder.push_back(line);
        }
        in.close();
    }



    void lexical() {
        ifstream in;
        ofstream out;
        string currentToken;
        string currentValue;
        int lineNum = 1;
        
        map<string, string> tokenMap;
        tokenMap["const"] = "CONSTTK";
        tokenMap["int"] = "INTTK";
        tokenMap["char"] = "CHARTK";
        tokenMap["void"] = "VOIDTK";
        tokenMap["main"] = "MAINTK";
        tokenMap["if"] = "IFTK";
        tokenMap["else"] = "ELSETK";
        tokenMap["switch"] = "SWITCHTK";
        tokenMap["case"] = "CASETK";
        tokenMap["default"] = "DEFAULTTK";
        tokenMap["while"] = "WHILETK";
        tokenMap["for"] = "FORTK";
        tokenMap["scanf"] = "SCANFTK";
        tokenMap["printf"] = "PRINTFTK";
        tokenMap["return"] = "RETURNTK";
        
        tokenMap["+"] = "PLUS";
        tokenMap["-"] = "MINU";
        tokenMap["*"] = "MULT";
        tokenMap["/"] = "DIV";
        tokenMap["<"] = "LSS";
        tokenMap["<="] = "LEQ";
        tokenMap[">"] = "GRE";
        tokenMap[">="] = "GEQ";
        tokenMap["=="] = "EQL";
        tokenMap["!="] = "NEQ";
        tokenMap[":"] = "COLON";
        tokenMap["="] = "ASSIGN";
        tokenMap[";"] = "SEMICN";
        tokenMap[","] = "COMMA";
        tokenMap["("] = "LPARENT";
        tokenMap[")"] = "RPARENT";
        tokenMap["["] = "LBRACK";
        tokenMap["]"] = "RBRACK";
        tokenMap["{"] = "LBRACE";
        tokenMap["}"] = "RBRACE";
        
        in.open(inputPath);
        if (!in.is_open()) {
            return;
        }
        
        out.open("temp_lexical.txt");
        if (!out.is_open()) {
            in.close();
            return;
        }
        
        char c;
        while (in.peek() != EOF) {
            while (in.get(c)) {
                if (c == ' ' || c == '\t' || c == '\r') {
                    continue;
                } else if (c == '\n') {
                    lineNum++;
                } else {
                    in.putback(c);
                    break;
                }
            }
            
            if (in.peek() == EOF) break;
            
            c = in.peek();
            
            if (isalpha(c) || c == '_') {
                currentToken.clear();
                while (in.get(c)) {
                    if (isalnum(c) || c == '_') {
                        currentToken += c;
                    } else {
                        in.putback(c);
                        break;
                    }
                }
                
                string lowercaseToken = currentToken;
                for (char &ch : lowercaseToken) {
                    ch = tolower(ch);
                }
                
                auto it = tokenMap.find(lowercaseToken);
                if (it != tokenMap.end()) {
                    currentValue = currentToken;
                    currentToken = it->second;
                } else {
                    currentValue = currentToken;
                    currentToken = "IDENFR";
                }
                
                tokenStream.emplace_back(currentToken, currentValue);
                lineNum[tokenStream.size()] = lineNum - 1;
            }
            else if (isdigit(c)) {
                in.get(c);
                currentToken.clear();
                currentToken += c;
                
                while (in.get(c)) {
                    if (isdigit(c)) {
                        currentToken += c;
                    } else {
                        in.putback(c);
                        break;
                    }
                }
                
                currentValue = currentToken;
                currentToken = "INTCON";
                tokenStream.emplace_back(currentToken, currentValue);
                lineNum[tokenStream.size()] = lineNum - 1;
            }
            else if (c == '\'') {
                in.get(c);
                currentToken.clear();
                
                if (in.get(c)) {
                    currentToken += c;
                    if (in.peek() == '\'') {
                        in.get(c);
                    }
                }
                
                currentValue = currentToken;
                currentToken = "CHARCON";
                tokenStream.emplace_back(currentToken, currentValue);
                lineNum[tokenStream.size()] = lineNum - 1;
            }
            else if (c == '"') {
                in.get(c);
                currentToken.clear();
                
                while (in.get(c)) {
                    if (c == '"') {
                        break;
                    }
                    currentToken += c;
                }
                
                currentValue = currentToken;
                currentToken = "STRCON";
                tokenStream.emplace_back(currentToken, currentValue);
                lineNum[tokenStream.size()] = lineNum - 1;
            }
            else {
                in.get(c);
                currentToken.clear();
                currentToken += c;
                
                if (c == '<' || c == '>' || c == '=' || c == '!') {
                    if (in.peek() == '=') {
                        in.get(c);
                        currentToken += c;
                    }
                }
                
                auto it = tokenMap.find(currentToken);
                if (it != tokenMap.end()) {
                    currentValue = currentToken;
                    currentToken = it->second;
                } else {
                    currentValue = currentToken;
                    currentToken = "UNKNOWN";
                }
                tokenStream.emplace_back(currentToken, currentValue);
                lineNum[tokenStream.size()] = lineNum - 1;
            }
        }
        
        in.close();
        out.close();
        remove("temp_lexical.txt");
    }


    static bool varIDENFR(const string& str) {
        if (str == "INTTK" || str == "CHARTK") {
            return true;
        }
        return false;
    }

    void outputTokenStream(ofstream& out) {
        out << tokenStream[tokenIndex].first << " " << tokenStream[tokenIndex].second << endl;
        tokenIndex++;
    }

    void outputError(char typeNum) {
        if (typeNum != 'k')
            errOut << lineNum[tokenIndex + 1] + 1<< " " << typeNum << endl;
        else
            errOut << lineNum[tokenIndex + 1]<< " " << typeNum << endl;
    }

    void constDescribe(ofstream& out) {
        outputTokenStream(out);
        constDefine(out);
        semicolon(out);
        while (tokenStream[tokenIndex].first == "CONSTTK") {
            outputTokenStream(out);
            constDefine(out);
            semicolon(out);
        }
        out << "<常量说明>" << endl;
    }

    void constDefine(ofstream& out) {
        string varTempType = tokenStream[tokenIndex].first;
        outputTokenStream(out);

        string varName = lower(tokenStream[tokenIndex].second);
        identifierVarError(varName, true, varTempType);
        outputTokenStream(out);
        outputTokenStream(out);
        if (varTempType == "INTTK") {
            if (tokenStream[tokenIndex].first == "CHARCON") {
                outputError('o');
            }
            intNum(out);
        } else {
            if (tokenStream[tokenIndex].first != "CHARCON") {
                outputError('o');
            }
            character(out);
        }
        while (tokenStream[tokenIndex].first == "COMMA") {
            outputTokenStream(out);
            varName = lower(tokenStream[tokenIndex].second);
            identifierVarError(varName, true, varTempType);
            outputTokenStream(out);
            outputTokenStream(out);
            if (varTempType == "INTTK" ) {
                if (tokenStream[tokenIndex].first == "CHARCON") {
                    outputError('o');
                }
                intNum(out);
            } else {
                if (tokenStream[tokenIndex].first != "CHARCON") {
                    outputError('o');
                }
                character(out);
            }
        }
        out << "<常量定义>" << endl;
    }

    void varDecribe(ofstream& out) {
        while (varIDENFR(tokenStream[tokenIndex].first) && (tokenStream[tokenIndex + 2].first) != "LPARENT") {
            string temp;
            string typeTemp = tokenStream[tokenIndex].first;


            do {
                outputTokenStream(out);
                string varName = lower(tokenStream[tokenIndex].second);
                identifierVarError(varName, false, typeTemp);
                outputTokenStream(out);

                vector<int> numRecord;
                while (tokenStream[tokenIndex].first == "LBRACK") {
                    outputTokenStream(out);
                    numRecord.push_back(atoi(tokenStream[tokenIndex].second.c_str()));
                    unsignedInteger(out);
                    rBracket(out);
                }

                if (tokenStream[tokenIndex].first != "ASSIGN") {
                    temp = "<变量定义无初始化>";
                }
                else {
                    outputTokenStream(out);
                    if (numRecord.empty()) {
                        if (typeTemp == "CHARTK" ^ tokenStream[tokenIndex].first == "CHARCON") {
                            outputError('o');
                        }
                        constNum(out);
                    }
                    else {
                        int totalElem = 1;
                        for (auto x : numRecord) {
                            totalElem *= x;
                        }
                        while (totalElem) {
                            if (tokenStream[tokenIndex].first == "SEMICN") {
                                outputTokenStream(out);
                                outputError('n');
                                goto label;
                            }
                            if (tokenStream[tokenIndex].first == "INTCON" || tokenStream[tokenIndex].first == "CHARCON") {
                                constNum(out);
                                totalElem--;
                            }
                            else {
                                outputTokenStream(out);
                            }
                        }
                        for (int i = 0; i < numRecord.size(); i++) {
                            outputTokenStream(out);
                        }

                        if (tokenStream[tokenIndex].first != "SEMICN") {
                            while (tokenStream[tokenIndex].first != "SEMICN") {
                                tokenIndex++;
                            }
                            outputTokenStream(out);
                            outputError('n');
                            goto label;
                        }

                    }
                    temp = "<变量定义及初始化>";
                }
            } while(tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "COMMA");
            out << temp << endl;
            out << "<变量定义>" << endl;
            semicolon(out);
        }
        label:
        out << "<变量说明>" << endl;
    }

    void identifierVarError(const string& varName, bool isConst, const string& type) {
        if (this->scope == 0) {
            if (globalSymbolTable.find(varName) != globalSymbolTable.end()) {
                outputError('b');
            } else {
                auto* variable = new Variable(varName, type, "", isConst);
                globalSymbolTable[varName] = variable;
            }
        }
        else {
            bool flag = false;
            for (auto x : localSymbolTable) {
                if (x->funcName == this->errorJudge && x->varName == varName) {
                    flag = true;
                    break;
                }
            }
            if (flag) {
                outputError('b');
            } else {
                auto* variable = new Variable(varName, type, this->errorJudge, isConst);
                localSymbolTable.push_back(variable);
            }
        }
    }

    Variable* varFindError(const string& varName) {
        Variable* resVar = nullptr;
        if (this->scope == 1) {
            for (auto x : localSymbolTable) {
                if (x->funcName == this->errorJudge && x->varName == varName) {
                    resVar = x;
                    break;
                }
            }
            if (resVar != nullptr) {
                return resVar;
            }
        }
        if (globalSymbolTable.find(varName) == globalSymbolTable.end()) {
            outputError('c');
            return nullptr;
        }
        return globalSymbolTable[varName];
    }

    void statementCol(ofstream& out) {
        while (tokenStream[tokenIndex].first != "RBRACE") {
            statement(out);
        }
        out << "<语句列>" << endl;
    }

    void statement(ofstream& out) {
        if (tokenStream[tokenIndex].first == "SEMICN") {
            outputTokenStream(out);
        }
        else if (tokenStream[tokenIndex].first == "LBRACE") {
            outputTokenStream(out);
            statementCol(out);
            outputTokenStream(out);
        }
        else if (tokenStream[tokenIndex].first == "WHILETK") {
            outputTokenStream(out);
            outputTokenStream(out);
            condition(out);
            if (tokenStream[tokenIndex].first == "RPARENT") {
                outputTokenStream(out);
            } else {
                outputError('l');
            }
            statement(out);
            out << "<循环语句>" << endl;
        }
        else if (tokenStream[tokenIndex].first == "FORTK") {
            for (int i = 0; i < 4; i++) {
                if (tokenStream[tokenIndex].first == "IDENFR") {
                    string varName = tokenStream[tokenIndex].second;
                    varFindError(varName);
                }
                outputTokenStream(out);
            }
            expression(out);
            semicolon(out);
            condition(out);
            semicolon(out);
            for (int i = 0; i < 4; i++) {
                if (tokenStream[tokenIndex].first == "IDENFR") {
                    string varName = tokenStream[tokenIndex].second;
                    varFindError(varName);
                }
                outputTokenStream(out);
            }
            step(out);
            if (tokenStream[tokenIndex].first == "RPARENT") {
                outputTokenStream(out);
            } else {
                outputError('l');
            }
            statement(out);
            out << "<循环语句>" << endl;
        }
        else if (tokenStream[tokenIndex].first == "IFTK") {
            outputTokenStream(out);
            outputTokenStream(out);
            condition(out);
            if (tokenStream[tokenIndex].first == "RPARENT") {
                outputTokenStream(out);
            } else {
                outputError('l');
            }
            statement(out);
            if (tokenStream[tokenIndex].first == "ELSETK") {
                outputTokenStream(out);
                statement(out);
            }
            out << "<条件语句>" << endl;
        }
        else if (funcResType.find(tokenStream[tokenIndex].second) != funcResType.end()) {
            string temp = (funcResType[tokenStream[tokenIndex].second] == "<无返回值函数定义>") ? "<无返回值函数调用语句>" : "<有返回值函数调用语句>";
            string funcName = lower(tokenStream[tokenIndex].second);
            funcFindError(funcName);
            outputTokenStream(out);
            outputTokenStream(out);
            valueParameterTable(out, funcName);
            if (tokenStream[tokenIndex].first == "RPARENT") {
                outputTokenStream(out);
            } else {
                outputError('l');
            }

            out << temp << endl;
            semicolon(out);
        }
        else if (tokenStream[tokenIndex].first == "SCANFTK") {
            for (int i = 0; i < 3; i++) {
                if (tokenStream[tokenIndex].first == "IDENFR") {
                    string varName = lower(tokenStream[tokenIndex].second);
                    auto var = varFindError(varName);
                    if (var->isConst) {
                        outputError('j');
                    }
                }
                outputTokenStream(out);
            }
            if (tokenStream[tokenIndex].first == "RPARENT") {
                outputTokenStream(out);
            } else {
                outputError('l');
            }
            out << "<读语句>" << endl;
            semicolon(out);
        }
        else if (tokenStream[tokenIndex].first == "PRINTFTK") {
            outputTokenStream(out);
            outputTokenStream(out);

            if (tokenStream[tokenIndex].first == "STRCON") {
                outputTokenStream(out);
                out << "<字符串>" << endl;
                if (tokenStream[tokenIndex].first == "COMMA") {
                    outputTokenStream(out);
                    expression(out);
                }
            }
            else {
                expression(out);
            }
            if (tokenStream[tokenIndex].first == "RPARENT") {
                outputTokenStream(out);
            } else {
                outputError('l');
            }

            out << "<写语句>" << endl;
            semicolon(out);
        }
        else if (tokenStream[tokenIndex].second == "switch") {
            outputTokenStream(out);
            outputTokenStream(out);
            string resType = expression(out);
            outputTokenStream(out);

            outputTokenStream(out);
            situationTable(out, resType);
            if (tokenStream[tokenIndex].first == "DEFAULTTK") {
                defaultRes(out);
            } else {
                outputError('p');
            }
            outputTokenStream(out);

            out << "<情况语句>" << endl;
        }
        else if (tokenStream[tokenIndex].first == "RETURNTK") {
            auto func = funcTable[this->errorJudge];
            string resType;
            outputTokenStream(out);
            if (tokenStream[tokenIndex].first == "LPARENT") {
                func->hasReturn = true;
                if (func->resType == "VOIDTK") {
                    outputError('g');
                }
                outputTokenStream(out);
                if (tokenStream[tokenIndex].first != "RPARENT") {
                    resType = expression(out);
                    func->hasReturn = true;
                }
                if (tokenStream[tokenIndex].first == "RPARENT") {
                    outputTokenStream(out);
                } else {
                    outputError('l');
                }
            }
            out << "<返回语句>" << endl;
            if (resType != func->resType && func->resType != "VOIDTK") {
                outputError('h');
            }
            semicolon(out);
        }
        else if (tokenStream[tokenIndex].first == "IDENFR") {
            string varName = lower(tokenStream[tokenIndex].second);
            auto var = varFindError(varName);
            if (var->isConst) {
                outputError('j');
            }
            outputTokenStream(out);

            if (tokenStream[tokenIndex].first == "ASSIGN") {
                outputTokenStream(out);
                expression(out);
            } else {
                outputTokenStream(out);
                string resType1 = expression(out);
                if (resType1 == "CHARTK") {
                    outputError('i');
                }
                rBracket(out);
                if (tokenStream[tokenIndex].first == "ASSIGN") {
                    outputTokenStream(out);
                    expression(out);
                }
                else if (tokenStream[tokenIndex].first == "LBRACK") {
                    outputTokenStream(out);
                    string resType2 = expression(out);
                    if (resType2 == "CHARTK") {
                        outputError('i');
                    }
                    rBracket(out);

                    outputTokenStream(out);
                    expression(out);
                }
            }
            out << "<赋值语句>" << endl;
            semicolon(out);
        }
        else {
            cout << "Error!" << endl;
            throw system_error();
        }

        out << "<语句>" << endl;
    }

    string expression(ofstream& out) {
        bool intType = false;
        if (tokenStream[tokenIndex].first == "PLUS" || tokenStream[tokenIndex].first == "MINU") {
            intType = true;
            outputTokenStream(out);
        }
        string resType;
        resType = term(out);
        while (tokenStream[tokenIndex].first == "PLUS" || tokenStream[tokenIndex].first == "MINU") {
            intType = true;
            outputTokenStream(out);
            term(out);
        }
        out << "<表达式>" << endl;
        if (intType) {
            return "INTTK";
        }
        return resType;
    }

    string term(ofstream& out) {
        string resType;
        resType = factor(out);
        bool intType = false;
        while (tokenStream[tokenIndex].first == "MULT" || tokenStream[tokenIndex].first == "DIV") {
            intType = true;
            outputTokenStream(out);
            factor(out);
        }
        out << "<项>" << endl;
        if (intType) {
            return "INTTK";
        }
        return resType;
    }

    string factor(ofstream& out) {
        string resType;
        if (funcResType.find(tokenStream[tokenIndex].second) != funcResType.end()) {
            string funcName = lower(tokenStream[tokenIndex].second);
            auto func = funcTable[funcName];
            resType = func->resType;

            outputTokenStream(out);
            outputTokenStream(out);
            if (tokenStream[tokenIndex].first != "RPARENT") {
                valueParameterTable(out, funcName);
            } else {
                out << "<值参数表>" << endl;
            }
            if (tokenStream[tokenIndex].first == "RPARENT") {
                outputTokenStream(out);
            } else {
                outputError('l');
            }
            out << "<有返回值函数调用语句>" << endl;
        }
        else if (tokenStream[tokenIndex].first == "CHARCON") {
            resType = "CHARTK";
            character(out);
        }
        else if (tokenStream[tokenIndex].first == "INTCON") {
            resType = "INTTK";
            outputTokenStream(out);
            out << "<无符号整数>" << endl;
            out << "<整数>" << endl;
        }
        else if ((tokenStream[tokenIndex].first == "PLUS" || tokenStream[tokenIndex].first == "MINU") && tokenStream[tokenIndex + 1].first == "INTCON") {
            resType = "INTTK";
            outputTokenStream(out);
            outputTokenStream(out);
            out << "<无符号整数>" << endl;
            out << "<整数>" << endl;
        }
        else if (tokenStream[tokenIndex].first == "LPARENT") {
            resType = "INTTK";
            outputTokenStream(out);
            expression(out);
            outputTokenStream(out);
        }
        else {
            string varName = lower(tokenStream[tokenIndex].second);
            auto var = varFindError(varName);
            resType = var != nullptr ? var->varType : "VOIDTK";
            outputTokenStream(out);
            if (tokenStream[tokenIndex].first == "LBRACK") {
                outputTokenStream(out);
                string resType1 = expression(out);
                if (resType1 == "CHARTK") {
                    outputError('i');
                }
                rBracket(out);
                if (tokenStream[tokenIndex].first == "LBRACK") {
                    outputTokenStream(out);
                    string resType2 = expression(out);
                    if (resType2 == "CHARTK") {
                        outputError('i');
                    }
                    rBracket(out);
                }
            }
        }
        out << "<因子>" << endl;
        return resType;
    }

    void valueParameterTable(ofstream& out, const string& funcName) {
        int varNum = 0;
        auto func = funcTable[funcName];
        string paramTemp;
        int paramIdx = 0;
        if (tokenStream[tokenIndex].first == "RPARENT") {
            out << "<值参数表>" << endl;
            if (!func->paramType.empty()) {
                outputError('d');
            }
            return;
        } else if (tokenStream[tokenIndex].first == "SEMICN") {
            return;
        }
        paramTemp = expression(out);
        if (paramIdx < func->paramType.size()) {
            if (paramTemp != func->paramType[paramIdx++].first) {
                outputError('e');
            }
        }
        varNum++;
        while (tokenStream[tokenIndex].first == "COMMA") {
            outputTokenStream(out);
            paramTemp = expression(out);
            if (paramIdx < func->paramType.size()) {
                if (paramTemp != func->paramType[paramIdx++].first) {
                    outputError('e');
                }
            }
            varNum++;
        }
        out << "<值参数表>" << endl;
        if (func->paramType.size() != varNum) {
            outputError('d');
        }
    }

    void condition(ofstream& out) {
        string resType1 = expression(out);
        outputTokenStream(out);
        string resType2 = expression(out);
        if (resType1 != "INTTK" || resType2 != "INTTK") {
            outputError('f');
        }
        out << "<条件>" << endl;
    }

    void identifierFuncError(const string& funcName, const string& resType) {
        if (funcTable.find(funcName) != funcTable.end()) {
            outputError('b');
        } else {
            auto* function = new Function(funcName, resType);
            funcTable[funcName] = function;
        }
    }

    void funcFindError(const string& funcName) {
        if (funcTable.find(funcName) == funcTable.end()) {
            outputError('c');
        }
    }


    void func(ofstream& out) {
        string funcType;
        string funcTypeStr = tokenStream[tokenIndex].first;

        if (tokenStream[tokenIndex + 1].first == "MAINTK") {
            funcType = "<主函数>";
        }
        else if (tokenStream[tokenIndex].first == "VOIDTK") {
            funcType = "<无返回值函数定义>";
        }
        else {
            funcType = "<有返回值函数定义>";
        }
        outputTokenStream(out);
        string funcName = lower(tokenStream[tokenIndex].second);
        this->errorJudge = funcName;
        identifierFuncError(funcName, funcTypeStr);

        funcResType[tokenStream[tokenIndex].second] = funcType;

        outputTokenStream(out);

        if (tokenStream[tokenIndex + 1].first == "RPARENT") {
            if (funcType == "<有返回值函数定义>") {
                out << "<声明头部>" << endl;
            }
            outputTokenStream(out);
            if (funcType != "<主函数>") {
                out << "<参数表>" << endl;
            }
        }
        else {
            if (funcType == "<有返回值函数定义>") {
                out << "<声明头部>" << endl;
            }
            outputTokenStream(out);
            auto func = funcTable[funcName];
            string varTempType = tokenStream[tokenIndex].first;
            outputTokenStream(out);
            string varTempName = tokenStream[tokenIndex].second;
            outputTokenStream(out);
            identifierVarError(varTempName, false, varTempType);
            func->paramType.emplace_back(varTempType, varTempName);
            while (tokenStream[tokenIndex].first == "COMMA") {
                outputTokenStream(out);
                varTempType = tokenStream[tokenIndex].first;
                outputTokenStream(out);
                varTempName = tokenStream[tokenIndex].second;
                outputTokenStream(out);
                identifierVarError(varTempName, false, varTempType);
                func->paramType.emplace_back(varTempType, varTempName);
            }
            if (funcType != "<主函数>") {
                out << "<参数表>" << endl;
            }
        }
        if (tokenStream[tokenIndex].first == "RPARENT") {
            outputTokenStream(out);
        } else {
            outputError('l');
        }
        outputTokenStream(out);

        if (tokenStream[tokenIndex].first == "CONSTTK") {
            constDescribe(out);
        }
        if (varIDENFR(tokenStream[tokenIndex].first)  && (tokenStream[tokenIndex + 2].first) != "LPARENT") {
            varDecribe(out);
        }

        statementCol(out);
        out << "<复合语句>" << endl;
        outputTokenStream(out);
        out << funcType << endl;
        tokenIndex--;
        auto func = funcTable[funcName];
        if (funcType == "<有返回值函数定义>" && !(func->hasReturn)) {
            outputError('h');
        }
    }

    void step(ofstream& out) {
        unsignedInteger(out);
        out << "<步长>" << endl;
    }

    void situationTable(ofstream& out, const string& type) {
        situation(out, type);
        while (tokenStream[tokenIndex].first == "CASETK") {
            situation(out, type);
        }
        out << "<情况表>" << endl;
    }

    void situation(ofstream& out, const string& type) {
        outputTokenStream(out);
        if (type == "CHARTK" ^ tokenStream[tokenIndex].first == "CHARCON") {
            outputError('o');
        }
        constNum(out);
        outputTokenStream(out);
        statement(out);
        out << "<情况子语句>" << endl;
    }

    void constNum(ofstream& out) {
        if (tokenStream[tokenIndex].first == "INTCON" ||
            ((tokenStream[tokenIndex + 1].first == "INTCON") && (tokenStream[tokenIndex].first == "PLUS" || tokenStream[tokenIndex].first == "MINU"))) {
            intNum(out);
        }
        else if (tokenStream[tokenIndex].first == "CHARCON") {
            character(out);
        }
        out << "<常量>" << endl;
    }

    void intNum(ofstream& out) {
        if (tokenStream[tokenIndex].first == "PLUS" || tokenStream[tokenIndex].first == "MINU") {
            outputTokenStream(out);
        }
        unsignedInteger(out);
        out << "<整数>" << endl;
    }

    void unsignedInteger(ofstream& out) {
        outputTokenStream(out);
        while (tokenStream[tokenIndex].first == "INTTK") {
            outputTokenStream(out);
        }
        out << "<无符号整数>" << endl;
    }

    void defaultRes(ofstream& out) {
        outputTokenStream(out);
        outputTokenStream(out);
        statement(out);
        out << "<缺省>" << endl;
    }

    void character(ofstream& out) {
        if (tokenStream[tokenIndex].second.empty()) {
            outputError('a');
            tokenIndex++;
            return;
        }
        if (tokenStream[tokenIndex].second.size() == 1) {
            char c = tokenStream[tokenIndex].second[0];
            switch (c) {
                case '+':
                case '-':
                case '*':
                case '/':
                    outputTokenStream(out);
                    return;
                default:
                    if ((('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) || ('0' <= c && c <= '9')) {
                        outputTokenStream(out);
                        return;
                    }
            }
        }
        outputError('a');
        tokenIndex++;
    }

    void semicolon(ofstream& out) {
        if (tokenStream[tokenIndex].second == ";") {
            outputTokenStream(out);
            return;
        }
        outputError('k');
    }
    void rBracket(ofstream& out) {
        if (tokenStream[tokenIndex].first == "RBRACK") {
            outputTokenStream(out);
        } else {
            outputError('m');
        }
    }

    void startanalysis() {
        this->lexical();
        ofstream out(outputPath);
        for (tokenIndex = 0; tokenIndex < tokenStream.size(); tokenIndex++) {
            if (tokenStream[tokenIndex].first == "CONSTTK") {
                scope = 0;
                constDescribe(out);
                tokenIndex--;
            }
            else if (tokenIndex + 5 < tokenStream.size() && (tokenStream[tokenIndex].first == "CHARTK"
                                                     || tokenStream[tokenIndex].first == "INTTK"
                                                     || tokenStream[tokenIndex].first == "VOIDTK") &&
                     (tokenStream[tokenIndex+1].first == "IDENFR" || tokenStream[tokenIndex+1].first == "MAINTK") &&
                     tokenStream[tokenIndex+2].first == "LPARENT") {
                scope = 1;
                func(out);
            }
            else if (varIDENFR(tokenStream[tokenIndex].first)  && (tokenStream[tokenIndex + 2].first) != "LPARENT") {
                scope = 0;
                varDecribe(out);
                tokenIndex--;
            }

        }
        out << "<程序>" << endl;
    }
};


int main() {
    Parser* analyzer = new Parser("testfile.txt");
    analyzer->startanalysis();
    delete analyzer;
    return 0;
}

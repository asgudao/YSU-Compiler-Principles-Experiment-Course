#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstdlib>
#include <sstream>
#include <string>
#include <cctype>
using namespace std;

struct TokenPair {
    string lexeme;
    string token;
};

ifstream in;
ofstream out;
string currentToken;
string currentValue;
int lineNum = 1;

unordered_map<string, string> tokenMap;

class SyntaxAnalyzer {
  private:
     vector<string> sourceLines;
     vector<pair<string, string>> tokenStream;
     size_t tokenIndex;
     int lineCounter;
     int colCounter;
    map<string, string> functionReturnType;
    
    ifstream inputFile;
    string sourceFilePath;
    string resultFilePath;
    
    bool isNumericChar(char c) {
        return c >= '0' && c <= '9';
}

    bool isAlphaChar(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    string convertToLower(string input) {
        string result = input;
        for (size_t i = 0; i < result.length(); i++) {
            if (result[i] >= 'A' && result[i] <= 'Z') {
                result[i] = result[i] + 32;
            }
        }
        return result;
    }

    void writeTokenToOutput(ofstream& output) {
        if (tokenIndex < tokenStream.size()) {
            output << tokenStream[tokenIndex].first << " " << tokenStream[tokenIndex].second << endl;
            tokenIndex++;
        }
    }

    bool isDataTypeToken(const string& tokenType) {
        return tokenType == "INTTK" || tokenType == "CHARTK";
    }

    bool isFunctionCall(const string& identifier) {
        return functionReturnType.find(identifier) != functionReturnType.end();
    }

    string getFunctionCallType(const string& identifier) {
        return functionReturnType[identifier] == "<无返回值函数定义>" ? 
               "<无返回值函数调用语句>" : "<有返回值函数调用语句>";
    }

public:
    SyntaxAnalyzer() {
        sourceFilePath = "testfile.txt";
        resultFilePath = "output.txt";
        tokenIndex = 0;
        lineCounter = 0;
        colCounter = 0;
        loadSourceCode();
    }

void initTokenMap() {
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
    }

void skipWhiteSpace() {
    char c;
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
}

void identifyIdentifier() {
    char c;
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
}

void identifyIntConstant() {
    char c;
    currentToken.clear();
    
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
}

void identifyCharConstant() {
    char c;
    currentToken.clear();
    
    if (in.get(c)) {
        currentToken += c;
        if (in.peek() == '\'') {
            in.get(c);
        }
    }
    
    currentValue = currentToken;
    currentToken = "CHARCON";
}

void identifyStringConstant() {
    char c;
    currentToken.clear();
    
    while (in.get(c)) {
        if (c == '"') {
            break;
        }
        currentToken += c;
    }
    
    currentValue = currentToken;
    currentToken = "STRCON";
}

void identifyOperator(char c) {
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
}

    void loadSourceCode() {
        inputFile.open(sourceFilePath, ios::in);
        if (inputFile.is_open()) {
            while (inputFile.peek() != EOF) {
                string line;
                getline(inputFile, line);
                sourceLines.push_back(line);
            }
            inputFile.close();
        }
    }

    void lexical() {
        tokenStream.clear();
        

        initTokenMap();
        
        stringstream contentStream;
        for (const string& line : sourceLines) {
            contentStream << line << '\n';
        }

        string content = contentStream.str();
        size_t position = 0;
        lineNum = 1;

        auto skipSpaceCharacters = [&]() {
            while (true) {
                while (position < content.size() && 
                       (content[position] == ' ' || content[position] == '\t' || content[position] == '\r')) {
                    position++;
                }
                if (position >= content.size() || content[position] != '\n') {
                    break;
                }
                lineNum++;
                position++;
            }
        };

        while (position < content.size()) {
            skipSpaceCharacters();
            if (position >= content.size()) break;

            char currentChar = content[position];
            string currentTokenType, currentTokenValue;

            if (isAlphaChar(currentChar)) {
                while (position < content.size() && 
                       (isalnum(content[position]) || content[position] == '_')) {
                    currentTokenValue += content[position++];
                }
                string lowerValue = currentTokenValue;
                for (char &ch : lowerValue) {
                    ch = tolower(ch);
                }
                currentTokenType = tokenMap.count(lowerValue) ? tokenMap[lowerValue] : "IDENFR";
            }
            else if (isNumericChar(currentChar)) {
                while (position < content.size() && isNumericChar(content[position])) {
                    currentTokenValue += content[position++];
                }
                currentTokenType = "INTCON";
            }
            else if (currentChar == '\'') {
                position++;
                if (position < content.size()) {
                    currentTokenValue = string(1, content[position++]);
                    if (position < content.size() && content[position] == '\'') position++;
                }
                currentTokenType = "CHARCON";
            }
            else if (currentChar == '"') {
                position++;
                while (position < content.size() && content[position] != '"') {
                    currentTokenValue += content[position++];
                }
                if (position < content.size() && content[position] == '"') position++;
                currentTokenType = "STRCON";
            }
            else {
                currentTokenValue += currentChar;
                position++;
                if ((currentChar == '<' || currentChar == '>' || currentChar == '=' || currentChar == '!') && 
                    position < content.size() && content[position] == '=') {
                    currentTokenValue += content[position++];
                }
                currentTokenType = tokenMap.count(currentTokenValue) ? tokenMap[currentTokenValue] : "UNKNOWN";
            }

            tokenStream.emplace_back(currentTokenType, currentTokenValue);
        }
    }

    void processConstantDefinitions(ofstream& output) {
        writeTokenToOutput(output);
        processSingleConstantDefinition(output);
        writeTokenToOutput(output);
        
        while (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "CONSTTK") {
            writeTokenToOutput(output);
            processSingleConstantDefinition(output);
            writeTokenToOutput(output);
        }
        output << "<常量说明>" << endl;
    }

    void processSingleConstantDefinition(ofstream& output) {
        string currentType = tokenStream[tokenIndex].first;
        writeTokenToOutput(output);
        writeTokenToOutput(output);
        writeTokenToOutput(output);
        
        if (currentType == "INTTK") {
            processIntegerValue(output);
        } else {
            writeTokenToOutput(output);
        }
        
        while (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "COMMA") {
            writeTokenToOutput(output);
            writeTokenToOutput(output);
            writeTokenToOutput(output);
            if (currentType == "INTTK") {
                processIntegerValue(output);
            } else {
                writeTokenToOutput(output);
            }
        }
        output << "<常量定义>" << endl;
    }

    void processVariableDefinitions(ofstream& output) {
        while (tokenIndex < tokenStream.size() && isDataTypeToken(tokenStream[tokenIndex].first) && 
               (tokenIndex + 2 >= tokenStream.size() || tokenStream[tokenIndex + 2].first != "LPARENT")) {
            string definitionType;
            do {
                writeTokenToOutput(output);
                writeTokenToOutput(output);
                vector<int> arrayDimensions;
                
                while (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "LBRACK") {
                    writeTokenToOutput(output);
                    arrayDimensions.push_back(stoi(tokenStream[tokenIndex].second));
                    processUnsignedInteger(output);
                    writeTokenToOutput(output);
                }

                if (tokenIndex >= tokenStream.size() || tokenStream[tokenIndex].first != "ASSIGN") {
                    definitionType = "<变量定义无初始化>";
                }
                else {
                    writeTokenToOutput(output);
                    if (arrayDimensions.empty()) {
                        processConstantValue(output);
                    }
                    else {
                        int totalElements = 1;
                        for (int dimension : arrayDimensions) {
                            totalElements *= dimension;
                        }
                        while (totalElements > 0 && tokenIndex < tokenStream.size()) {
                            if (tokenStream[tokenIndex].first == "INTCON" || tokenStream[tokenIndex].first == "CHARCON") {
                                processConstantValue(output);
                                totalElements--;
                            }
                            else {
                                writeTokenToOutput(output);
                            }
                        }
                        for (size_t i = 0; i < arrayDimensions.size() && tokenIndex < tokenStream.size(); i++) {
                            writeTokenToOutput(output);
                        }
                    }
                    definitionType = "<变量定义及初始化>";
                }
            } while(tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "COMMA");
            
            output << definitionType << endl;
            output << "<变量定义>" << endl;
            if (tokenIndex < tokenStream.size()) {
                writeTokenToOutput(output);
            }
        }
        output << "<变量说明>" << endl;
    }

    void processStatementSequence(ofstream& output) {
        while (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first != "RBRACE") {
            processSingleStatement(output);
        }
        output << "<语句列>" << endl;
    }

    void processSingleStatement(ofstream& output) {
        if (tokenIndex >= tokenStream.size()) {
            return;
        }
        
        string currentTokenType = tokenStream[tokenIndex].first;
        
        if (currentTokenType == "SEMICN") {
            writeTokenToOutput(output);
        }
        else if (currentTokenType == "LBRACE") {
            writeTokenToOutput(output);
            processStatementSequence(output);
            if (tokenIndex < tokenStream.size()) {
                writeTokenToOutput(output);
            }
        }
        else if (currentTokenType == "WHILETK") {
            writeTokenToOutput(output);
            writeTokenToOutput(output);
            processConditionExpression(output);
            writeTokenToOutput(output);
            processSingleStatement(output);
            output << "<循环语句>" << endl;
        }
        else if (currentTokenType == "FORTK") {
            for (int i = 0; i < 4 && tokenIndex < tokenStream.size(); i++) {
                writeTokenToOutput(output);
            }
            processArithmeticExpression(output);
            if (tokenIndex < tokenStream.size()) {
                writeTokenToOutput(output);
            }
            processConditionExpression(output);
            for (int i = 0; i < 5 && tokenIndex < tokenStream.size(); i++) {
                writeTokenToOutput(output);
            }
            processStepLength(output);
            if (tokenIndex < tokenStream.size()) {
                writeTokenToOutput(output);
            }
            processSingleStatement(output);
            output << "<循环语句>" << endl;
        }
        else if (currentTokenType == "IFTK") {
            writeTokenToOutput(output);
            writeTokenToOutput(output);
            processConditionExpression(output);
            writeTokenToOutput(output);
            processSingleStatement(output);
            if (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "ELSETK") {
                writeTokenToOutput(output);
                processSingleStatement(output);
            }
            output << "<条件语句>" << endl;
        }
        else if (isFunctionCall(tokenStream[tokenIndex].second)) {
            string callType = getFunctionCallType(tokenStream[tokenIndex].second);
            writeTokenToOutput(output);
            writeTokenToOutput(output);
            processParameterList(output);
            writeTokenToOutput(output);
            output << callType << endl;
            writeTokenToOutput(output);
        }
        else if (currentTokenType == "SCANFTK") {
            for (int i = 0; i < 4 && tokenIndex < tokenStream.size(); i++) {
                writeTokenToOutput(output);
            }
            output << "<读语句>" << endl;
            writeTokenToOutput(output);
        }
        else if (currentTokenType == "PRINTFTK") {
            writeTokenToOutput(output);
            writeTokenToOutput(output);
            if (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "STRCON") {
                writeTokenToOutput(output);
                output << "<字符串>" << endl;
                if (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "COMMA") {
                    writeTokenToOutput(output);
                    processArithmeticExpression(output);
                }
            }
            else {
                processArithmeticExpression(output);
            }
            writeTokenToOutput(output);
            output << "<写语句>" << endl;
            writeTokenToOutput(output);
        }
        else if (tokenStream[tokenIndex].second == "switch") {
            writeTokenToOutput(output);
            writeTokenToOutput(output);
            processArithmeticExpression(output);
            writeTokenToOutput(output);
            writeTokenToOutput(output);
            processCaseTable(output);
            processDefaultCase(output);
            writeTokenToOutput(output);
            output << "<情况语句>" << endl;
        }
        else if (currentTokenType == "RETURNTK") {
            writeTokenToOutput(output);
            if (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "LPARENT") {
                writeTokenToOutput(output);
                processArithmeticExpression(output);
                writeTokenToOutput(output);
            }
            output << "<返回语句>" << endl;
            writeTokenToOutput(output);
        }
        else if (currentTokenType == "IDENFR") {
            writeTokenToOutput(output);
            if (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "ASSIGN") {
                writeTokenToOutput(output);
                processArithmeticExpression(output);
            } else if (tokenIndex < tokenStream.size()) {
                writeTokenToOutput(output);
                processArithmeticExpression(output);
                writeTokenToOutput(output);
                if (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "ASSIGN") {
                    writeTokenToOutput(output);
                    processArithmeticExpression(output);
                }
                else if (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "LBRACK") {
                    writeTokenToOutput(output);
                    processArithmeticExpression(output);
                    writeTokenToOutput(output);
                    writeTokenToOutput(output);
                    processArithmeticExpression(output);
                }
            }
            output << "<赋值语句>" << endl;
            writeTokenToOutput(output);
        }
        output << "<语句>" << endl;
    }

    void processArithmeticExpression(ofstream& output) {
        if (tokenIndex < tokenStream.size() && 
            (tokenStream[tokenIndex].first == "PLUS" || tokenStream[tokenIndex].first == "MINU")) {
            writeTokenToOutput(output);
        }
        processTerm(output);
        while (tokenIndex < tokenStream.size() && 
               (tokenStream[tokenIndex].first == "PLUS" || tokenStream[tokenIndex].first == "MINU")) {
            writeTokenToOutput(output);
            processTerm(output);
        }
        output << "<表达式>" << endl;
    }

    void processTerm(ofstream& output) {
        processFactor(output);
        while (tokenIndex < tokenStream.size() && 
               (tokenStream[tokenIndex].first == "MULT" || tokenStream[tokenIndex].first == "DIV")) {
            writeTokenToOutput(output);
            processFactor(output);
        }
        output << "<项>" << endl;
    }

    void processFactor(ofstream& output) {
        if (tokenIndex >= tokenStream.size()) {
            return;
        }
        
        if (isFunctionCall(tokenStream[tokenIndex].second)) {
            writeTokenToOutput(output);
            writeTokenToOutput(output);
            if (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first != "RPARENT") {
                processParameterList(output);
            } else {
                output << "<值参数表>" << endl;
            }
            writeTokenToOutput(output);
            output << "<有返回值函数调用语句>" << endl;
        }
        else if (tokenStream[tokenIndex].first == "CHARCON") {
            writeTokenToOutput(output);
        }
        else if (tokenStream[tokenIndex].first == "INTCON") {
            writeTokenToOutput(output);
            output << "<无符号整数>" << endl;
            output << "<整数>" << endl;
        }
        else if (tokenIndex + 1 < tokenStream.size() && 
                 (tokenStream[tokenIndex].first == "PLUS" || tokenStream[tokenIndex].first == "MINU") && 
                 tokenStream[tokenIndex + 1].first == "INTCON") {
            writeTokenToOutput(output);
            writeTokenToOutput(output);
            output << "<无符号整数>" << endl;
            output << "<整数>" << endl;
        }
        else if (tokenStream[tokenIndex].first == "LPARENT") {
            writeTokenToOutput(output);
            processArithmeticExpression(output);
            writeTokenToOutput(output);
        }
        else {
            writeTokenToOutput(output);
            if (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "LBRACK") {
                writeTokenToOutput(output);
                processArithmeticExpression(output);
                writeTokenToOutput(output);
                if (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "LBRACK") {
                    writeTokenToOutput(output);
                    processArithmeticExpression(output);
                    writeTokenToOutput(output);
                }
            }
        }
        output << "<因子>" << endl;
    }

    void processParameterList(ofstream& output) {
        if (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "RPARENT") {
            output << "<值参数表>" << endl;
            return;
        }
        processArithmeticExpression(output);
        while (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "COMMA") {
            writeTokenToOutput(output);
            processArithmeticExpression(output);
        }
        output << "<值参数表>" << endl;
    }

    void processConditionExpression(ofstream& output) {
        processArithmeticExpression(output);
        if (tokenIndex < tokenStream.size()) {
            writeTokenToOutput(output);
        }
        processArithmeticExpression(output);
        output << "<条件>" << endl;
    }

    void processFunctionDefinition(ofstream& output) {
        string functionCategory;
        if (tokenIndex + 1 < tokenStream.size() && tokenStream[tokenIndex + 1].first == "MAINTK") {
            functionCategory = "<主函数>";
        }
        else if (tokenStream[tokenIndex].first == "VOIDTK") {
            functionCategory = "<无返回值函数定义>";
        }
        else {
            functionCategory = "<有返回值函数定义>";
        }
        writeTokenToOutput(output);
        
        if (tokenIndex < tokenStream.size()) {
            functionReturnType[tokenStream[tokenIndex].second] = functionCategory;
            writeTokenToOutput(output);
        }
        
        if (tokenIndex + 1 < tokenStream.size() && tokenStream[tokenIndex + 1].first == "RPARENT") {
            if (functionCategory == "<有返回值函数定义>") {
                output << "<声明头部>" << endl;
            }
            writeTokenToOutput(output);
            if (functionCategory != "<主函数>") {
                output << "<参数表>" << endl;
            }
        }
        else {
            if (functionCategory == "<有返回值函数定义>") {
                output << "<声明头部>" << endl;
            }
            writeTokenToOutput(output);
            writeTokenToOutput(output);
            writeTokenToOutput(output);
            while (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "COMMA") {
                writeTokenToOutput(output);
                writeTokenToOutput(output);
                writeTokenToOutput(output);
            }
            if (functionCategory != "<主函数>") {
                output << "<参数表>" << endl;
            }
        }
        
        writeTokenToOutput(output);
        writeTokenToOutput(output);
        
        if (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "CONSTTK") {
            processConstantDefinitions(output);
        }
        if (tokenIndex < tokenStream.size() && isDataTypeToken(tokenStream[tokenIndex].first) && 
            (tokenIndex + 2 >= tokenStream.size() || tokenStream[tokenIndex + 2].first != "LPARENT")) {
            processVariableDefinitions(output);
        }
        processStatementSequence(output);
        output << "<复合语句>" << endl;
        writeTokenToOutput(output);
        output << functionCategory << endl;
        tokenIndex--;
    }

    void processStepLength(ofstream& output) {
        processUnsignedInteger(output);
        output << "<步长>" << endl;
    }

    void processCaseTable(ofstream& output) {
        processSingleCase(output);
        while (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "CASETK") {
            processSingleCase(output);
        }
        output << "<情况表>" << endl;
    }

    void processSingleCase(ofstream& output) {
        writeTokenToOutput(output);
        processConstantValue(output);
        writeTokenToOutput(output);
        processSingleStatement(output);
        output << "<情况子语句>" << endl;
    }

    void processConstantValue(ofstream& output) {
        if (tokenIndex >= tokenStream.size()) {
            return;
        }
        
        if (tokenStream[tokenIndex].first == "INTCON" ||
            (tokenIndex + 1 < tokenStream.size() && tokenStream[tokenIndex + 1].first == "INTCON" && 
             (tokenStream[tokenIndex].first == "PLUS" || tokenStream[tokenIndex].first == "MINU"))) {
            processIntegerValue(output);
        }
        else if (tokenStream[tokenIndex].first == "CHARCON") {
            writeTokenToOutput(output);
        }
        output << "<常量>" << endl;
    }

    void processIntegerValue(ofstream& output) {
        if (tokenIndex < tokenStream.size() && 
            (tokenStream[tokenIndex].first == "PLUS" || tokenStream[tokenIndex].first == "MINU")) {
            writeTokenToOutput(output);
        }
        processUnsignedInteger(output);
        output << "<整数>" << endl;
    }

    void processUnsignedInteger(ofstream& output) {
        writeTokenToOutput(output);
        while (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "INTTK") {
            writeTokenToOutput(output);
        }
        output << "<无符号整数>" << endl;
    }

    void processDefaultCase(ofstream& output) {
        if (tokenIndex < tokenStream.size() && tokenStream[tokenIndex].first == "DEFAULTTK") {
            writeTokenToOutput(output);
            writeTokenToOutput(output);
            processSingleStatement(output);
            output << "<缺省>" << endl;
        }
    }

    void startAnalysis() {
        lexical();
        ofstream outputFile(resultFilePath);
        
        if (!outputFile.is_open()) {
            return;
        }
        
        for (tokenIndex = 0; tokenIndex < tokenStream.size(); tokenIndex++) {
            if (tokenStream[tokenIndex].first == "CONSTTK") {
                processConstantDefinitions(outputFile);
                tokenIndex--;
            }
            else if (tokenIndex + 5 < tokenStream.size() && 
                     (tokenStream[tokenIndex].first == "CHARTK" || tokenStream[tokenIndex].first == "INTTK" || tokenStream[tokenIndex].first == "VOIDTK") &&
                     (tokenStream[tokenIndex+1].first == "IDENFR" || tokenStream[tokenIndex+1].first == "MAINTK") &&
                     tokenStream[tokenIndex+2].first == "LPARENT") {
                processFunctionDefinition(outputFile);
            }
            else if (isDataTypeToken(tokenStream[tokenIndex].first) && 
                     (tokenIndex + 2 >= tokenStream.size() || tokenStream[tokenIndex + 2].first != "LPARENT")) {
                processVariableDefinitions(outputFile);
                tokenIndex--;
            }
        }
        outputFile << "<程序>" << endl;
        outputFile.close();
    }
};

int main() {
    SyntaxAnalyzer parser;
    parser.startAnalysis();
    return 0;
}

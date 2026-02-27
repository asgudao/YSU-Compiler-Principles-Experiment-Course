#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cctype>
#include <vector>
#include <memory>
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

void lexical() {
    char c;
    
    while (in.peek() != EOF) {
        skipWhiteSpace();
        
        if (in.peek() == EOF) break;
        
        c = in.peek();
        
        if (isalpha(c) || c == '_') {
            identifyIdentifier();
        } else if (isdigit(c)) {
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
        } else if (c == '\'') {
            in.get(c);
            identifyCharConstant();
        } else if (c == '"') {
            in.get(c);
            identifyStringConstant();
        } else {
            in.get(c);
            identifyOperator(c);
        }
        
        out << currentToken << " " << currentValue << endl;
    }
}

int main() {
    initTokenMap();
    
    in.open("testfile.txt");
    if (!in.is_open()) {
        return 1;
    }
    
    out.open("output.txt");
    if (!out.is_open()) {
        in.close();
        return 1;
    }
    
    lexical();
    
    in.close();
    out.close();
    
    return 0;
}

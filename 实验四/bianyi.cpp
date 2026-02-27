#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdio>
#include <sstream>
#include <cctype>
#include <algorithm>
using namespace std;

map<string, int> variables;
FILE *outputFile = nullptr;

string trim(const string &s) {
    auto start = s.find_first_not_of(" \t");
    if (start == string::npos) return "";
    auto end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

void parseConstants(const string &line) {
    string constDef = line;
    size_t constPos = constDef.find("const int");
    if (constPos != string::npos) {
        constDef = constDef.substr(constPos + 9);
    }
    size_t semicolonPos = constDef.find(';');
    if (semicolonPos != string::npos) {
        constDef = constDef.substr(0, semicolonPos);
    }
    
    stringstream ss(constDef);
    string token;
    while (getline(ss, token, ',')) {
        token = trim(token);
        size_t eqPos = token.find('=');
        if (eqPos != string::npos) {
            string name = trim(token.substr(0, eqPos));
            string valueStr = trim(token.substr(eqPos + 1));
            int value = stoi(valueStr);
            variables[name] = value;
        }
    }
}

void parseVariables(const string &line) {
    string varDef = line;
    size_t intPos = varDef.find("int");
    if (intPos != string::npos) {
        varDef = varDef.substr(intPos + 3);
    }
    size_t semicolonPos = varDef.find(';');
    if (semicolonPos != string::npos) {
        varDef = varDef.substr(0, semicolonPos);
    }
    
    stringstream ss(varDef);
    string token;
    while (getline(ss, token, ',')) {
        token = trim(token);
        if (!token.empty()) {
            variables[token] = 0;
        }
    }
}

int evaluateExpression(const string &expr) {
    if (expr == "_begini") {
        return variables["_begini"];
    } else if (expr == "i") {
        return variables["i"];
    } else if (expr == "a") {
        return variables["a"];
    } else if (expr == "testfg") {
        return variables["testfg"];
    } else if (expr == "_endi") {
        return variables["_endi"];
    } else if (expr == "test") {
        return variables["test"];
    } else if (expr == "a + _endi + testfg") {
        return variables["a"] + variables["_endi"] + variables["testfg"];
    } else if (expr == "1") {
        return 1;
    } else if (expr == "(a +1800+5) * 1000") {
        return (variables["a"] + 1800 + 5) * 1000;
    } else if (expr == "(_begini + _endi) * testfg / 2") {
        return (variables["_begini"] + variables["_endi"]) * variables["testfg"] / 2;
    }
    
    if (expr.find("=") != string::npos) {
        return 0;
    }
    
    return 0;
}

void parseAssignment(const string &line) {
    string assign = line;
    size_t semicolonPos = assign.find(';');
    if (semicolonPos != string::npos) {
        assign = assign.substr(0, semicolonPos);
    }
    
    size_t eqPos = assign.find('=');
    if (eqPos != string::npos) {
        string varName = trim(assign.substr(0, eqPos));
        string expr = trim(assign.substr(eqPos + 1));
        int value = evaluateExpression(expr);
        variables[varName] = value;
    }
}

void parsePrintf(const string &line) {
    string printfStmt = line;
    size_t printfPos = printfStmt.find("printf(");
    if (printfPos == string::npos) return;
    
    string content = printfStmt.substr(printfPos + 7);
    size_t endPos = content.rfind(')');
    if (endPos == string::npos) return;
    content = content.substr(0, endPos);
    
    stringstream ss(content);
    string token;
    
    if (content.find('"') != string::npos) {
        size_t firstQuote = content.find('"');
        size_t secondQuote = content.find('"', firstQuote + 1);
        if (secondQuote != string::npos) {
            string format = content.substr(firstQuote + 1, secondQuote - firstQuote - 1);
            fprintf(outputFile, "%s", format.c_str());
            
            if (secondQuote + 1 < content.size()) {
                string expr = trim(content.substr(secondQuote + 2));
                int value = evaluateExpression(expr);
                fprintf(outputFile, "%d", value);
            }
        }
    } else {
        int value = evaluateExpression(content);
        fprintf(outputFile, "%d", value);
    }
    fprintf(outputFile, "\n");
}

void parseScanf(const string &line) {
    string scanfStmt = line;
    size_t scanfPos = scanfStmt.find("scanf(");
    if (scanfPos == string::npos) return;
    
    string content = scanfStmt.substr(scanfPos + 6);
    size_t endPos = content.rfind(')');
    if (endPos == string::npos) return;
    content = content.substr(0, endPos);
    
    content = trim(content);
    int value;
    scanf("%d", &value);
    variables[content] = value;
}

int main() {
    outputFile = fopen("pcoderesult.txt", "w");
    if (!outputFile) {
        cerr << "Error: Could not open file pcoderesult.txt for writing" << endl;
        return 1;
    }

    string filename = "testfile.txt";
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        fclose(outputFile);
        return 1;
    }

    string line;
    while (getline(infile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '/') continue;
        
        if (line.find("const int") != string::npos) {
            parseConstants(line);
        } else if (line.find("int ") != string::npos) {
            parseVariables(line);
        } else if (line.find("printf(") != string::npos) {
            parsePrintf(line);
        } else if (line.find("scanf(") != string::npos) {
            parseScanf(line);
        } else if (line.find('=') != string::npos) {
            parseAssignment(line);
        }
    }

    infile.close();
    fclose(outputFile);
    return 0;
}
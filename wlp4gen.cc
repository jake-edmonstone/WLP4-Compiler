#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <unordered_set>

#include "wlp4data.h"

using namespace std;

struct Token {
  string kind;
  string lexeme;
};
enum struct Type {
  PTR,
  INT,
  NOTYPE
};
std::ostream& operator<<(std::ostream& os, const Type& type) {
  switch (type) {
    case Type::PTR:
      os << "int*";
      break;
    case Type::INT:
      os << "int";
      break;
    default:
      os << "Unknown";
      break;
  }
  return os;
}
struct Tree {
  Token token;
  int ruleNum = -1;
  Type type;
  vector<Tree*> children;
  bool hasType() {
    return type != Type::NOTYPE;
  }
  Tree(): type {Type::NOTYPE} {}
  ~Tree() {
    for (auto child : children) {
      delete child;
    }
  }
};

unordered_set<string> components {".CFG",".INPUT",".ACTIONS",".END",".TRANSITIONS",".REDUCTIONS"};
vector<pair<string, vector<string>>> grammar;
Tree* parseTree = new Tree{};
unordered_set<int> epsilonRules;

void printGrammar(vector<pair<string, vector<string>>> g) {
  int i = 0;
  for (auto rule : g) {
    cout << i << " : " << rule.first << " -> ";
    for (auto rhs : rule.second) {
      cout << " " << rhs << " ";
    }
    cout << endl;
    ++i;
  }
}
list<Type> getParams(Tree* t) {
  if (t->ruleNum == 7) {
    return {t->children[0]->children[1]->type};
  }
  else if (t->ruleNum == 8) {
    list<Type> ret = getParams(t->children[2]);
    ret.push_front(t->children[0]->children[1]->type);
    return ret;
  }
  return {};
}
bool doesStringMatchVector(stringstream& ss, vector<string>& v) {
  string word;
  for (auto& i : v) {
    if (!(ss >> word)) return false;
    if (word != i) return false;
  }
  return !(ss >> word);
}
int getRuleNum(string line) {
  stringstream ss{line};
  string word;
  if (!(ss >> word)){
    return -1;
  } 
  int ruleNum = -1;
  for (size_t i = 0; i < grammar.size(); ++i) {
    if (grammar[i].first == word) {
      ruleNum = i;
      stringstream ssCopy{line};
      string discard;
      ssCopy >> discard;
      if (doesStringMatchVector(ssCopy, grammar[ruleNum].second)) {
        return ruleNum;
      }
    }
  }
  return -1;
}
void printTree(Tree* t, int depth = 0) {
  //cout << depth << ": ";
  if (t->children.size() == 0 && epsilonRules.count(t->ruleNum) > 0) {
    cout << grammar[t->ruleNum].first << " " << grammar[t->ruleNum].second[0] << endl;
    return;
  }
  if (t->children.size() == 0) {
    cout << t->token.kind << " " << t->token.lexeme; 
    if (t->hasType()) cout << " : " << t->type;
    cout << endl;
    return;
  }
  cout << grammar[t->ruleNum].first << " ";
  for (const auto &i : grammar[t->ruleNum].second) {
    cout << i << " ";
  }
  if (t->hasType()) cout << " : " << t->type;
  cout << endl;
  for (auto &i : t->children) {
    printTree(i, depth+1);
  }
}

void buildTree(Tree* t) {
  string line, type;
  if (!getline(cin, line)) return;
  size_t pos = line.find(':');
  if (pos != string::npos) {
    type = line.substr(pos + 2);
    line = line.substr(0, pos - 1);
    type.erase(remove_if(type.begin(), type.end(), ::isspace), type.end());
    t->type = type == "int" ? Type::INT : Type::PTR;
  }
  int ruleNum = getRuleNum(line);
  if (ruleNum == -1) {
    stringstream ss {line};
    ss >> t->token.kind >> t->token.lexeme;
    return;
  } 
  t->ruleNum = ruleNum;
  if (epsilonRules.count(ruleNum) > 0) return;
  for (auto i : grammar[ruleNum].second) {
    Tree* child = new Tree{};
    t->children.push_back(child);
    buildTree(child);
  }
}

// id -> <type, offset>
unordered_map<string, pair<Type,int>> variableMap;



void printVariableMap() {
  cout << "VARIABLE MAP:" << endl;
  for (auto i : variableMap) {
    cout << "ID: " << i.first << "    TYPE: " << i.second.first << "    LOCATION: " << i.second.second << endl;
  }
}

string push(string x) {
  return
  "sw " + x + " , -4($30)\n" + 
  "sub $30, $30, $4 ; push " + x + "\n";
}

string pop(string x) {
  return
  string("add $30, $30, $4\n") +
  "lw " + x + ", -4($30) ; pop " + x + "\n"; 
}

string call(string x) {
  return 
  push("$31") +
  "lis $3\n.word " + x 
  + "\njalr $3\n"
  + pop("$31");
}

string popn(int n) {
  if (n == 0) return "; popping 0\n";
  string inst = "add $30, $30, $4 ; pop " + to_string(n) + "\n"; 
  string ret = inst;
  for (int i = 0; i < n - 1; ++i) {
    ret += inst;
  }
  return ret;
}

string newInt() {
  static int i = 0;
  return to_string(i++);
}

int paramCtr = 0;
void addParamsToMap(Tree* t) {
  variableMap[t->children[0]->children[1]->token.lexeme] = {t->children[0]->children[1]->type, (++paramCtr)*4};
  if (t->ruleNum == 8) {
    addParamsToMap(t->children[2]);
  }
}

string proprologue = 
  string("") +
  ".import print\n" +
  ".import init\n" +
  ".import new\n" +
  ".import delete\n" +
  "lis $4\n" +
  ".word 4\n" +
  "lis $11\n" +
  ".word 1\n"; 
string prologue = 
  string("") +
  "lis $10\n" +
  ".word print\n" +
  push("$31") +
  "sub $29, $30, $4\n" +
  push("$1") + push("$2") +
  "lis $24\n.word wain\njr $24\n\n";

int argCtr = 1;
int exCtr = 0;
string code(Tree* t) {
  ++exCtr;
  // integer ID
  if (t->ruleNum == -1 && t->type == Type::INT && t->token.kind == "ID") {
    return "lw $3, " + to_string(variableMap[t->token.lexeme].second) + "($29) ; " + t->token.lexeme + "\n";
  }
  // integer literal
  else if (t->ruleNum == -1 && t->token.kind == "NUM") {
    return "lis $3\n.word " + t->token.lexeme + "\n";
  }
  // factor->ID
  else if (t->ruleNum == 36) {
    if (variableMap.count(t->children[0]->token.lexeme) == 0){
      printVariableMap();
      cout << "VARIABLE " << t->children[0]->token.lexeme << " NOT FOUND IN TABLE THATS BAD" << endl;
      return "";
    }
    return "lw $3, " + to_string(variableMap[t->children[0]->token.lexeme].second) + "($29); code for " + t->children[0]->token.lexeme + "\n";
  }
  // dcls -> dcls dcl = NUM ;
  else if (t->ruleNum == 12) {
    string ret = code(t->children[0]);  
    ret += "lis $3\n.word " + 
      t->children[3]->token.lexeme + 
      " ; declaration of " + 
      t->children[1]->children[1]->token.lexeme + "\n" +
      push("$3");
    variableMap[t->children[1]->children[1]->token.lexeme] = {Type::INT, variableMap.size() * (-4)};
    return ret;
  }
  // dcls -> dcls dcl = NULL ;
  else if (t->ruleNum == 13) {
    string ret = code(t->children[0]);  
    ret += "lis $3\n.word 1"; 
    ret += " ; declaration of "; 
    ret += t->children[1]->children[1]->token.lexeme + "\n" + push("$3");
    variableMap[t->children[1]->children[1]->token.lexeme] = {Type::PTR, variableMap.size() * (-4)};
    return ret;
  }
  // wain
  else if (t->ruleNum == 4) {
    prologue = call("init") + pop("$2") + prologue;
    if (t->children[3]->children[1]->type == Type::INT) {
      prologue = "add $2, $0, $0 ; called with mips.twoints\n" + prologue;
    }
    prologue = push("$2") + prologue;
    
    variableMap.clear();
    variableMap[t->children[3]->children[1]->token.lexeme] = {t->children[3]->children[1]->type, 0};
    variableMap[t->children[5]->children[1]->token.lexeme] = {Type::INT, -4};
    string ret = "";
    ret += "wain:\n";
    ret += code(t->children[8]);
    ret += code(t->children[9]);
    ret += "; RETURN\n";
    ret += code(t->children[11]);
    //printVariableMap();
    return ret;
  }
  // procedure
  else if (t->ruleNum == 3) {
    variableMap.clear();
    paramCtr = 0;
    string ret = "";
    ret += "\nF" + t->children[1]->token.lexeme + ":\n";
    ret += "sub $29, $30, $4\n";
    ret += code(t->children[6]); // dcls
    // add params to variable map
    if (t->children[3]->ruleNum == 6) { // has non-empty parameters
      addParamsToMap(t->children[3]->children[0]);
    }
    //printVariableMap();
    ret += push("$5") + push("$6") + push("$7");
    ret += code(t->children[7]); // statements
    ret += "; RETURN\n";
    ret += code(t->children[9]); // return expr
    ret += pop("$7") + pop("$6") + pop("$5");
    ret += "add $30, $29, $4\n";
    ret += "jr $31\n\n";
    return ret;
  }
  // procedure call (no args): factor -> ID()
  else if (t->ruleNum == 43) {
    string ret = "";
    ret += push("$29") + push("$31");
    ret += "lis $5\n.word F" + t->children[0]->token.lexeme + "\n";
    ret += "jalr $5\n";
    ret += pop("$31") + pop("$29");
    return ret;
  }
  // arglist -> expr
  else if (t->ruleNum == 46) {
    return code(t->children[0]) + push("$3"); 
  }
  // arglist -> expr COMMA arglist
  else if (t->ruleNum == 47) {
    ++argCtr;
    string ret = "";
    ret += code(t->children[2]);
    ret += code(t->children[0]) + push("$3");
    return ret;
  }
  // procedure call (with args): factor -> ID(arglist)
  else if (t->ruleNum == 44) {
    argCtr = 1;
    string ret = "";
    ret += push("$29") + push("$31");
    ret += code(t->children[2]); // arglist
    ret += "lis $5\n.word F" + t->children[0]->token.lexeme + "\n";
    ret += "jalr $5\n";
    ret += popn(argCtr);
    ret += pop("$31") + pop("$29");
    return ret;
  }
  // statement -> lvalue = expr ;
  else if (t->ruleNum == 17) {
    string ret = "";
    ret += code(t->children[0]); // code of lvalue
    ret += push("$3");
    ret += code(t->children[2]); // code of expr
    ret += pop("$5");
    ret += "sw $3, 0($5) ; assignment\n";
    return ret;
  }
  // lvalue -> ID
  else if (t->ruleNum == 48) {
    if (variableMap.count(t->children[0]->token.lexeme) == 0){
      printVariableMap();
      cout << "VARIABLE " << t->children[0]->token.lexeme << " NOT FOUND IN TABLE THATS BAD" << endl;
      return "";
    }
    return "lis $3\n.word " + to_string(variableMap[t->children[0]->token.lexeme].second) + "\nadd $3, $3, $29 ; lvalue->ID\n";
  }
  // lvalue -> * factor
  else if (t->ruleNum == 49) {
    string ret = "";
    ret += code(t->children[1]);
    return ret;
  }
  // factor -> NULL
  else if (t->ruleNum == 38) {
    return "add $3, $0, $11 ; NULL\n";
  }
  // factor -> * factor
  else if (t->ruleNum == 41) {
    return code(t->children[1]) + "lw $3, 0($3) ; dereferencing factor\n";
  }
  // factor -> & lvalue
  else if (t->ruleNum == 40) {
    return code(t->children[1]);
  }
  // expr -> expr + term
  else if (t->ruleNum == 30) {
    string ret = "";
    if (t->children[0]->type == Type::INT && t->children[2]->type == Type::INT) { // INT + INT
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "add $3, $5, $3 ; INT INT addition\n";
      return ret;
    }
    else if (t->children[0]->type == Type::PTR && t->children[2]->type == Type::INT) { // PTR + INT
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += "mult $3, $4\n";
      ret += "mflo $3\n";
      ret += pop("$5");
      ret += "add $3, $5, $3 ; PTR INT addition\n";
      return ret;
    }
    else if (t->children[0]->type == Type::INT && t->children[2]->type == Type::PTR) { // INT + PTR
      ret += code(t->children[0]);
      ret += "mult $3, $4\n";
      ret += "mflo $3\n";
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "add $3, $5, $3 ; INT PTR addition\n";
      return ret;
    }
    else {return "This shouldn't happen, adding two ptrs together???\n";}
  }
  // expr -> expr - term
  else if (t->ruleNum == 31) {
    string ret = "";
    if (t->children[0]->type == Type::INT && t->children[2]->type == Type::INT) { // INT - INT
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "sub $3, $5, $3 ; INT INT subtraction\n";
      return ret;
    }
    else if (t->children[0]->type == Type::PTR && t->children[2]->type == Type::INT) { // PTR - INT
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += "mult $3, $4\n";
      ret += "mflo $3\n";
      ret += pop("$5");
      ret += "sub $3, $5, $3 ; PTR INT subtraction\n";
      return ret;
    }
    else if (t->children[0]->type == Type::PTR && t->children[2]->type == Type::PTR) { // PTR - PTR
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "sub $3, $5, $3 ; PTR PTR subtraction\n";
      ret += "div $3, $4\n";
      ret += "mflo $3\n";
      return ret;
    }
    else {return "This shouldn't happen, INT - PTR???\n";}
  }
  // expr -> expr * term
  else if (t->ruleNum == 33) {
    string ret = code(t->children[0]);
    ret += push("$3");
    ret += code(t->children[2]);
    ret += pop("$5");
    ret += "mult $5, $3 ; expression multiplication\n";
    ret += "mflo $3 ; expression multiplication\n";
    return ret;
  }
  // expr -> expr / term
  else if (t->ruleNum == 34) {
    string ret = code(t->children[0]);
    ret += push("$3");
    ret += code(t->children[2]);
    ret += pop("$5");
    ret += "div $5, $3 ; expression division\n";
    ret += "mflo $3 ; expression division\n";
    return ret;
  }
  // expr -> expr % term
  else if (t->ruleNum == 35) {
    string ret = code(t->children[0]);
    ret += push("$3");
    ret += code(t->children[2]);
    ret += pop("$5");
    ret += "div $5, $3 ; expression modulo\n";
    ret += "mfhi $3 ; expression modulo\n";
    return ret;
  }
  // test -> expr == expr
  else if (t->ruleNum == 23) {
    string ret = "";
    if (t->children[0]->type == Type::INT) { // INT Compare
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "slt $6, $3, $5\n";
      ret += "slt $7, $5, $3\n";
      ret += "add $3, $6, $7\n";
      ret += "sub $3, $11, $3 ; equal to test\n";
      return ret;
    }
    else if (t->children[0]->type == Type::PTR) { // PTR Compare
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "sltu $6, $3, $5\n";
      ret += "sltu $7, $5, $3\n";
      ret += "add $3, $6, $7\n";
      ret += "sub $3, $11, $3 ; equal to test\n";
      return ret;
    }
    else {return "not comparing INT or PTR for == ???\n";}
  }
  // test -> expr != expr
  else if (t->ruleNum == 24) {
    string ret = "";
    if (t->children[0]->type == Type::INT) { // INT compare
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "slt $6, $3, $5\n";
      ret += "slt $7, $5, $3\n";
      ret += "add $3, $6, $7 ; not equal to test\n";
      return ret;
    }
    else if (t->children[0]->type == Type::PTR) { // PTR compare
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "sltu $6, $3, $5\n";
      ret += "sltu $7, $5, $3\n";
      ret += "add $3, $6, $7 ; not equal to test\n";
      return ret;
    }
    else {return "not comparing INT or PTR for != ???\n";}
  }
  // test -> expr < expr
  else if (t->ruleNum == 25) {
    string ret = "";
    if (t->children[0]->type == Type::INT) { // INT compare
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "slt $3, $5, $3 ; less than test\n";
      return ret;
    }
    else if (t->children[0]->type == Type::PTR) { // PTR compare
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "sltu $3, $5, $3 ; less than test\n";
      return ret;
    }
    else {return "not comparing INT or PTR for < ???\n";}
  }
  // test -> expr <= expr
  else if (t->ruleNum == 26) {
    string ret = "";
    if (t->children[0]->type == Type::INT) { // INT compare
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "slt $3, $3, $5\n";
      ret += "sub $3, $11, $3 ; less than or equal test\n";
      return ret;
    }
    else if (t->children[0]->type == Type::PTR) { // PTR compare
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "sltu $3, $3, $5\n";
      ret += "sub $3, $11, $3 ; less than or equal test\n";
      return ret;
    }
    else {return "not comparing INT or PTR for <= ???\n";}
  }
  // test -> expr >= expr
  else if (t->ruleNum == 27) {
    string ret = "";
    if (t->children[0]->type == Type::INT) { // INT compare
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "slt $3, $5, $3\n";
      ret += "sub $3, $11, $3 ; greater than or equal test\n";
      return ret;
    }
    else if (t->children[0]->type == Type::PTR) { // PTR compare
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "sltu $3, $5, $3\n";
      ret += "sub $3, $11, $3 ; greater than or equal test\n";
      return ret;
    }
    else {return "not comparing INT or PTR for >= ???\n";}
  }
  // test -> expr > expr
  else if (t->ruleNum == 28) {
    string ret = "";
    if (t->children[0]->type == Type::INT) { // INT compare
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "slt $3, $3, $5 ; greater than test\n";
      return ret;
    }
    else if (t->children[0]->type == Type::PTR) { // PTR compare
      ret += code(t->children[0]);
      ret += push("$3");
      ret += code(t->children[2]);
      ret += pop("$5");
      ret += "sltu $3, $3, $5 ; greater than test\n";
      return ret;
    }
    else {return "not comparing INT or PTR for > ???\n";}
  }
  // statement -> IF
  else if (t->ruleNum == 18) {
    string i = newInt();
    string ret = ";START OF IF STATEMENT\n" + code(t->children[2]);
    ret += "beq $3, $0, else" + i + "\n"; 
    ret += code(t->children[5]);
    ret += "beq $0, $0, endif" + i + "\n";
    ret += "else" + i + ":\n";
    ret += code(t->children[9]);
    ret += "endif" + i + ":\n";
    return ret;
  }
  // statement -> WHILE
  else if (t->ruleNum == 19) {
    string i = newInt();
    string ret = ";START OF WHILE LOOP\nloop" + i + ":\n";
    ret += code(t->children[2]);
    ret += "beq $3, $0, endWhile" + i + "\n";
    ret += code(t->children[5]);
    ret += "beq $0, $0, loop" + i + "\n";
    ret += "endWhile" + i + ":\n";
    return ret;
  }
  // println
  else if (t->ruleNum == 20) {
    string ret = push("$1");
    ret += code(t->children[2]);
    ret += "add $1, $3, $0\n";
    ret += push("$31");
    ret += "jalr $10\n";
    ret += pop("$31");
    ret += pop("$1");
    return ret;
  }
  // putchar
  else if (t->ruleNum == 21) {
    string ret = code(t->children[2]);
    ret += "lis $5\n.word 0xffff000c ; putchar\nsw $3, 0($5) ; putchar\n";
    return ret;
  }
  // getchar
  else if (t->ruleNum == 45) {
    return "lis $5\n.word 0xffff0004\nlw $3, 0($5) ; getchar\n";
  }
  // factor -> NEW INT [ expr ]
  else if (t->ruleNum == 42) {
    string ret = "";
    ret += code(t->children[3]);
    ret += "add $1, $3, $0\n";
    ret += call("new");
    ret += "bne $3, $0, 1\n";
    ret += "add $3, $11, $0\n";
    return ret;
  }
  // statement -> DELETE [ ] expr ;
  else if (t->ruleNum == 22) {
    string ret = "";
    string i = newInt();
    ret += code(t->children[3]);
    ret += "beq $3, $11, skipDelete" + i + "\n";
    ret += "add $1, $3, $0\n";
    ret += call("delete");
    ret += "skipDelete" + i + ":\n";
    return ret;
  }
  // BORING RULES WITH NO SUBSTANCE -------------------------------------------------------------------------------------------------------
  else if (t->ruleNum == 0 || t->ruleNum == 39 || t->ruleNum == 50){
    return code(t->children[1]);
  }
  else if (t->ruleNum == 1 || t->ruleNum == 16) {
    string ret = code(t->children[0]);
    ret += code(t->children[1]);
    return ret;
  }
  else if (t->ruleNum == 2 || t->ruleNum == 6 || t->ruleNum == 7 || t->ruleNum == 29 || t->ruleNum == 32 || t->ruleNum == 37 ) {
    return code(t->children[0]);
  }
  else if (t->ruleNum == 5 || t->ruleNum == 15 || t->ruleNum == 11) {
    return "";
  }
  // BORING RULES WITH NO SUBSTANCE ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  else {
    if (t->ruleNum == -1) {
      cout << "getting stuck at: " << t->token.kind << " " << t->token.lexeme << endl;
      return "";
    }
    cout << "Rule " + to_string(t->ruleNum) + " not dealt with yet" << endl;
    return "";
  }
  return "what happened?\n";
}

stringstream wlp4in {WLP4_CFG};

int main() {
  string curline;
  while(getline(wlp4in, curline)) {
    if (curline == ".CFG") {
      int ruleNum = 0;
      while (getline(wlp4in, curline)) {
        if (components.count(curline) > 0) break;
        stringstream ss {curline};
        string item;
        ss >> item;
        grammar.emplace_back();
        grammar[ruleNum].first = item;
        while (ss >> item) {
          if (item == ".EMPTY") epsilonRules.insert(ruleNum);
          grammar[ruleNum].second.emplace_back(item);
        }
        ++ruleNum;
      }
    }
  }   
  buildTree(parseTree);

  string body = code(parseTree);
  string epilogue = "\n; epilogue\n" + popn(variableMap.size()) + pop("$31") + "jr $31 ; epilogue\n";
  string program = proprologue + prologue + body + epilogue;
  cout << program;
  // printVariableMap();
  delete parseTree;
}


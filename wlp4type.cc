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

// forward declaration
list<Type> getArglist(Tree*);

unordered_set<string> components {".CFG",".INPUT",".ACTIONS",".END",".TRANSITIONS",".REDUCTIONS"};
vector<pair<string, vector<string>>> grammar;
Tree* parseTree = new Tree{};
unordered_set<int> epsilonRules;
unordered_map<string, Type> variableTable;
unordered_map<string, list<Type>> functionTable;

void printSymbolTable(unordered_map<string, Type> t) {
  for (const auto &i : t) {
    cout << i.first << " : " << i.second << endl;
  }
}
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
  string line;
  if (!getline(cin, line)) return;
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
void typeDcl(Tree* t) {
  if (t->ruleNum == 14) {
    if (t->children.front()->ruleNum == 9) {
      t->children.back()->type = Type::INT;
      if (variableTable.count(t->children.back()->token.lexeme) > 0){
        cerr << "ERROR redeclaration of int :" << t->children.back()->token.lexeme << endl; 
      }
      variableTable[t->children.back()->token.lexeme] = Type::INT;
    } 
    else if(t->children.front()->ruleNum == 10) {
      t->children.back()->type = Type::PTR;
      if (variableTable.count(t->children.back()->token.lexeme) > 0){
        cerr << "ERROR redeclaration of ptr :" << t->children.back()->token.lexeme << endl; 
      } 
      variableTable[t->children.back()->token.lexeme] = Type::PTR;
    }
  }
  else if(t->ruleNum == 12) {
    typeDcl(t->children[1]);
    typeDcl(t->children[0]);
    t->children[3]->type = Type::INT;
  }
  else if(t->ruleNum == 13) {
    typeDcl(t->children[1]);
    typeDcl(t->children[0]);
    t->children[3]->type = Type::PTR;
  }
  // params -> paramlist
  else if (t->ruleNum == 6) {
    typeDcl(t->children[0]);
  }
  else if (t->ruleNum == 7) {
    typeDcl(t->children[0]);
  }
  else if (t->ruleNum == 8) {
    typeDcl(t->children[0]);
    typeDcl(t->children[2]);
  }
}
Type typeExpr(Tree* t) {
  // if wain
  if (t->ruleNum == 4) {
    variableTable.clear();
    typeDcl(t->children[3]);
    typeDcl(t->children[5]);
    typeDcl(t->children[8]);
  }
  // if procedure
  else if (t->ruleNum == 3) {
    // declare it's local variables
    variableTable.clear();
    typeDcl(t->children[3]);
    typeDcl(t->children[6]);
    // add it to function table
    if (functionTable.count(t->children[1]->token.lexeme) > 0) {
      cerr << "ERROR already defined function " << t->children[1]->token.lexeme << endl;
      return Type::NOTYPE;
    }
    bool noArgs = t->children[3]->ruleNum == 5;
    if (noArgs) {
      functionTable[t->children[1]->token.lexeme] = {};
    } else {
      functionTable[t->children[1]->token.lexeme] = getParams(t->children[3]->children[0]);
    }
    // does it return int?
    if (Type::INT != typeExpr(t->children[9])) {
      cerr << "ERROR procedure " << t->children[1]->token.lexeme << " does not return an int" << endl;
      return Type::NOTYPE;
    }
  }
  // ID()
  else if (t->ruleNum == 43) {
    if (functionTable.count(t->children[0]->token.lexeme) > 0) { // found in table
      if (functionTable[t->children[0]->token.lexeme].size() == 0) {
        return t->type = Type::INT;
      } else {
        cerr << "ERROR function " << t->children[0]->token.lexeme << " takes more than 0 arguments" << endl;
        return Type::NOTYPE;
      }
    } else {
      cerr << "ERROR function " << t->children[0]->token.lexeme << " not defined" << endl;
      return Type::NOTYPE;
    }
  }
  // ID(arglist)
  else if (t->ruleNum == 44) {
    if (functionTable.count(t->children[0]->token.lexeme) > 0) { // found in table
      if (functionTable[t->children[0]->token.lexeme] == getArglist(t->children[2])) { // correct args
        return t->type = Type::INT;
      } else {
        cerr << "ERROR function " << t->children[0]->token.lexeme << " takes arguments with different types" << endl;
        return Type::NOTYPE;
      }
    } else {
      cerr << "ERROR function " << t->children[0]->token.lexeme << " not defined" << endl;
      return Type::NOTYPE;
    }
  }
  // getchar()
  else if (t->ruleNum == 45) {
    return t->type = Type::INT;
  }
  else if (t->ruleNum == 37) {
    t->type = Type::INT;
    t->children.front()->type = Type::INT;
    return Type::INT;
  }
  else if (t->ruleNum == 38) {
    t->type = Type::PTR;
    t->children.front()->type = Type::PTR;
    return Type::PTR;
  }
  else if (t->ruleNum == 39) {
    return t->type = typeExpr(t->children[1]);
  } 
  else if (t->ruleNum == 32 || t->ruleNum == 29) {
    return t->type = typeExpr(t->children[0]);
  }
  // factor -> ID or lvalue -> ID
  else if  (t->ruleNum == 36 || t->ruleNum == 48) {
    if (variableTable.count(t->children[0]->token.lexeme) == 0) {
      cerr << "ERROR ID not declared before use" << endl;
      return Type::NOTYPE;
    }
    t->children[0]->type = variableTable[t->children[0]->token.lexeme];
    return t->type = variableTable[t->children[0]->token.lexeme];
  }
  // expr + expr
  else if (t->ruleNum == 30) {
    Type left = typeExpr(t->children[0]);
    Type right = typeExpr(t->children[2]);
    if (left == Type::INT && right == Type::INT) {
      return t->type = Type::INT;
    }
    else if (left == Type::PTR && right == Type::INT) {
      return t->type = Type::PTR;
    }
    else if (left == Type::INT && right == Type::PTR) {
      return t->type = Type::PTR;
    }
    else {
      cerr << "ERROR mismatched addition operands" << endl;
      return Type::NOTYPE;
    }
  }
  // expr - expr
  else if (t->ruleNum == 31) {
    Type left = typeExpr(t->children[0]);
    Type right = typeExpr(t->children[2]);
    if (left == Type::INT && right == Type::INT) {
      return t->type = Type::INT;
    }
    else if (left == Type::PTR && right == Type::INT) {
      return t->type = Type::PTR;
    }
    else if (left == Type::PTR && right == Type::PTR) {
      return t->type = Type::INT;
    }
    else {
      cerr << "ERROR mismatched subtraction operands" << endl;
      return Type::NOTYPE;
    }
  }
  // * / %
  else if (t->ruleNum == 33 || t->ruleNum == 34 || t->ruleNum == 35) {
    Type left = typeExpr(t->children[0]);
    Type right = typeExpr(t->children[2]);
    if (left == Type::INT && right == Type::INT) {
      return t->type = Type::INT;
    }
    else {
      cerr << "ERROR mismatched * / % operands" << endl;
      return Type::NOTYPE;
    }
  }
  // factor -> & lvalue
  else if (t->ruleNum == 40) {
    Type type = typeExpr(t->children[1]);
    if (type == Type::INT) {
      return t->type = Type::PTR;
    }
    else {
      cerr << "ERROR getting address of ptr" << endl;
      return Type::NOTYPE;
    } 
  }
  // factor -> * factor
  else if (t->ruleNum == 41) {
    Type type = typeExpr(t->children[1]);
    if (type == Type::PTR) {
      return t->type = Type::INT;
    }
    else {
      cerr << "ERROR dereferencing an int" << endl;
      return Type::NOTYPE;
    } 
  }
  // new
  else if (t->ruleNum == 42) {
    Type type = typeExpr(t->children[3]);
    if (type == Type::INT) {
      return t->type = Type::PTR;
    }
    else {
      cerr << "ERROR bad arg to new" << endl;
      return Type::NOTYPE;
    } 
  }
  // lvalue -> * factor
  else if (t->ruleNum == 49) {
    Type type = typeExpr(t->children[1]);
    if (type == Type::PTR) {
      return t->type = Type::INT;
    }
    else {
      cerr << "ERROR bad use of lvalue -> * factor" << endl;
      return Type::NOTYPE;
    } 
  }
  // lvalue -> (lvalue)
  else if (t->ruleNum == 50) {
    return t->type = typeExpr(t->children[1]);
  }
  return Type::NOTYPE;
}
list<Type> getArglist(Tree* t) {
  if (t->ruleNum == 46) {
    return {typeExpr(t->children[0])};
  }
  else if (t->ruleNum == 47) {
    auto temp = getArglist(t->children[2]);
    temp.push_front(typeExpr(t->children[0]));
    return temp;
  }
  cerr << "ERROR arglist bad" << endl;
  return {};
}
void addTypes(Tree* t) {
  typeExpr(t);
  for (auto& i : t->children) {
    addTypes(i);
  }
}
bool isError(Tree* t) {
  // if in wain
  if (t->ruleNum == 4) {
    // if two arguments have same name
    if (t->children[3]->children[1]->token.lexeme == t->children[5]->children[1]->token.lexeme) {
      cerr << "ERROR both arguments to wain had the same name" << endl;
      return true;
    }
    // if second arg has pointer type
    if (t->children[5]->children[1]->type != Type::INT) {
      cerr << "ERROR second arg to wain had pointer type" << endl;
      return true;
    }
    // return type is not int
    if (t->children[11]->type != Type::INT) {
      cout << t->children[11]->type << endl;
      cerr << "ERROR return type of wain was not int" << endl;
      return true;
    }
  }
  // if factor->ID
  // if (t->ruleNum == 36) {
  //   if (wainTable.count(t->children[0]->token.lexeme) == 0) {
  //     cerr << "ERROR ID not declared yet : " << t->children[0]->token.lexeme << endl;
  //     printSymbolTable(wainTable);
  //     return true;
  //   }
  // }
  // dcl initializing to NUM
  if (t->ruleNum == 12) {
    if (t->children[1]->children.front()->ruleNum != 9) {
      cerr << "ERROR initializing pointer to num" << endl;
      return true;
    }
  }
  // dcl initializing to NULL
  if (t->ruleNum == 13) {
    if (t->children[1]->children.front()->ruleNum != 10) {
      cerr << "ERROR initializing num to pointer" << endl;
      return true;
    }
  }
  if (t->ruleNum == 17) {
    if (t->children[0]->type != t->children[2]->type) {
      cerr << "ERROR mismatch assignment" << endl;
      return true;
    }
  }
  if (t->ruleNum == 20) {
    if (t->children[2]->type != Type::INT) {
      cerr << "ERROR not right type for println" << endl;
      return true;
    }
  }
  if (t->ruleNum == 21) {
    if (t->children[2]->type != Type::INT) {
      cerr << "ERROR not right type for putchar" << endl;
      return true;
    }
  }
  if (t->ruleNum == 22) {
    if (t->children[3]->type != Type::PTR) {
      cerr << "ERROR must delete pointers" << endl;
      return true;
    }
  }
  if (t->ruleNum >= 23 && t->ruleNum <=28 ) {
    if (t->children[0]->type != t->children[2]->type) {
      cerr << "ERROR mismatched types for test" << endl;
      return true;
    } 
  }
  bool error = false;
  for (auto& i : t->children) {
    error = error || isError(i);
  }
  return error;
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
  // printGrammar(grammar);
  buildTree(parseTree);
  addTypes(parseTree);
  //printSymbolTable(wainTable);
  if (!isError(parseTree)) printTree(parseTree);
  //printTree(parseTree);
  delete parseTree;
}


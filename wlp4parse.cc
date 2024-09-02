#include <climits>
#include <cstddef>
#include <iostream>
#include <list>
#include <stack>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_set>
#include <map>

#include "wlp4data.h"

using namespace std;

struct Token {
  string kind;
  string lexeme;
};

struct Tree {
  string nonTerminal;
  Token token;
  int ruleNum = 0;
  list<Tree*> children;
  ~Tree() {
    for (auto child : children) {
      delete child;
    }
  }
};

unordered_set<string> components {".CFG",".INPUT",".ACTIONS",".END",".TRANSITIONS",".REDUCTIONS"};
vector<pair<string, vector<string>>> grammar;
list<Tree*> input;
list<Tree*> reductionSeq;
map<pair<int, string>, int> slrdfa;
map<pair<int, string>, int> reductionMap;

void printGrammar(vector<pair<string, vector<string>>> g) {
  for (auto rule : g) {
    cout << rule.first << " : ";
    for (auto rhs : rule.second) {
      cout << " " << rhs << " ";
    }
    cout << endl;
  }
}

void printTree(Tree* t) {
  if (t->children.size() == 0) {
    cout << t->token.kind << " " << t->token.lexeme << endl;
    return;
  }
  cout << grammar[t->ruleNum].first << " ";
  for (auto i : grammar[t->ruleNum].second) {
    cout << i << " ";
  }
  cout << endl;
  for (auto i=t->children.begin() ; i != t->children.end(); ++i) {
    printTree(*i);
  }
}

void shift() {
  reductionSeq.push_back(input.front());
  if (input.size() > 0) input.pop_front();
}
void reduce(int n) {
  int size = grammar[n].second.size();
  if (size == 1 && grammar[n].second[0] == ".EMPTY") {
    Tree* emptyLeaf = new Tree{};
    emptyLeaf->token.kind = grammar[n].first;
    emptyLeaf->token.lexeme = ".EMPTY";
    reductionSeq.push_back(emptyLeaf);
    return;
  }
  list<Tree*> leaves;
  for (int i = 0;  i < size; ++i) {
    leaves.push_front(reductionSeq.back());
    reductionSeq.pop_back();
  }
  Tree* nonLeaf = new Tree;
  nonLeaf->children = leaves;
  nonLeaf->ruleNum = n;
  reductionSeq.push_back(nonLeaf);
}



stringstream wlp4in {WLP4_COMBINED};

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
          grammar[ruleNum].second.emplace_back(item);
        }
        ++ruleNum;
      }
    }
    if (curline == ".TRANSITIONS") {
      while (getline(wlp4in, curline)) {
        if (components.count(curline) > 0) break;
        stringstream ss {curline};
        int state, newstate;
        string token;
        ss >> state >> token >> newstate;
        slrdfa[{state, token}] = newstate;
      }
    }
    if (curline == ".REDUCTIONS") {
      while (getline(wlp4in, curline)) {
        if (components.count(curline) > 0) break;
        stringstream ss {curline};
        string lookahead;
        int state, prodrule;
        ss >> state >> prodrule >> lookahead;
        reductionMap[{state,lookahead}] = prodrule;
      }
    }
  }
  Tree* bof = new Tree{};
  bof->token.kind = "BOF";
  bof->token.lexeme = "BOF";
  input.push_back(bof);
  while (getline(cin, curline)) {
    stringstream ss {curline};
    string kind, lexeme;
    ss >> kind >> lexeme;
    Tree* leaf =  new Tree{};
    leaf->token.kind = kind;
    leaf->token.lexeme = lexeme;
    input.push_back(leaf);
  }
  Tree* eof = new Tree{};
  eof->token.kind = "EOF";
  eof->token.lexeme = "EOF";
  input.push_back(eof);
  
  stack<int> stateStack;
  stateStack.push(0);
  list copyInput = input;
  int reads = 0;
  for (auto &a : copyInput) {
    while (reductionMap.count({stateStack.top(), a->token.kind}) > 0) {
      int rule = reductionMap[{stateStack.top(), a->token.kind}];
      reduce(rule);
      if (grammar[rule].second.size() != 1 || grammar[rule].second[0] != ".EMPTY") {
        for (size_t i = 0; i < grammar[rule].second.size(); ++i) {
          stateStack.pop();
        }
      }
      stateStack.push(slrdfa[{stateStack.top(), grammar[rule].first}]);
    }
    shift();
    ++reads;
    if (slrdfa.count({stateStack.top(),a->token.kind}) == 0) {
      cerr << "ERROR at " << reads - 1 << endl;
      for (auto i : reductionSeq) {
        delete i;
      }
      for (auto i : input) {
        delete i;
      }
      return 0;
    }
    stateStack.push(slrdfa[{stateStack.top(),a->token.kind}]);
  }
  reduce(0);

  printTree(reductionSeq.back());

  for (auto i : reductionSeq) {
    delete i;
  }
  for (auto i : input) {
    delete i;
  }
}



#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <map>

using namespace std;

const std::string ALPHABET    = ".ALPHABET";
const std::string STATES      = ".STATES";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string INPUT       = ".INPUT";
const std::string EMPTY       = ".EMPTY";

unordered_set<char> alphabet;
unordered_set<string> states;
unordered_set<string> acceptStates;
string initialState;
map<pair<string,char>, string> transitions;

bool isChar(std::string s) {
  return s.length() == 1;
}
bool isRange(std::string s) {
  return s.length() == 3 && s[1] == '-';
}

string wlp4dfa = R"(.ALPHABET
a-z A-Z 0-9 / ; + - = & * % ! , { } [ ] ( ) < > .SPACE .NEWLINE 
.STATES
start id! zero! num! slash! startcomment! comment! newline! space!
lparen! rparen! lbrace! rbrace! lbrac! rbrac! plus! minus! amp! star! becomes! equals!
bang ne! lt! gt! le! ge! comma! semi! mod!
.TRANSITIONS
start a-z A-Z id
id a-z A-Z 0-9 id
start 0 zero
start 1-9 num
num 0-9 num
start / slash
slash / startcomment
start .SPACE space
start ( lparen
start ) rparen
start { lbrace
start } rbrace
start [ lbrac
start ] rbrac
start + plus
start - minus
start & amp
start * star
start = becomes
becomes = equals
start ! bang
bang = ne
start < lt
start > gt
lt = le
gt = ge
start , comma
start ; semi
start % mod
.INPUT
)";


int main() {
  string tempS;
  while (getline(cin, tempS)) {
    wlp4dfa += tempS;
  }
  
  
  std::istringstream iss{wlp4dfa};
  std::istream& in = iss;
  std::string s;

  std::getline(in, s); // Alphabet section (skip header)
  // Read characters or ranges separated by whitespace
  while(in >> s) {
    if (s == STATES) { 
      break; 
    } else {
      if (isChar(s)) {
        //// Variable 's[0]' is an alphabet symbol
        alphabet.insert(s[0]);
      } else if (isRange(s)) {
        for(char c = s[0]; c <= s[2]; ++c) {
          //// Variable 'c' is an alphabet symbol
          alphabet.insert(c);
        }
      } 
    }
  }

  std::getline(in, s); // States section (skip header)
  // Read states separated by whitespace
  while(in >> s) {
    if (s == TRANSITIONS) { 
      break; 
    } else {
      static bool initial = true;
      bool accepting = false;
      if (s.back() == '!' && !isChar(s)) {
        accepting = true;
        s.pop_back();
      }
      //// Variable 's' contains the name of a state
      states.insert(s);
      if (initial) {
        //// The state is initial
        initialState = s;
        initial = false;
      }
      if (accepting) {
        //// The state is accepting
        acceptStates.insert(s);
      }
    }
  }

  std::getline(in, s); // Transitions section (skip header)
  // Read transitions line-by-line
  while(std::getline(in, s)) {
    if (s == INPUT) { 
      // Note: Since we're reading line by line, once we encounter the
      // input header, we will already be on the line after the header
      break; 
    } else {
      std::string fromState, symbols, toState;
      std::istringstream line(s);
      std::vector<std::string> lineVec;
      while(line >> s) {
        lineVec.push_back(s);
      }
      fromState = lineVec.front();
      toState = lineVec.back();
      for(size_t i = 1; i < lineVec.size()-1; ++i) {
        std::string s = lineVec[i];
        if (isChar(s)) {
          symbols += s;
        } else if (isRange(s)) {
          for(char c = s[0]; c <= s[2]; ++c) {
            symbols += c;
          }
        } else if (s == ".SPACE") {
          symbols += ' ';
        }
      }
      for ( char c : symbols ) {
        //// There is a transition from 'fromState' to 'toState' on 'c'
        transitions[{fromState,c}] = toState;
      }
    }
  }

  // Input section (already skipped header)
  bool errorState = false;
  while (getline(in,s)) {
    //// Variable 's' contains all of input
    string curState = initialState;
    string curToken = "";
    for (size_t i = 0; i < s.length();) {
      char c = s[i];
      if (transitions.count({curState,c}) > 0) {
        curToken += c;
        curState = transitions[{curState,c}];
        ++i;
      } else { // no arrow out
        if (acceptStates.count(curState) > 0) {
          cout << curToken << endl;
          curState = initialState;
          curToken = "";
        } else {
          cerr << "ERROR here" << endl;
          errorState = true;
          break;
        } 
      }
    }
    if (acceptStates.count(curState) > 0 && curToken != " ") {
      cout << curToken << endl;
    } else if (!errorState){
      cerr << "ERROR" << endl;
    }
  }
}

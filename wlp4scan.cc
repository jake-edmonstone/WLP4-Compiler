#include <iostream>
#include <string>
#include <unordered_set>

using namespace std;

int main() {
  unordered_set<string> digits = {"0","1","2","3","4","5","6","7","8","9"};
  string line;
  while (getline(cin, line)) {
    if (line.substr(0,2) == "//") continue;
    else if (line == ".NEWLINE") continue;
    else if (line == ".SPACE") continue;
    else if (line == " " || line == "") continue;
    else if (line == ".ERROR") {
      cerr << "ERROR" << endl;
      break;
    }
    else if (digits.count(line.substr(0,1)) > 0) { // is numeric
      double num = stod(line);
      if (num > 2147483647) {
        cerr << "ERROR" << endl;
        break;
      } else {
        cout << "NUM " << line << endl;
      }
    }
    else if (line == "(") cout << "LPAREN " << line << endl;
    else if (line == ")") cout << "RPAREN " << line << endl;
    else if (line == "{") cout << "LBRACE " << line << endl;
    else if (line == "}") cout << "RBRACE " << line << endl;
    else if (line == "return") cout << "RETURN " << line << endl;
    else if (line == "if") cout << "IF " << line << endl;
    else if (line == "else") cout << "ELSE " << line << endl;
    else if (line == "while") cout << "WHILE " << line << endl;
    else if (line == "println") cout << "PRINTLN " << line << endl;
    else if (line == "putchar") cout << "PUTCHAR " << line << endl;
    else if (line == "getchar") cout << "GETCHAR " << line << endl;
    else if (line == "wain") cout << "WAIN " << line << endl;
    else if (line == "=") cout << "BECOMES " << line << endl;
    else if (line == "int") cout << "INT " << line << endl;
    else if (line == "==") cout << "EQ " << line << endl;
    else if (line == "!=") cout << "NE " << line << endl;
    else if (line == "<") cout << "LT " << line << endl;
    else if (line == ">") cout << "GT " << line << endl;
    else if (line == "<=") cout << "LE " << line << endl;
    else if (line == ">=") cout << "GE " << line << endl;
    else if (line == "+") cout << "PLUS " << line << endl;
    else if (line == "-") cout << "MINUS " << line << endl;
    else if (line == "*") cout << "STAR " << line << endl;
    else if (line == "/") cout << "SLASH " << line << endl;
    else if (line == "%") cout << "PCT " << line << endl;
    else if (line == ",") cout << "COMMA " << line << endl;
    else if (line == ";") cout << "SEMI " << line << endl;
    else if (line == "new") cout << "NEW " << line << endl;
    else if (line == "delete") cout << "DELETE " << line << endl;
    else if (line == "[") cout << "LBRACK " << line << endl;
    else if (line == "]") cout << "RBRACK " << line << endl;
    else if (line == "&") cout << "AMP " << line << endl;
    else if (line == "NULL") cout << "NULL " << line << endl;
    else cout << "ID "<< line << endl;
  }
}

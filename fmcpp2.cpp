#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <iostream>
#include <locale>
#include <ranges>
#include <string>
#include <vector>
namespace Color {
  enum Code { FG_RED = 31, FG_GREEN = 32, FG_BLUE = 34, FG_DEFAULT = 39, BG_RED = 41, BG_GREEN = 42, BG_BLUE = 44, BG_DEFAULT = 49 };
  class Modifier {
    Code code;

  public:
    Modifier(Code pCode) : code(pCode) {}
    friend std::ostream &operator<<(std::ostream &os, const Modifier &mod) { return os << "\033[" << mod.code << "m"; }
  };
}  // namespace Color

std::array<char, 11> operator_chars{'+', '-', '*', '/', '>', '=', '|', '-', '<', '!', ','};
// trim from start (in place)
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

std::string remove_extra_whitespaces(const std::string &input) {
  std::string output;
  std::unique_copy(input.begin(), input.end(), std::back_insert_iterator<std::string>(output), [](char a, char b) { return isspace(a) && isspace(b); });
  return output;
}

bool is_oper(char c) { return std::find(operator_chars.begin(), operator_chars.end(), c) != operator_chars.end(); }
bool is_eq(char c) { return c == '='; }
bool is_col(char c) { return c == ':'; }
bool is_plus(char c) { return c == '+'; }
bool is_minus(char c) { return c == '-'; }
bool is_dollar(char c) { return c == '$'; }
bool is_semicol(char c) { return c == ';'; }
bool is_comma(char c) { return c == ','; }
bool is_Lparen(char c) { return c == '('; }
bool is_Rparen(char c) { return c == ')'; }
bool is_Lbrace(char c) { return c == '{'; }
bool is_Rbrace(char c) { return c == '}'; }
bool is_dash(char c) { return c == '-'; }
bool is_Rbracket(char c) { return c == '>'; }
bool is_Lbracket(char c) { return c == '<'; }
bool is_Rsqbracket(char c) { return c == ']'; }
bool is_Lsqbracket(char c) { return c == '['; }
bool is_naming_char(char c) { return std::isalnum(c) || c == '_'; }

std::size_t replace_all(std::string &inout, std::string_view what, std::string_view with) {
  std::size_t count{};
  for (std::string::size_type pos{}; inout.npos != (pos = inout.find(what.data(), pos, what.length())); pos += with.length(), ++count)
    inout.replace(pos, what.length(), with.data(), with.length());
  return count;
}
void apply_col_rules(std::string &str, std::string::size_type &n) {
  // CHECK BEFORE
  if (n > 3 && n + 2 != std::string::npos) {
    // SPECIAL CASE :() LAMBDA in function call
    if (is_comma(str[n - 1]) || is_Lparen(str[n - 1]) && !std::isspace(str[n + 1])) return;
    if (is_comma(str[n - 1]) || is_Lparen(str[n - 1]) && std::isspace(str[n + 1])) {
      str.erase(n + 1, 1);
      --n;
      return;
    }
  }
  if (n > 1) {
    // REMOVE SPACE
    if (std::isspace(str[n - 1])) {
      str.erase(n - 1, 1);
      --n;
    }
  }

  // CHECK after
  if (n + 1 != std::string::npos) {
    // REMOVE SPACE AFTER
    if (n + 2 != std::string::npos && std::isspace(str[n + 1]) && is_Lparen(str[n + 2]) && n > 2 && !is_naming_char(str[n - 1])) {
      str.erase(n + 1, 1);
      --n;
    }  // ADD SPACE AFTER
    else if (n > 1 && (is_Rparen(str[n - 1]) || is_comma(str[n - 1])) && !std::isspace(str[n + 1])) {
      str.insert(n + 1, 1, ' ');
      ++n;
    }
    // ADD SPACE AFTER
    if (n > 2 && !is_col(str[n - 1]) && (is_naming_char(str[n + 1]) || !std::isspace(str[n + 1]))) {
      str.insert(n + 1, 1, ' ');
      ++n;
    }
  }
}
void apply_eq_rules(std::string &str, std::string::size_type &n) {
  if (n > 1) {
    // ADD SPACE BEFORE
    if (!std::isspace(str[n - 1]) && !is_oper(str[n - 1]) && !is_col(str[n - 1])) {
      str.insert(n, 1, ' ');
      ++n;
    }
  }
  // ADD SPACE AFTER
  if (n + 1 != std::string::npos) {
    if (!std::isspace(str[n + 1]) && !is_eq(str[n + 1])) {
      str.insert(n + 1, 1, ' ');
      ++n;
    }
    // REMOVE SPACE BEFORE
    if (n > 2 && std::isspace(str[n - 1]) && (is_oper(str[n - 1]) || is_col(str[n - 2]))) {
      str.erase(n - 1, 1);
      --n;
    }
  }
}
void apply_Lparen_rules(std::string &str, std::string::size_type &n) {
  if (n > 1) {
    // REMOVE SPACE BEFORE
    if (n > 3 && std::isspace(str[n - 1]) && is_naming_char(str[n - 2]) && str[n - 2] != 'o' && str[n - 3] != 'd' && !is_naming_char(str[n - 2])) {
      str.erase(n - 1, 1);
      --n;
    }
    // INSERT BEFORE
    if (!std::isspace(str[n - 1]) && (is_oper(str[n - 1]) || (n > 3 && str[n - 1] == 'o' && str[n - 2] == 'd'))) {
      str.insert(n, 1, ' ');
      ++n;
    }
  }
  // REMOVE SPACE AFTER
  if (n + 1 != std::string::npos) {
    if (std::isspace(str[n + 1])) {
      str.erase(n + 1, 1);
      --n;
    }
  }
}
void apply_Rparen_rules(std::string &str, std::string::size_type &n) {
  if (n + 1 != std::string::npos) {
    // ADD SPACE AFTER
    if (!std::isspace(str[n + 1]) && !is_Rparen(str[n + 1]) && !is_semicol(str[n + 1]) && !is_dollar(str[n + 1])) {
      str.insert(n + 1, 1, ' ');
      ++n;
    }
  }
  if (n > 1) {
    // REMOVE SPACE Before
    if (std::isspace(str[n - 1])) {
      str.erase(n - 1, 1);
      --n;
    }
  }
}
void apply_semicol_rules(std::string &str, std::string::size_type &n) {
  if (n > 1) {
    // REMOVE SPACE BEFORE
    if (std::isspace(str[n - 1])) {
      str.erase(n - 1, 1);
      --n;
    }
  }
}
void apply_Rbracket_rules(std::string &str, std::string::size_type &n) {
  if (n > 2) {
    // REMOVE SPACE BEFORE
    if (std::isspace(str[n - 1]) && is_oper(str[n - 2])) {
      str.erase(n - 1, 1);
      --n;
    }
    // INSERT SPACE BEFORE
    if (!std::isspace(str[n - 1]) && !is_oper(str[n - 1])) {
      str.insert(n, 1, ' ');
      ++n;
    }
  }
  // ADD SPACE AFTER
  if (n + 1 != std::string::npos) {
    if (!std::isspace(str[n + 1]) && !is_oper(str[n + 1])) {
      str.insert(n + 1, 1, ' ');
      ++n;
    }
  }
}
void apply_Lbracket_rules(std::string &str, std::string::size_type &n) {
  if (n > 2) {
    // REMOVE SPACE BEFORE
    if (std::isspace(str[n - 1]) && is_oper(str[n - 2])) {
      str.erase(n - 1, 1);
      --n;
    }
    // INSERT SPACE BEFORE
    if (!std::isspace(str[n - 1]) && !is_oper(str[n - 1])) {
      str.insert(n, 1, ' ');
      ++n;
    }
  }
  // ADD SPACE AFTER
  if (n + 1 != std::string::npos) {
    if (!std::isspace(str[n + 1]) && !is_oper(str[n + 1])) {
      str.insert(n + 1, 1, ' ');
      ++n;
    }
  }
}
void apply_comma_rules(std::string &str, std::string::size_type &n) {
  if (n > 1) {
    // REMOVE SPACE BEFORE
    if (std::isspace(str[n - 1])) {
      str.erase(n - 1, 1);
      --n;
    }
  }
  // ADD SPACE AFTER
  if (n + 1 != std::string::npos) {
    if (!std::isspace(str[n + 1])) {
      str.insert(n + 1, 1, ' ');
      ++n;
    }
  }
}
void apply_lefthand_oper_rules(std::string &str, std::string::size_type &n) {
  if (n > 2) {
    // ADD SPACE BEFORE
    if (!std::isspace(str[n - 1]) && !is_oper(str[n - 1]) && n + 1 != std::string::npos && !is_oper(str[n + 1])) {
      str.insert(n, 1, ' ');
      ++n;
    }
  }
  // ADD SPACE AFTER
  if (n + 1 != std::string::npos) {
    if (!std::isspace(str[n + 1]) && !is_oper(str[n + 1])) {
      str.insert(n + 1, 1, ' ');
      ++n;
    }
  }
}
void apply_Rsqbracket_rules(std::string &str, std::string::size_type &n) {
  if (n > 1) {
    // REMOVE SPACE BEFORE
    if (std::isspace(str[n - 1])) {
      str.erase(n - 1, 1);
      --n;
    }
  }
  // REMOVE SPACE AFTER
  if (n + 2 != std::string::npos) {
    if (std::isspace(str[n + 1]) && is_Lsqbracket(str[n + 2])) {
      str.erase(n + 1, 1);
      --n;
    }
  }
}
void apply_Lsqbracket_rules(std::string &str, std::string::size_type &n) {
  if (n > 2) {
    // REMOVE SPACE BEFORE
    if (std::isspace(str[n - 1]) && is_naming_char(str[n - 1])) {
      str.erase(n - 1, 1);
      --n;
    }
  }
  // REMOVE SPACE AFTER
  if (n + 1 != std::string::npos) {
    if (std::isspace(str[n + 1])) {
      str.erase(n + 1, 1);
      --n;
    }
  }
}

void apply_Lbrace_rules(std::string &str, std::string::size_type &n) {
  if (n + 1 != std::string::npos) {
    // INSERT SPACE AFTER
    if (!std::isspace(str[n + 1])) {
      str.insert(n + 1, 1, ' ');
      ++n;
    }
  }
}
void apply_Rbrace_rules(std::string &str, std::string::size_type &n) {
  if (n - 1 != std::string::npos) {
    // INSERT SPACE BEFORE
    if (!std::isspace(str[n - 1])) {
      str.insert(n, 1, ' ');
      ++n;
    }
  }
}
void fix_line(std::string &str) {
  std::string::size_type n = 0;
  for (;; ++n) {
    n = str.find_first_of(":=();><,+-[]{}", n);
    if (n != std::string::npos) {
      if (is_col(str[n])) apply_col_rules(str, n);
      if (is_eq(str[n])) apply_eq_rules(str, n);
      if (is_Lparen(str[n])) apply_Lparen_rules(str, n);
      if (is_Rparen(str[n])) apply_Rparen_rules(str, n);
      if (is_semicol(str[n])) apply_semicol_rules(str, n);
      if (is_Rbracket(str[n])) apply_Rbracket_rules(str, n);
      if (is_Lbracket(str[n])) apply_Lbracket_rules(str, n);
      if (is_comma(str[n])) apply_comma_rules(str, n);
      if (is_plus(str[n]) || is_minus(str[n])) apply_lefthand_oper_rules(str, n);
      if (is_Lsqbracket(str[n])) apply_Lsqbracket_rules(str, n);
      if (is_Rsqbracket(str[n])) apply_Rsqbracket_rules(str, n);
      if (is_Rbrace(str[n])) apply_Rbrace_rules(str, n);
      if (is_Lbrace(str[n])) apply_Lbrace_rules(str, n);
    } else {
      break;
    }
  }
  replace_all(str, "operator =", "operator=");
}
struct Line {
  explicit Line(int ind, std::string str) : txt{std::move(str)}, indent{ind} {
    before = txt;
    fix_line(txt);
  };
  int indent{0};
  std::string txt;
  std::string before;

  int braceDiff() {
    int cnt_RBrace{0};
    int cnt_LBrace{0};
    for (auto &&c : txt) {
      if (is_Rbrace(c)) cnt_RBrace++;
      if (is_Lbrace(c)) cnt_LBrace++;
    }
    return cnt_RBrace - cnt_LBrace;
  }
  std::string indTxt() { return std::string(indent * 4, ' ') + txt; }
  void printBeforeAfter(int lineno = -1) {
    Color::Modifier red(Color::FG_RED);
    Color::Modifier green(Color::FG_GREEN);
    std::cout << red << ((lineno != -1) ? std::to_string(lineno) : "") << " " << std::string(indent * 4, '|') + before << std::endl;
    std::cout << green << ((lineno != -1) ? std::to_string(lineno) : "") << " " << indTxt() << std::endl;
  }
  bool empty() const { return txt.empty(); }
};

int main(int argc, const char *argv[]) {
  const std::string filename = argv[1];
  std::fstream fs(filename);
  std::vector<Line> lns;
  int ind = 0;
  for (std::string line; getline(fs, line);) {
    ltrim(line);
    rtrim(line);
    line = remove_extra_whitespaces(line);

    if (lns.size() > 1 && lns.back().empty() && line.empty()) {
      continue;
    }
    lns.push_back(Line(ind, line));
    if (line.rfind('}') != std::string::npos && lns.back().braceDiff() != 0) {
      --ind;
    }
    lns.back().indent = ind;
    if (line.rfind('{') != std::string::npos && lns.back().braceDiff() != 0) {
      ++ind;
    }
    if (fs.peek() == EOF) break;
  }
  fs.clear();
  fs.close();
  std::ofstream of(filename, std::ios::trunc);
  for (auto i = 0; auto &ln : lns) {
    // ln.printBeforeAfter(i);
    of << ln.indTxt() << std::endl;
    // ++i;
  }
  of.close();
  

  return 0;
}

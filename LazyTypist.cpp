#include <iostream>
#include <map>
#include <utility>
#include <string>
#include <vector>
#include <sstream>
#include <stack>


// position of letters
const int NO_MOVE = -1;
const int SPACE1 = -2;
const int SPACE2 = -3;
const int SPACE3 = -4;
const int SPACE4 = -5;
const int SPACE5 = -6;
const int SHIFT1 = -7;
const int SHIFT2 = -8;

const char SPACE_CHAR = ' ';
const char SHIFT_CHAR = '^';

struct state {
   enum Hands { LEFT, RIGHT, NONE };

   state(int l = NO_MOVE, int r = NO_MOVE, 
         Hands h = NONE, int e = 0) :
      left(l), right(r), lastMoved(h), effort(e) {}

   int left, right;
   Hands lastMoved;
   int effort = 0;
};

struct Compare {
    bool operator()(state a, state b) {
      return a.left < b.left || (a.left == b.left && a.right < b.right);
    }
};

typedef std::map<state, state, Compare> stateMap;
typedef std::pair<int,int> position;

const std::map<char, position> LETTER_POS = [&] {
   std::map<char, position> map;
   std::string str1 = "QWERTYUIOP";
   std::string str2 = "ASDFGHJKL";
   std::string str3 = "ZXCVBNM";
   for (int i = 0; i<str1.length(); i++)
      map[str1[i]] = position(0,i);
   for (int i = 0; i<str2.length(); i++)
      map[str2[i]] = position(1,i);
   for (int i = 0; i<str3.length(); i++)
      map[str3[i]] = position(2,i+1);
   return map;
}();

const std::map<int, position> SPACE_POS =
   { {SPACE1, position(3,3)}, {SPACE2, position(3,4)}, {SPACE3, position(3,5)},
     {SPACE4, position(3,6)}, {SPACE5, position(3,7)} };
const std::map<int, position> SHIFT_POS =
   { {SHIFT1, position(2,0)}, {SHIFT2, position(2,9)} };

// helper functions
bool isLowerCase(char c1) { return c1 >= 'a' && c1 <= 'z'; }
bool isUpperCase(char c1) { return c1 >= 'A' && c1 <= 'Z'; }
bool isLetter(char c1) { return isLowerCase(c1) || isUpperCase(c1); }
bool isSpace(char c1) { return c1 == SPACE_CHAR; }
bool isSpaceIndex(int i1) { return SPACE_POS.count(i1) != 0; }
bool isShift(char c1) { return c1 == SHIFT_CHAR; }
bool isShiftIndex(int i1) { return SHIFT_POS.count(i1) != 0; }

position getPosition(int  i0) {
   if (i0 >= 0 && LETTER_POS.count(char(i0)) > 0) 
      return LETTER_POS.at(char(i0));
   if (isSpaceIndex(i0)) return SPACE_POS.at(i0);
   if (isShiftIndex(i0)) return SHIFT_POS.at(i0);
   return position(0,0);
}

int distance(int i0, int i1) {
   if (i0 == NO_MOVE || i1 == NO_MOVE) return 0;
   position p0 = getPosition(i0), p1 = getPosition(i1);
   return std::abs(p0.first - p1.first) + std::abs(p0.second - p1.second);
}

int findClosestSpaceIndex(int i0) {
   position p0 = getPosition(i0);
   int i_min = 0, min = INT_MAX;
   for (auto m : SPACE_POS) {
      int d = std::abs(m.second.second - p0.second);
      if (d < min) { min = d; i_min = m.first; }
   }
   return i_min;
}

// Tokenizer
std::string tokenize(const std::string& str) {
   std::stringstream sstr;
   for (auto it = str.begin(); it != str.end(); ++it) {
      if (isLowerCase(*it))
         sstr << char(*it - 'a' + 'A');
      else if (isUpperCase(*it)) 
         sstr << SHIFT_CHAR << *it;
      else if (isSpace(*it))
         sstr << *it;
   }
   std::string outstr;
   std::getline(sstr, outstr);
   return outstr;
};

std::string getString(int c) {
   if (isSpaceIndex(c)) return "Space";
   if (isLetter(char(c))) return std::string(1,char(c));
   if (isShiftIndex(c)) return "Shift";
   return "";
};

std::string printLine(state s0, state s1) {
   std::stringstream sstr;
   int i0, i1;
   std::string handString;
   if (s1.lastMoved == state::LEFT) {
      i0 = s0.left, i1 = s1.left;
      handString = "left hand";
   }
   else if (s1.lastMoved == state::RIGHT) {
      i0 = s0.right, i1 = s1.right;
      handString = "right hand";
   }
   else return "";

   sstr << getString(i1) << ": ";
   if (i0 == NO_MOVE ) {
      sstr << "Use " << handString;
   }
   else if (i0 == i1) { 
      sstr << "Use " << handString << " again";
   }
   else {
      sstr << "Move " << handString << " from " 
           << getString(i0) << " (effort: " 
           << (s1.effort - s0.effort) << ")";
   }
   sstr << std::endl;
   std::string totalString;
   std::getline(sstr, totalString);
   return totalString;
}
 

void printOutput(const std::string &str, const state & sMin,
      std::vector<stateMap> & previousStates) {

   std::stack<std::string> stack;
   state s0 = sMin, s1;

   for (int index = str.length(); index > 0; --index) {
      s1 = s0; 
      s0 = previousStates[index][s0];
      stack.push(printLine(s0, s1));
   }   
   while (!stack.empty()) {
      std::cout << stack.top() << std::endl;
      stack.pop();
   }

   std::cout << "Total effort: " << sMin.effort;
};

void updateMap(stateMap & currentStates, const state & s0, char newChar) {
   int i1, effort;
   std::vector<state> s1Vec;
   if (isSpace(newChar)) {
      // left
      i1 = findClosestSpaceIndex(s0.left);
      effort = s0.effort + distance(s0.left, i1); 
      s1Vec.push_back( state(i1, s0.right, state::LEFT, effort));
      // right
      i1 = findClosestSpaceIndex(s0.right);
      effort = s0.effort + distance(s0.right, i1); 
      s1Vec.push_back( state(s0.left, i1, state::RIGHT, effort));
   }
   else if (isShift(newChar)) {
      for ( auto m : SHIFT_POS ) {
         i1 = m.first;
         //left
         effort = s0.effort + distance(s0.left, i1);
         s1Vec.push_back( state(i1, s0.right, state::LEFT, effort));
         // right
         effort = s0.effort + distance(s0.right, i1); 
         s1Vec.push_back( state(s0.left, i1, state::RIGHT, effort));
      }
   }
   else if (isLetter(newChar)) {
      i1 = newChar;
      //left
      if (!isShiftIndex(s0.left) || s0.lastMoved != state::LEFT) {
         effort = s0.effort + distance(s0.left, i1);
         s1Vec.push_back( state(i1, s0.right, state::LEFT, effort));
      }
      // right
      if (!isShiftIndex(s0.right) || s0.lastMoved != state::RIGHT) {
         effort = s0.effort + distance(s0.right, i1); 
         s1Vec.push_back( state(s0.left, i1, state::RIGHT, effort));
      }
   }
   for (state s1 : s1Vec) {
      if (!currentStates.count(s1)) {
         currentStates[s1] = s0;
      } else if (currentStates.find(s1)->first.effort > s1.effort) {
         currentStates.erase(s1);
         currentStates[s1] = s0;
      }
   }
}

int main() {

   std::vector<stateMap> states;

   std::cout << "Enter sentence: " << std::endl;
   std::string sentence;
   std::getline(std::cin, sentence);
   sentence = tokenize(sentence); 

   state initState;
   states.push_back(stateMap());
   states[0][initState] = initState;

   for (char c : sentence) {
      states.push_back(stateMap());
      stateMap & map = states[states.size()-2];
      stateMap & newMap = states[states.size()-1];

      for (auto m : map ) {
         updateMap(newMap, m.first, c);
      }
   }
   //find min
   int minEffort = INT_MAX;
   state minState;
   for (auto m : states.back()) {
      if (m.first.effort < minEffort) {
         minState = m.first;
         minEffort = minState.effort;
      }
   } 
   std::cout << std::endl;
   printOutput(sentence, minState, states);
}


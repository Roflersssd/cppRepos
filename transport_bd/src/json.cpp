#include "json.h"

#include <iostream>

using namespace std;

namespace Json {

Document::Document(Node root) : root(move(root)) {}

const Node& Document::GetRoot() const {
  return root;
}

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
  vector<Node> result;

  for (char c; input >> c && c != ']';) {
    if (c != ',') {
      input.putback(c);
    }
    result.push_back(LoadNode(input));
  }

  return Node(move(result));
}

Node LoadBool(istream& input) {
  string result;
  while (isalpha(input.peek()))
    result += tolower(input.get());

  if (result == "true")
    return true;
  else
    return false;
}

Node LoadDouble(istream& input) {
  string result;
  while (isdigit(input.peek()) || input.peek() == '.' || input.peek() == '-') {
    result += input.get();
  }

  return Node(stod(result));
}

Node LoadValue(istream& input) {
  if (isdigit(input.peek()) || input.peek() == '-')
    return LoadDouble(input);
  else
    return LoadBool(input);
}

Node LoadString(istream& input) {
  string line;
  getline(input, line, '"');
  return Node(move(line));
}

Node LoadDict(istream& input) {
  map<string, Node> result;

  for (char c; input >> c && c != '}';) {
    if (c == ',') {
      input >> c;
    }

    string key = LoadString(input).AsString();
    input >> c;
    result.emplace(move(key), LoadNode(input));
  }

  return Node(move(result));
}

Node LoadNode(istream& input) {
  char c;
  input >> c;
  if (c == '[') {
    return LoadArray(input);
  } else if (c == '{') {
    return LoadDict(input);
  } else if (c == '"') {
    return LoadString(input);
  } else {
    input.putback(c);
    return LoadValue(input);
  }
}

Document Load(istream& input) {
  return Document{LoadNode(input)};
}

}  // namespace Json

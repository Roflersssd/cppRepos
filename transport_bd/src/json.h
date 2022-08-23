#pragma once

#include <istream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace Json {

class Node : std::variant<std::vector<Node>,
                          std::map<std::string, Node>,
                          std::string,
                          double,
                          bool> {
 public:
  using variant::variant;

  const auto& AsArray() const { return std::get<std::vector<Node>>(*this); }
  const auto& AsMap() const {
    return std::get<std::map<std::string, Node>>(*this);
  }
  int AsInt() const { return static_cast<int>(std::get<double>(*this)); }
  double AsDouble() const { return std::get<double>(*this); }
  int AsBool() const { return std::get<bool>(*this); }
  const auto& AsString() const { return std::get<std::string>(*this); }
};

class Document {
 public:
  explicit Document(Node root);

  const Node& GetRoot() const;

 private:
  Node root;
};

Document Load(std::istream& input);

}  // namespace Json

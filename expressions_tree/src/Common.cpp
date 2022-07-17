#include "../headers/Common.h"
#include <stdexcept>

class Number : public Expression {
public:
  Number(int i_value) : value(i_value) {}
  int Evaluate() const override { return value; }
  std::string ToString() const override { return std::to_string(value); }

private:
  int value;
};

class Operation : public Expression {

public:
  Operation(ExpressionPtr l, char opr, ExpressionPtr r)
      : left(std::move(l)), right(std::move(r)), operation(opr) {}
  int Evaluate() const override {
    if (operation == '*') {
      return left->Evaluate() * right->Evaluate();
    } else if (operation == '+') {
      return left->Evaluate() + right->Evaluate();
    } else {
      throw std::logic_error("Unknown opeartion");
    }
  }
  std::string ToString() const override {
    return "(" + left->ToString() + ")" + operation + "(" + right->ToString() +
           ")";
  }

private:
  char operation;
  ExpressionPtr left;
  ExpressionPtr right;
};

ExpressionPtr Value(int value) {
  return std::move(std::make_unique<Number>(value));
}

ExpressionPtr Sum(ExpressionPtr left, ExpressionPtr right) {
  ExpressionPtr expression =
      std::make_unique<Operation>(std::move(left), '+', std::move(right));
  return std::move(expression);
}

ExpressionPtr Product(ExpressionPtr left, ExpressionPtr right) {
  ExpressionPtr expression =
      std::make_unique<Operation>(std::move(left), '*', std::move(right));
  return std::move(expression);
}
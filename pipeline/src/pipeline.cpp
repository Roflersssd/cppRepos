#include "../../test_runner.h"
#include <functional>
#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

struct Email {
  string from;
  string to;
  string body;
};

class Worker {
protected:
  unique_ptr<Worker> nextWorker;

public:
  virtual ~Worker() = default;
  virtual void Process(unique_ptr<Email> email) = 0;
  virtual void Run() {
    // только первому worker-у в пайплайне нужно это имплементировать
    throw logic_error("Unimplemented");
  }

protected:
  // реализации должны вызывать PassOn, чтобы передать объект дальше
  // по цепочке обработчиков
  void PassOn(unique_ptr<Email> email) const {
    if (nextWorker)
      nextWorker->Process(move(email));
  }

public:
  void SetNext(unique_ptr<Worker> next) { nextWorker = move(next); }
};

class Reader : public Worker {
  istream &input;

public:
  Reader(istream &in) : input(in) {}
  void Process(unique_ptr<Email> email) override { PassOn(move(email)); }
  void Run() override {
    std::string from, to, body;
    while (getline(input, from)) {
      getline(input, to);
      getline(input, body);
      unique_ptr<Email> email = make_unique<Email>();
      email->from = from;
      email->to = to;
      email->body = body;
      Process(move(email));
    }
  }
};

class Filter : public Worker {
public:
  using Function = function<bool(const Email &)>;

public:
  Filter(const Function &i_func) : func(i_func) {}
  void Process(unique_ptr<Email> email) override {
    if (func(*email))
      PassOn(move(email));
  }

private:
  Function func;
};

class Copier : public Worker {
  string recipient;

public:
  Copier(const string &i_recipient) : recipient(i_recipient) {}
  void Process(unique_ptr<Email> email) override {
    if (recipient != email->to) {
      unique_ptr<Email> recipEmail = make_unique<Email>();
      recipEmail->from = email->from;
      recipEmail->to = recipient;
      recipEmail->body = email->body;
      PassOn(move(email));
      PassOn(move(recipEmail));
    } else
      PassOn(move(email));
  }
};

class Sender : public Worker {
  ostream &out;

public:
  Sender(ostream &i_out) : out(i_out){};
  void Process(unique_ptr<Email> email) override {
    out << email->from << endl;
    out << email->to << endl;
    out << email->body << endl;
    PassOn(move(email));
  }
};

// реализуйте класс
class PipelineBuilder {
  unique_ptr<Worker> start;
  unique_ptr<Worker> next;
  list<unique_ptr<Worker>> workers;

public:
  // добавляет в качестве первого обработчика Reader
  explicit PipelineBuilder(istream &in) { start = make_unique<Reader>(in); }

  // добавляет новый обработчик Filter
  PipelineBuilder &FilterBy(Filter::Function filter) {
    workers.push_front(make_unique<Filter>(filter));
    return *this;
  }

  // добавляет новый обработчик Copier
  PipelineBuilder &CopyTo(string recipient) {
    workers.push_front(make_unique<Copier>(recipient));
    return *this;
  }

  // добавляет новый обработчик Sender
  PipelineBuilder &Send(ostream &out) {
    workers.push_front(make_unique<Sender>(out));
    return *this;
  }

  // возвращает готовую цепочку обработчиков
  unique_ptr<Worker> Build() {
    unique_ptr<Worker> prev;
    for (auto &worker : workers) {
      if (!prev) {
        prev = move(worker);
        continue;
      }
      worker->SetNext(move(prev));
      prev = move(worker);
    }
    start->SetNext(move(prev));
    return move(start);
  }
};

void TestSanity() {
  string input = ("erich@example.com\n"
                  "richard@example.com\n"
                  "Hello there\n"

                  "erich@example.com\n"
                  "ralph@example.com\n"
                  "Are you sure you pressed the right button?\n"

                  "ralph@example.com\n"
                  "erich@example.com\n"
                  "I do not make mistakes of that kind\n");
  istringstream inStream(input);
  ostringstream outStream;

  PipelineBuilder builder(inStream);
  builder.FilterBy(
      [](const Email &email) { return email.from == "erich@example.com"; });
  builder.CopyTo("richard@example.com");
  builder.Send(outStream);
  auto pipeline = builder.Build();

  pipeline->Run();

  string expectedOutput = ("erich@example.com\n"
                           "richard@example.com\n"
                           "Hello there\n"

                           "erich@example.com\n"
                           "ralph@example.com\n"
                           "Are you sure you pressed the right button?\n"

                           "erich@example.com\n"
                           "richard@example.com\n"
                           "Are you sure you pressed the right button?\n");

  ASSERT_EQUAL(expectedOutput, outStream.str());
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestSanity);
  return 0;
}

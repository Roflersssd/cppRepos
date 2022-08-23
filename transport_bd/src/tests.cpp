#include "tests.h"
#include "json.h"

#include <fstream>

using namespace std;

namespace {

void TestJsonParser() {
  string file = "/home/ilya/dev/cppRepos/transport_bd/src/requests.json";

  ifstream input(file);
  if (input.is_open()) {
    cout << "File is opened" << endl;
  }
  const auto respones = processJson(input);
  PrintResponses(respones);
}

}  // namespace

namespace TransportTests {

void RunTests() {
  TestRunner tr;
  RUN_TEST(tr, TestJsonParser);
}

}  // namespace TransportTests
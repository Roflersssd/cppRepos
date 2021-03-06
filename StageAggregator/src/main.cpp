#include "../../test_runner.h"
#include "../headers/stats_aggregator.h"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

void TestAll();

unique_ptr<StatsAggregator> ReadAggregators(istream& input) {
  const unordered_map<string, std::function<unique_ptr<StatsAggregator>()>>
      known_builders = {
          {"sum", [] { return make_unique<StatsAggregators::Sum>(); }},
          {"min", [] { return make_unique<StatsAggregators::Min>(); }},
          {"max", [] { return make_unique<StatsAggregators::Max>(); }},
          {"avg", [] { return make_unique<StatsAggregators::Average>(); }},
          {"mode", [] { return make_unique<StatsAggregators::Mode>(); }}};

  auto result = make_unique<StatsAggregators::Composite>();

  int aggr_count;
  input >> aggr_count;

  string line;
  for (int i = 0; i < aggr_count; ++i) {
    input >> line;
    result->Add(known_builders.at(line)());
  }

  return result;
}

int main() {
  TestAll();
  return 0;
}

void TestAll() {
  TestRunner tr;
  RUN_TEST(tr, StatsAggregators::TestSum);
  RUN_TEST(tr, StatsAggregators::TestMin);
  RUN_TEST(tr, StatsAggregators::TestMax);
  RUN_TEST(tr, StatsAggregators::TestAverage);
  RUN_TEST(tr, StatsAggregators::TestMode);
  RUN_TEST(tr, StatsAggregators::TestComposite);
}

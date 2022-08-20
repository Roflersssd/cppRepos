#include "test_runner.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <vector>

using namespace std;

const double PI = 3.1415926535;
const int EARTH_R = 6371;

struct Stop {
  string name;
  double lat;
  double lon;

  bool operator==(const Stop& other) const { return name == other.name; }
};

double degToRad(double deg) {
  return deg * (PI / 180);
}

double calculateDistance(const Stop& lhs, const Stop& rhs) {
  return acos(sin(degToRad(lhs.lat)) * sin(degToRad(rhs.lat)) +
              cos(degToRad(lhs.lat)) * cos(degToRad(rhs.lat)) *
                  cos(abs(degToRad(lhs.lon - rhs.lon)))) *
         6371000;
}

class Bus {
 public:
  Bus() : uniqueStops(0), isCircle_(false) {}

  void addStop(const string& stop) {
    if (find(begin(stops_), end(stops_), stop) == end(stops_))
      ++uniqueStops;
    stops_.push_back(stop);
  }

  Bus& setNumber(string number) {
    number_ = number;
    return *this;
  }

  Bus& setIsCircle(bool isCircle) {
    isCircle_ = isCircle;
    return *this;
  }

  double getUniqueStopsNumber() const { return uniqueStops; }

  size_t getStopsNumber() const {
    if (isCircle_)
      return stops_.size() * 2 - 1;
    else {
      return stops_.size();
    }
  }

  bool getIsCircle() const { return isCircle_; }

  string getNumber() const { return number_; }

  const vector<string>& getStops() const { return stops_; }

 private:
  bool isCircle_;
  string number_;
  int uniqueStops;
  vector<string> stops_;
};

string_view trim(string_view str, const std::string& whitespace = " \t") {
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string::npos)
    return "";  // no content

  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

pair<string_view, optional<string_view>> SplitTwoStrict(
    string_view s,
    string_view delimiter = " ") {
  const size_t pos = s.find(delimiter);
  if (pos == s.npos) {
    return {s, nullopt};
  } else {
    return {s.substr(0, pos), s.substr(pos + delimiter.length())};
  }
}

pair<string_view, string_view> SplitTwo(string_view s,
                                        string_view delimiter = " ") {
  const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
  return {lhs, rhs_opt.value_or("")};
}

string_view ReadToken(string_view& s, string_view delimiter = " ") {
  const auto [lhs, rhs] = SplitTwo(s, delimiter);
  s = rhs;
  return trim(lhs);
}

double ConvertToDouble(string_view str) {
  // use std::from_chars when available to git rid of string copy
  size_t pos;
  const double result = stod(string(str), &pos);
  if (pos != str.length()) {
    std::stringstream error;
    error << "string " << str << " contains " << (str.length() - pos)
          << " trailing chars";
    throw invalid_argument(error.str());
  }
  return result;
}

template <typename Number>
Number ReadNumberOnLine(istream& stream) {
  Number number;
  stream >> number;
  string dummy;
  getline(stream, dummy);
  return number;
}

double calculateRouteDist(const Bus& bus,
                          const unordered_map<string, Stop>& stopsInfo) {
  const auto& stops = bus.getStops();
  double dist = 0;

  for (int i = 0; i < stops.size() - 1; ++i) {
    dist +=
        calculateDistance(stopsInfo.at(stops[i]), stopsInfo.at(stops[i + 1]));
  }

  if (bus.getIsCircle())
    dist *= 2;

  return dist;
}

class BusManager {
 public:
  void addBus(const Bus& bus) { buses.emplace(bus.getNumber(), bus); }

  string getBusInfo(string busNumber) const {
    ostringstream info;
    info.precision(6);
    info << "Bus " << busNumber << ": ";
    if (buses.find(busNumber) == end(buses)) {
      info << "not found";
      return info.str();
    }

    const Bus& bus = buses.at(busNumber);
    info << bus.getStopsNumber() << " stops on route, ";
    info << bus.getUniqueStopsNumber() << " unique stops, ";
    info << calculateRouteDist(bus, allStops) << " route length";

    return info.str();
  }

  void addStop(const Stop& stop) { allStops[stop.name] = stop; }

 private:
  unordered_map<string, Bus> buses;
  unordered_map<string, Stop> allStops;
};

Bus readBus(string_view& busRequest) {
  Bus bus;
  string number(ReadToken(busRequest, ":"));

  const bool isCircle = busRequest.find("-") != busRequest.npos;
  bus.setIsCircle(isCircle);
  const string stopDelim = isCircle ? "-" : ">";

  bus.setNumber(number);
  vector<string> stops;
  for (auto stopName = ReadToken(busRequest, stopDelim); !stopName.empty();
       stopName = ReadToken(busRequest, stopDelim))
    bus.addStop(string(stopName));

  return bus;
}

Stop readStop(string_view& stopRequst) {
  string name = string(ReadToken(stopRequst, ":"));
  const auto lat = ConvertToDouble(ReadToken(stopRequst, ","));
  const auto lon = ConvertToDouble(ReadToken(stopRequst, "\n"));
  return {name, lat, lon};
}

BusManager readBusManager(istream& in_stream = cin) {
  const size_t request_count = ReadNumberOnLine<size_t>(in_stream);

  BusManager manager;

  for (size_t i = 0; i < request_count; ++i) {
    string request_str;
    getline(in_stream, request_str);
    auto [type, requestBody] = SplitTwo(request_str);
    if (type == "Bus")
      manager.addBus(readBus(requestBody));
    else if (type == "Stop")
      manager.addStop(readStop(requestBody));
    else
      throw std::runtime_error("Invalid requst type");
  }
  return manager;
}

vector<string> processRequests(const BusManager& manager,
                               istream& in_stream = cin) {
  vector<string> res;
  const size_t request_count = ReadNumberOnLine<size_t>(in_stream);
  for (size_t i = 0; i < request_count; ++i) {
    string request_str;
    getline(in_stream, request_str);
    auto [type, requestBody] = SplitTwo(request_str);
    if (type == "Bus") {
      string number(requestBody);
      res.push_back(manager.getBusInfo(number));
    }
  }

  return res;
}

void PrintResponses(const vector<string>& responses, ostream& stream = cout) {
  stream.precision(25);
  for (const string& response : responses) {
    stream << response << endl;
  }
}

void TestCommonRequest() {
  string busInfo =
      "10\n"
      "Stop Tolstopaltsevo: 55.611087, 37.20829\n"
      "Stop Marushkino: 55.595884, 37.209755\n"
      "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo "
      "Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
      "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
      "Stop Rasskazovka: 55.632761, 37.333324\n"
      "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517\n"
      "Stop Biryusinka: 55.581065, 37.64839\n"
      "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n"
      "Stop Universam: 55.587655, 37.645687\n"
      "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n";

  istringstream in(busInfo);
  BusManager manager = readBusManager(in);

  string request =
      "3\n"
      "Bus 256\n"
      "Bus 750\n"
      "Bus 751\n";
  vector<string> expectedResult = {
      "Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length",
      "Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length",
      "Bus 751: not found"};
  istringstream rin(request);
  const auto result = processRequests(manager, rin);
  ASSERT_EQUAL(result, expectedResult)
}

int main() {
  // TestRunner tr;
  // RUN_TEST(tr, TestCommonRequest);
  BusManager manager = readBusManager();
  const auto responses = processRequests(manager);
  PrintResponses(responses);

  return 0;
}
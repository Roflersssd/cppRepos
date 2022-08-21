#include "../../test_runner.h"

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

class Stop {
 public:
  Stop() {}
  Stop(string name, double lat, double lon)
      : name_(name), lat_(lat), lon_(lon) {}

  string getName() const { return name_; }
  double getLat() const { return lat_; }
  double getLon() const { return lon_; }

  optional<double> getDistance(const string& stopName) const {
    if (distanceToOtherStops.find(stopName) == end(distanceToOtherStops))
      return nullopt;
    else {
      return distanceToOtherStops.at(stopName);
    }
  }

  void addDistance(const string& otherStopName, double distance) {
    distanceToOtherStops.insert({otherStopName, distance});
  }

 private:
  string name_;
  double lat_;
  double lon_;
  unordered_map<string, double> distanceToOtherStops;
};

double degToRad(double deg) {
  return deg * (PI / 180);
}

double calculStraightDist(const Stop& lhs, const Stop& rhs) {
  return acos(sin(degToRad(lhs.getLat())) * sin(degToRad(rhs.getLat())) +
              cos(degToRad(lhs.getLat())) * cos(degToRad(rhs.getLat())) *
                  cos(abs(degToRad(lhs.getLon() - rhs.getLon())))) *
         6371000;
}

double calculGivenDist(const Stop& lhs, const Stop& rhs) {
  auto distance = lhs.getDistance(rhs.getName());
  if (!distance)
    distance = rhs.getDistance(lhs.getName());
  return *distance;
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

struct StopInfo {
  Stop stop;
  set<string> passingBuses;
};

using SoptsInfo = unordered_map<string, StopInfo>;

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

template <typename Func>
double calculateRouteDist(const Bus& bus,
                          const SoptsInfo& stopsInfo,
                          Func calculateFunc) {
  const auto& stops = bus.getStops();
  double dist = 0;

  for (int i = 0; i < stops.size() - 1; ++i)
    dist += calculateFunc(stopsInfo.at(stops[i]).stop,
                          stopsInfo.at(stops[i + 1]).stop);

  if (bus.getIsCircle())
    for (int i = stops.size() - 1; i > 0; --i)
      dist += calculateFunc(stopsInfo.at(stops[i]).stop,
                            stopsInfo.at(stops[i - 1]).stop);

  return dist;
}

class BusManager {
 public:
  void addBus(const Bus& bus) {
    buses.emplace(bus.getNumber(), bus);
    for (const auto& stop : bus.getStops())
      allStops[stop].passingBuses.insert(bus.getNumber());
  }

  string getBusInfo(const string& busNumber) const {
    ostringstream info;
    info.precision(6);
    info << "Bus " << busNumber << ": ";
    if (buses.find(busNumber) == end(buses)) {
      info << "not found";
      return info.str();
    }

    const Bus& bus = buses.at(busNumber);

    const double gDist = calculateRouteDist(bus, allStops, calculGivenDist);
    const double curvature =
        gDist / calculateRouteDist(bus, allStops, calculStraightDist);

    info << bus.getStopsNumber() << " stops on route, ";
    info << bus.getUniqueStopsNumber() << " unique stops, ";
    info << gDist << " route length, ";
    info << std::setprecision(7) << curvature;
    info << " curvature";

    return info.str();
  }

  string getStopInfo(const string& stopName) const {
    ostringstream info;
    info.precision(6);
    info << "Stop " << stopName << ": ";
    if (allStops.find(stopName) == end(allStops)) {
      info << "not found";
      return info.str();
    }

    const set<string>& passingBuses = allStops.at(stopName).passingBuses;
    if (passingBuses.empty())
      info << "no buses";
    else {
      info << "buses";
      for (const auto& bus : passingBuses)
        info << " " << bus;
    }

    return info.str();
  }

  void addStop(const Stop& stop) { allStops[stop.getName()].stop = stop; }

 private:
  unordered_map<string, Bus> buses;
  SoptsInfo allStops;
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
  const auto lon = ConvertToDouble(ReadToken(stopRequst, ","));
  Stop stop(name, lat, lon);
  for (auto distanceStr = ReadToken(stopRequst, "m to "); !distanceStr.empty();
       distanceStr = ReadToken(stopRequst, "m to ")) {
    string otherStopName(ReadToken(stopRequst, ","));
    double distance = ConvertToDouble(distanceStr);
    stop.addDistance(otherStopName, distance);
  }
  return stop;
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
    string name(requestBody);

    if (type == "Bus")
      res.push_back(manager.getBusInfo(name));
    else if (type == "Stop")
      res.push_back(manager.getStopInfo(name));
  }

  return res;
}

void PrintResponses(const vector<string>& responses, ostream& stream = cout) {
  stream.precision(25);
  for (const string& response : responses) {
    stream << response << endl;
  }
}

void TestPartARequest() {
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

void TestPartBRequest() {
  string busInfo =
      "13\n"
      "Stop Tolstopaltsevo: 55.611087, 37.20829\n"
      "Stop Marushkino: 55.595884, 37.209755\n"
      "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo "
      "Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
      "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
      "Stop Rasskazovka: 55.632761, 37.333324\n"
      "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517\n"
      "Stop Biryusinka: 55.581065, 37.64839\n"
      "Stop Universam: 55.587655, 37.645687\n"
      "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n"
      "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n"
      "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > "
      "Biryulyovo Zapadnoye\n"
      "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"
      "Stop Prazhskaya: 55.611678, 37.603831\n";

  istringstream in(busInfo);
  BusManager manager = readBusManager(in);

  string request =
      "6\n"
      "Bus 256\n"
      "Bus 750\n"
      "Bus 751\n"
      "Stop Samara\n"
      "Stop Prazhskaya\n"
      "Stop Biryulyovo Zapadnoye\n";
  vector<string> expectedResult = {
      "Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length",
      "Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length",
      "Bus 751: not found",
      "Stop Samara: not found",
      "Stop Prazhskaya: no buses",
      "Stop Biryulyovo Zapadnoye: buses 256 828"};
  istringstream rin(request);
  const auto result = processRequests(manager, rin);
  ASSERT_EQUAL(result, expectedResult)
}

void TestPartCRequest() {
  string busInfo =
      "13\n"
      "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n"
      "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka\n"
      "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo "
      "Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
      "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
      "Stop Rasskazovka: 55.632761, 37.333324\n"
      "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya "
      "ulitsa, 1800m to Biryusinka, 2400m to Universam\n"
      "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n"
      "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, "
      "900m to Biryulyovo Tovarnaya\n"
      "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo "
      "Passazhirskaya\n"
      "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to "
      "Biryulyovo Zapadnoye\n"
      "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > "
      "Biryulyovo Zapadnoye\n"
      "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"
      "Stop Prazhskaya: 55.611678, 37.603831\n";

  istringstream in(busInfo);
  BusManager manager = readBusManager(in);

  string request =
      "6\n"
      "Bus 256\n"
      "Bus 750\n"
      "Bus 751\n"
      "Stop Samara\n"
      "Stop Prazhskaya\n"
      "Stop Biryulyovo Zapadnoye\n";
  vector<string> expectedResult = {
      "Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.361239 "
      "curvature",
      "Bus 750: 5 stops on route, 3 unique stops, 27600 route length, 1.318084 "
      "curvature",
      "Bus 751: not found",
      "Stop Samara: not found",
      "Stop Prazhskaya: no buses",
      "Stop Biryulyovo Zapadnoye: buses 256 828"};

  istringstream rin(request);
  const auto result = processRequests(manager, rin);

  ASSERT_EQUAL(result, expectedResult)
}

void Test() {
  TestRunner tr;
  // RUN_TEST(tr, TestPartARequest);
  // RUN_TEST(tr, TestPartBRequest);
  RUN_TEST(tr, TestPartCRequest);
}

int main() {
  Test();
  BusManager manager = readBusManager();
  const auto responses = processRequests(manager);
  PrintResponses(responses);

  return 0;
}
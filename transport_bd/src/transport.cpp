#include "transport.h"
#include "json.h"
//#include "tests.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <iomanip>
#include <memory>
#include <sstream>

using namespace std;

namespace {

const double PI = 3.1415926535;
const int EARTH_R = 6371;

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

Bus toBus(const std::map<std::string, Json::Node>& busMap) {
  Bus bus;
  bus.setNumber(busMap.at("name").AsString());
  bus.setIsCircle(!busMap.at("is_roundtrip").AsBool());
  const auto stops = busMap.at("stops").AsArray();
  for (const auto& stop : stops)
    bus.addStop(stop.AsString());
  return bus;
}

Stop toStop(const std::map<std::string, Json::Node>& stopMap) {
  Stop stop(stopMap.at("name").AsString(), stopMap.at("latitude").AsDouble(),
            stopMap.at("longitude").AsDouble());
  const auto& roadDistances = stopMap.at("road_distances").AsMap();
  for (const auto& [name, distance] : roadDistances)
    stop.addDistance(name, distance.AsInt());

  return stop;
}

}  // namespace

Stop::Stop(string name, double lat, double lon)
    : name_(name), lat_(lat), lon_(lon) {}

string Stop::getName() const {
  return name_;
}
double Stop::getLat() const {
  return lat_;
}
double Stop::getLon() const {
  return lon_;
}

optional<double> Stop::getDistance(const string& stopName) const {
  if (distanceToOtherStops.find(stopName) == end(distanceToOtherStops))
    return nullopt;
  else {
    return distanceToOtherStops.at(stopName);
  }
}

void Stop::addDistance(const string& otherStopName, double distance) {
  distanceToOtherStops.insert({otherStopName, distance});
}

Bus::Bus() : uniqueStops(0), isCircle_(false) {}

void Bus::addStop(const string& stop) {
  if (find(begin(stops_), end(stops_), stop) == end(stops_))
    ++uniqueStops;
  stops_.push_back(stop);
}

Bus& Bus::setNumber(string number) {
  number_ = number;
  return *this;
}

Bus& Bus::setIsCircle(bool isCircle) {
  isCircle_ = isCircle;
  return *this;
}

double Bus::getUniqueStopsNumber() const {
  return uniqueStops;
}

size_t Bus::getStopsNumber() const {
  if (isCircle_)
    return stops_.size() * 2 - 1;
  else {
    return stops_.size();
  }
}

bool Bus::getIsCircle() const {
  return isCircle_;
}

string Bus::getNumber() const {
  return number_;
}

const vector<string>& Bus::getStops() const {
  return stops_;
}

void BusManager::addBus(const Bus& bus) {
  buses.emplace(bus.getNumber(), bus);
  for (const auto& stop : bus.getStops())
    allStops[stop].passingBuses.insert(bus.getNumber());
}

string BusManager::getBusInfo(const string& busNumber, int requestId) const {
  ostringstream info;
  info.precision(6);
  info << "{" << endl;
  info << "\"request_id\": " << requestId << ",\n";
  if (buses.find(busNumber) == end(buses)) {
    info << "\"error_message\": \"not found\"\n";
    info << "}";
    return info.str();
  }

  const Bus& bus = buses.at(busNumber);

  const double gDist = calculateRouteDist(bus, allStops, calculGivenDist);
  const double curvature =
      gDist / calculateRouteDist(bus, allStops, calculStraightDist);

  info << "\"stop_count\": " << bus.getStopsNumber() << ",\n";
  info << "\"unique_stop_count\": " << bus.getUniqueStopsNumber() << ",\n";
  info << "\"route_length\": " << gDist << ",\n";
  info << "\"curvature\": " << curvature << endl;
  info << "}";

  return info.str();
}

string BusManager::getStopInfo(const string& stopName, int requestId) const {
  ostringstream info;
  info.precision(6);
  info << "{" << endl;
  info << "\"request_id\": " << requestId << ",\n";
  if (allStops.find(stopName) == end(allStops)) {
    info << "\"error_message\": \"not found\"\n";
    info << "}";
    return info.str();
  }

  info << "\"buses\": [\n";

  const set<string>& passingBuses = allStops.at(stopName).passingBuses;
  bool isFirst = true;
  for (const auto& bus : passingBuses) {
    if (!isFirst)
      info << ",\n";
    else
      isFirst = false;
    info << '"' << bus << '"';
  }
  info << "\n]\n}";
  return info.str();
}

void BusManager::addStop(const Stop& stop) {
  allStops[stop.getName()].stop = stop;
}

BusManager readBusManagerFromJson(const Json::Node& root) {
  const auto& requests = root.AsMap().at("base_requests").AsArray();
  BusManager manager;

  for (const auto& request : requests) {
    const auto& requestMap = request.AsMap();
    const string type = requestMap.at("type").AsString();
    if (type == "Bus")
      manager.addBus(toBus(requestMap));
    else if (type == "Stop")
      manager.addStop(toStop(requestMap));
    else
      throw std::runtime_error("Invalid requst type");
  }
  return manager;
}

vector<string> processRequestsFromJson(const BusManager& manager,
                                       const Json::Node& root) {
  const auto& requests = root.AsMap().at("stat_requests").AsArray();
  vector<string> res;

  for (const auto& request : requests) {
    const auto& requestMap = request.AsMap();
    const string type = requestMap.at("type").AsString();
    const string name = requestMap.at("name").AsString();
    const int id = requestMap.at("id").AsInt();
    if (type == "Bus")
      res.push_back(manager.getBusInfo(name, id));
    else if (type == "Stop")
      res.push_back(manager.getStopInfo(name, id));
  }

  return res;
}

vector<string> processJson(istream& input) {
  const auto root = Json::Load(input).GetRoot();
  BusManager manager = readBusManagerFromJson(root);
  return processRequestsFromJson(manager, root);
}

void PrintResponses(const vector<string>& responses, ostream& stream) {
  stream << "[" << endl;
  bool isFirst = true;
  for (const string& response : responses) {
    if (!isFirst)
      stream << ",\n";
    else
      isFirst = false;
    stream << response;
  }
  stream << "\n]" << endl;
}

int main() {
  // TransportTests::RunTests();
  const auto respones = processJson();
  PrintResponses(respones);
  return 0;
}
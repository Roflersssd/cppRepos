#pragma once

#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class Stop {
 public:
  Stop() {}
  Stop(std::string name, double lat, double lon);

  std::string getName() const;
  double getLat() const;
  double getLon() const;

  std::optional<double> getDistance(const std::string& stopName) const;

  void addDistance(const std::string& otherStopName, double distance);

 private:
  std::string name_;
  double lat_;
  double lon_;
  std::unordered_map<std::string, double> distanceToOtherStops;
};

class Bus {
 public:
  Bus();

  void addStop(const std::string& stop);

  Bus& setNumber(std::string number);

  Bus& setIsCircle(bool isCircle);

  double getUniqueStopsNumber() const;

  size_t getStopsNumber() const;

  bool getIsCircle() const;

  std::string getNumber() const;

  const std::vector<std::string>& getStops() const;

 private:
  bool isCircle_;
  std::string number_;
  int uniqueStops;
  std::vector<std::string> stops_;
};

struct StopInfo {
  Stop stop;
  std::set<std::string> passingBuses;
};

using SoptsInfo = std::unordered_map<std::string, StopInfo>;

class BusManager {
 public:
  void addBus(const Bus& bus);

  std::string getBusInfo(const std::string& busNumber, int requestId) const;

  std::string getStopInfo(const std::string& stopName, int requestId) const;

  void addStop(const Stop& stop);

 private:
  std::unordered_map<std::string, Bus> buses;
  SoptsInfo allStops;
};

void PrintResponses(const std::vector<std::string>& responses,
                    std::ostream& stream = std::cout);

std::vector<std::string> processJson(std::istream& input = std::cin);
#include <cstdint>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <utility>
#include <iostream>
#include <limits>
#include <chrono>

#include <osmium/io/any_input.hpp>
#include <osmium/visitor.hpp>
#include <osmium/geom/haversine.hpp>
#include <osmium/geom/coordinates.hpp>
#include <osmium/handler.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/all.hpp>

#define LOG(X) std::cout << (X) << std::endl
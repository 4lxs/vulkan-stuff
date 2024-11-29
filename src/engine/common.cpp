#include "common.hpp"

#include <memory>

#include "bootstrap.hpp"
#include "engine.hpp"

Engine& get_engine() {
  static auto engine = std::make_unique<Engine>();
  return *engine;
}

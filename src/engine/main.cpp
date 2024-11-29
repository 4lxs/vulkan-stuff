#include "engine.hpp"

int main() {
  Engine &engine = get_engine();

  engine.run();

  return 0;
}

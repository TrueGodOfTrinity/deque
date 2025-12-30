#include <exception>
#include <iostream>

void runApiBasicTests();
void runIteratorTests();
void runModifierTests();
void runCompareTests();
void runVsStdDequeTests();

int main() {
  try {
    runApiBasicTests();
    runIteratorTests();
    runModifierTests();
    runCompareTests();
    runVsStdDequeTests();
  } catch (const std::exception& ex) {
    std::cerr << "Test failed with exception: " << ex.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "Test failed with unknown exception.\n";
    return 1;
  }

  std::cout << "All tests passed.\n";
  return 0;
}

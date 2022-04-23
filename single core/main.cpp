#include <iostream>
#include "def.hpp"

using namespace std;

int main() {
    ResourceScheduler resScheduler;

    resScheduler.schedule();
    // cout << resScheduler.getFinishTimeDeviation() << '\n';
    resScheduler.printResultText();
}
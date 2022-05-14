#include <iostream>
#include "def.hpp"

using namespace std;

int main() {
    ResourceScheduler resScheduler;

    resScheduler.loadData("../../input/task1_case10.txt");
    resScheduler.scheduleSingleHostLPTOnly();
    resScheduler.printResultText();
    // // resScheduler.exportData();

    resScheduler.loadData("../../input/task1_case10.txt");
    resScheduler.scheduleSingleHostLPTWithBFMulticores();
    resScheduler.printResultText();

}

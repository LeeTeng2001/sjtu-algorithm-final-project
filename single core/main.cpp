#include "def.hpp"
#include "generator.hpp"

using namespace std;

int main() {
    ResourceScheduler resScheduler;

    resScheduler.loadData("../../input/task1_case10.txt");
    resScheduler.scheduleSingleHostLPTOnly();
    resScheduler.printResultText("SingleHost LPT");
    resScheduler.exportData("../../output/task1_case10-SingleHost-LPT.json");

    resScheduler.loadData("../../input/task1_case10.txt");
    resScheduler.scheduleSingleHostLPTWithBFMulticores();
    resScheduler.printResultText("SingleHost Weighted");
    resScheduler.exportData("../../output/task1_case10-SingleHost-MultiWeight.json");

}

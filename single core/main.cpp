#include "def.hpp"
#include "generator.hpp"
#include <random>

using namespace std;

int main() {
    ResourceScheduler resScheduler;

    // resScheduler.loadData("../../input/task1_case10.txt");
    // resScheduler.scheduleSingleHostLPTOnly();
    // resScheduler.printResultText("SingleHost LPT");
    // resScheduler.exportData("../../output/task1_case10-SingleHost-LPT.json");
    //
    // resScheduler.loadData("../../input/task1_case10.txt");
    // resScheduler.scheduleSingleHostLPTWithBFMulticores();
    // resScheduler.printResultText("SingleHost Weighted");
    // resScheduler.exportData("../../output/task1_case10-SingleHost-MultiWeight.json");

    // Benchmark
    for (int i = 0; i < 2; ++i) {
        GeneratorConfig config {1, 11, false, false};
        GeneratorInfo info {
            // host, job, alpha
            1, 8, 0.05,
            // range of core, block, size, job speed
            5, 6,
            1, 10,
            1, 10,
            10, 19,
        };


        ResourceScheduler testData;
        generateTestCases(testData, info, config);
        resScheduler = testData;
        resScheduler.scheduleSingleHostLPTOnly();
        resScheduler.printResultText("SingleHost LPT");
        resScheduler = testData;
        resScheduler.scheduleSingleHostLPTWithBFMulticores();
        resScheduler.printResultText("SingleHost Weighted");
    }
}

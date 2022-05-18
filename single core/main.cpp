#include "def.hpp"
#include "generator.hpp"

using std::cin;

constexpr bool interactiveBenchmark = false;

using namespace std;

int main() {
    ResourceScheduler resScheduler;

    // // ! Test by reading data from test file
    resScheduler.loadData("../../input/task1_case10.txt");
    resScheduler.scheduleSingleHostLPTOnly();
    resScheduler.printResultText("SingleHost LPT");
    // resScheduler.exportData("../../output/task1_case10-SingleHost-LPT.json");

    resScheduler.loadData("../../input/task1_case10.txt");
    resScheduler.scheduleSingleHostLPTWithBFMulticores();
    resScheduler.printResultText("SingleHost Weighted");
    // resScheduler.exportData("../../output/task1_case10-SingleHost-MultiWeight.json");

    // /// Test by generating data (small cases)
    // for (int i = 0; i < 2; ++i) {
    //     GeneratorConfig config {1, 11, false, false};
    //     GeneratorInfo info {
    //         // host, job, alpha
    //         1, 8, 0.05,
    //         // range of core, block, size, job speed
    //         5, 6,
    //         1, 10,
    //         1, 10,
    //         10, 19,
    //     };
    //
    //     ResourceScheduler testData;
    //     generateTestCases(testData, info, config);
    //     resScheduler = testData;
    //     resScheduler.scheduleSingleHostLPTOnly();
    //     resScheduler.printResultText("SingleHost LPT");
    //     resScheduler = testData;
    //     resScheduler.scheduleSingleHostLPTWithBFMulticores();
    //     resScheduler.printResultText("SingleHost Weighted");
    // }

    /// Test by generating data (large cases)
    // for (int i = 0; i < 10; ++i) {
    //     GeneratorConfig config {1, 200 + i, false, false};
    //     GeneratorInfo info {
    //         // host, job, alpha
    //         1, 15, 0.005,
    //         // range of core, block, size, job speed
    //         8, 15,
    //         1, 20,
    //         1, 30,
    //         10, 30,
    //     };
    //
    //     ResourceScheduler testData;
    //     generateTestCases(testData, info, config);
    //     resScheduler = testData;
    //     resScheduler.scheduleSingleHostLPTOnly();
    //     resScheduler.printResultText("SingleHost LPT");
    //     resScheduler = testData;
    //     resScheduler.scheduleSingleHostLPTWithBFMulticores();
    //     resScheduler.printResultText("SingleHost Weighted");
    //
    //     // Save test case if needed
    //     if (!interactiveBenchmark) continue;
    //     string inp;
    //     cout << "Save test data (y/n)? : ";
    //     cin >> inp;
    //     if (inp == "y") {
    //         cout << "Saving test data..." << '\n';
    //         config.writeToOutput = true;
    //         writeTestCaseToFile(testData, config);
    //         config.writeToOutput = false;
    //     }
    // }
}

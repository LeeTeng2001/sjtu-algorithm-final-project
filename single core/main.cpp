#include "def.hpp"
#include "generator.hpp"
#include <random>

using std::cin;

constexpr bool interactiveBenchmark = false;

using namespace std;

void benchmarkAlgorithm(int size);

int main() {
    ResourceScheduler resScheduler;

    benchmarkAlgorithm(1000);  // benchmark against 1000 randomised data

    // // ! Test by reading data from test file
    // resScheduler.loadData("../../input/task1_case10.txt");
    // resScheduler.scheduleSingleHostLPTOnly();
    // resScheduler.printResultText("SingleHost LPT");
    // // resScheduler.exportData("../../output/task1_case10-SingleHost-LPT.json");
    //
    // resScheduler.loadData("../../input/task1_case10.txt");
    // resScheduler.scheduleSingleHostLPTWithBFMulticores();
    // resScheduler.printResultText("SingleHost Weighted");
    // // resScheduler.exportData("../../output/task1_case10-SingleHost-MultiWeight.json");

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

    // /// Test by generating data (large cases)
    // for (int i = 0; i < 1000; ++i) {
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

struct BenchmarkReportWeighted {
    int totalWorstTimeThanLPT = 0;
    int totalWorstCPUUtilThanLPT = 0;
    double timeGain = 0;
    double cpuUtilGain = 0;
};

void benchmarkAlgorithm(int size) {
    ResourceScheduler resScheduler;
    BenchmarkReportWeighted benchmarkResult;

    // Random generator
    std::random_device dev;
    std::mt19937 rng(dev());
    // Create range generator, [a, b) for double, [a, b] for int
    std::uniform_int_distribution<int> gnJobs(10, 20);
    std::uniform_real_distribution<double> gnAlpha(0.05, 0.1);

    for (int i = 0; i < size; ++i) {
        GeneratorConfig config {1, size + i, false, false};
        GeneratorInfo info {
                // host, job, alpha
                1, 15, 0.075,
                // range of core, block, size, job speed
                4, 12,
                20, 80,
                10, 100,
                5, 40,
        };

        ResourceScheduler testData;
        generateTestCases(testData, info, config);
        resScheduler = testData;
        resScheduler.scheduleSingleHostLPTOnly();
        auto performanceLPT = resScheduler.evaluatePerformanceSingleHost();
        resScheduler = testData;
        resScheduler.scheduleSingleHostLPTWithBFMulticores();
        auto performanceWeighted = resScheduler.evaluatePerformanceSingleHost();

        // compare
        if (performanceWeighted.longestFinishTime > performanceLPT.longestFinishTime)
            benchmarkResult.totalWorstTimeThanLPT++;
        if (performanceWeighted.utilisationPercentage < performanceLPT.utilisationPercentage)
            benchmarkResult.totalWorstCPUUtilThanLPT++;

        benchmarkResult.timeGain += (performanceLPT.longestFinishTime - performanceWeighted.longestFinishTime) / size;
        benchmarkResult.cpuUtilGain += (performanceWeighted.utilisationPercentage - performanceLPT.utilisationPercentage) / size;
    }

    cout << "Total worst time than LPT: " << benchmarkResult.totalWorstTimeThanLPT << "/" << size << '\n';
    cout << "Total worst CPU than LPT: " << benchmarkResult.totalWorstCPUUtilThanLPT << "/" << size << '\n';
    cout << "Time gain: " << benchmarkResult.timeGain << '\n';
    cout << "CPU gain: " << benchmarkResult.cpuUtilGain << '\n';
}
#include "generator.hpp"
#include "def.hpp"

int main() {
    // type, caseID, write to output
    GeneratorConfig config {1, 11, false};
    ResourceScheduler rs;  // placeholder

    // TODO: Generate test cases in different conditions

    // 1
    GeneratorInfo g1 {
            // host, job, alpha
            1, 8, 0.05,
            // range of core, block, size, speed
            5, 6,
            1, 10,
            1, 10,
            10, 19,
    };
    generateTestCases(rs, g1, config);
}
#pragma once

#include <vector>
#include <iosfwd>
#include <string>

using std::vector;
using std::string;
using std::ifstream;

struct PerformanceReportSingleHost {
    double fragmentTimeInternal;
    double fragmentTimeEnd;
    double totalRealProcessingTime;
    double averageRealProcessingTime;
    double averageFragmentTime;
    double utilisationPercentage;

    double longestFinishTime;
    double finishTimeStd;
};

struct JobBlock {
    int jobId; // belongs to nth job
    int jobBlockId;
    double dataSize;  // data size

    bool operator<(const JobBlock &rhs) const {
        return dataSize < rhs.dataSize;
    };

    bool operator>(const JobBlock &rhs) const {
        return dataSize > rhs.dataSize;
    };

    [[nodiscard]] double executionTime(int totalCore, double speed, double alpha) const {
        return dataSize / speed / (1 - alpha * (totalCore - 1));
    }
};

struct Job {
    int id{};
    int totalBlocks{};  // amount of blocks
    double speed{};
    vector<JobBlock> blocks;

    // Extra info for single job sorting
    int totalSingleCoreExecTime = 0;

    bool operator>(const Job &rhs) const {
        return totalSingleCoreExecTime > rhs.totalSingleCoreExecTime;
    };

    bool operator<(const Job &rhs) const {
        return totalSingleCoreExecTime < rhs.totalSingleCoreExecTime;
    };

    void calculateSingleCoreExecTime() {
        for (const auto &item: blocks)
            totalSingleCoreExecTime += item.dataSize / speed;
    }
};

// Start and stop time for block in a core
struct BlockExecInfo {
    double startTime;
    double endTime;
};

struct Core {
    int coreId{};
    vector<JobBlock> blocks;  // block to be processed
    vector<BlockExecInfo> blockInfos;  // block process time

    [[nodiscard]] double getFinishTime() const {
        return blockInfos.empty() ? 0 : blockInfos.back().endTime;
    }

    bool operator<(const Core &rhs) const {
        return getFinishTime() < rhs.getFinishTime();
    }

    bool operator>(const Core &rhs) const {
        return getFinishTime() > rhs.getFinishTime();
    }

    void recalculateExecInfo() {
        for (int i = 1; i < blockInfos.size(); ++i) {
            double blockExecTime = blockInfos[i].endTime - blockInfos[i].startTime;
            blockInfos[i].startTime = blockInfos[i - 1].endTime;
            blockInfos[i].endTime = blockInfos[i].startTime + blockExecTime;
        }
    }
};

class ResourceScheduler {
private:
    int totalJob{};  // total number of job
    int totalHost{}; // total number of host
    double alpha{};  // alpha for speed decay constant
    vector<vector<Core>> hostCore;   // the jth core for ith host
    vector<Job> jobs;  // job infos

    // double getFinishTime();
    double getFinishTimeDeviation(const vector<Core> &cores);
    double getLongestFinishTime(const vector<Core> &cores);

    // Helper functions
    vector<Core> splitJobBlocksToNCore(Job &job, int n);
    vector<Core> bruteForceMultiCore(const vector<Core> &cores, Job& job);

    // Performance evaluation
    PerformanceReportSingleHost evaluatePerformanceSingleHost();

public:
    ResourceScheduler();
    void loadData(const string &path);  // load data from file path

    // Main algorithm
    void scheduleSingleHostLPTOnly();
    void scheduleSingleHostLPTWithBFMulticores();  // BF = Brute Force
    // TODO: Algorithm for multiple hosts

    // Helper function for debug
    void printResultText(const string &evalTitle);
    void exportData(const string &filePath);
};
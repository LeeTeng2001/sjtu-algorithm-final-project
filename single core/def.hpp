#pragma once

#include <vector>
#include <iosfwd>
#include <string>

using std::vector;
using std::string;
using std::ifstream;

struct JobBlock {
    int jobId; // belongs to nth job
    int jobBlockId;
    double dataSize;  // data size

    bool operator<(const JobBlock &rhs) const {
        return dataSize < rhs.dataSize;
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

    int totalSingleCoreExecTime = 0;
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
};

class ResourceScheduler {
private:
    int totalJob{};  // total number of job
    int totalHost{}; // total number of host
    double alpha{};  // alpha for speed decay constant
    vector<vector<Core>> hostCore;   // the jth core for ith host
    vector<Job> jobs;  // job infos

    void loadData();

    // double getFinishTime();
    double getFinishTimeDeviation(int host);
    double getLongestFinishTime(int host);

public:
    ResourceScheduler();

    // Main algorithm
    void schedule();

    void printResultText();
};
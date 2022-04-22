#pragma once

#include <vector>
#include <fstream>
#include <string>

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::ifstream;

struct JobBlock {
    int jobId; // belongs to nth job
    double dataSize;  // data size
};

struct Job {
    int id{};
    int totalBlocks{};  // amount of blocks
    double speed{};
    vector<JobBlock> blocks;
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
};

class ResourceScheduler {
private:
    int totalJob{};  // total number of job
    int totalHost{}; // total number of host
    double alpha{};  // alpha for speed decay constant
    vector<vector<Core>> hostCore;   // the jth core for ith host
    vector<Job> jobs;  // job infos

public:
    ResourceScheduler() {
        loadData();
    }

    void loadData() {
        // TODO: Load file for mode 2
        string filePath = "../../input/task1_case1.txt";
        ifstream inputFile(filePath);
        if (!inputFile) {
            cerr << "Cannot open input file for path: " << filePath << endl;
            return;
        }

        // Get total job, host and alpha
        inputFile >> totalJob >> totalHost >> alpha;

        // Initialise k cores for each host
        hostCore.resize(totalHost);
        for (auto &item: hostCore) {
            int totalCore;
            inputFile >> totalCore;
            item.resize(totalCore);
        }

        // Initialise job with job size and id
        for (int i = 0; i < totalJob; ++i) {
            int totalBlocks;
            inputFile >> totalBlocks;
            jobs.push_back({
                i,
                totalBlocks
            });
        }

        // Get base process speed for each job
        for (int i = 0; i < totalJob; ++i) {
            inputFile >> jobs[i].speed;
        }

        // Initialise job block for each job
        for (int i = 0; i < totalJob; ++i) {
            jobs[i].blocks.resize(jobs[i].totalBlocks);
            for (auto &block: jobs[i].blocks) {
                inputFile >> block.dataSize;
                block.jobId = i;
            }
        }

        // TODO: run loc?
    }
};
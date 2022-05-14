#pragma once

#include "def.hpp"
#include "color.hpp"
#include <random>
#include <iostream>
#include <fstream>

using std::cout;
using std::endl;
using std::ofstream;
using std::to_string;

// Essential info for task
struct GeneratorInfo {
    int numHost;
    int numJob;
    double alpha;

    int minCore, maxCore;
    int minBlock, maxBlock;
    int minSize, maxSize;
    double minJobSpeed, maxJobSpeed;

    // Multicore
    double minTransferSpeed, maxTransferSpeed;
};

struct GeneratorConfig {
    int taskType;
    int caseID;
    bool writeToOutput;
};

void writeTestCaseToFile(ResourceScheduler &rs, const GeneratorConfig& config);

void generateTestCases(ResourceScheduler &rs, const GeneratorInfo &info, const GeneratorConfig &config) {
    // Random generator
    std::random_device dev;
    std::mt19937 rng(dev());

    // Create range generator, [a, b)
    std::uniform_int_distribution<int> gnCore(info.minCore, info.maxCore + 1);
    std::uniform_int_distribution<int> gnBlock(info.minBlock, info.maxBlock + 1);
    std::uniform_int_distribution<int> gnSize(info.minSize, info.maxSize + 1);
    std::uniform_int_distribution<int> gnInitHostLocation(0, info.numHost);
    std::uniform_real_distribution<double> gnSpeedJob(info.minJobSpeed, info.maxJobSpeed);
    std::uniform_real_distribution<double> gnSpeedTransfer(info.minTransferSpeed, info.maxTransferSpeed);

    // Set init value in scheduler
    rs.totalJob = info.numJob, rs.totalHost = info.numHost, rs.alpha = info.alpha;
    rs.speedOfTransfer = gnSpeedTransfer(rng);
    rs.hostCore.resize(rs.totalHost);
    rs.jobs.resize(rs.totalJob);

    cout << "\n-----------Generator starts.--------------\n";

    cout << "numJob = " << rs.totalJob << ", numHost = " << rs.totalHost << ", alpha = " << rs.alpha;
    if (config.taskType == 2) cout << ", speedOfTransferBtHost = " << rs.speedOfTransfer;
    cout << "\n\n";

    cout << "Total core in each host:\n\t";
    for (int i = 0; i < rs.totalHost; i++) {
        rs.hostCore[i].clear();  // necessary to reset previous calculation cache (if any)
        rs.hostCore[i].resize(gnCore(rng));
        for (int j = 0; j < rs.hostCore[i].size(); ++j)
            rs.hostCore[i][j].coreId = j;
        cout << rs.hostCore[i].size() << " ";
    }

    cout << "\n\nTotal blocks in each job:\n\t";
    for (int i = 0; i < rs.totalJob; i++) {
        rs.jobs[i].totalBlocks = gnBlock(rng);
        rs.jobs[i].id = i;
        cout << rs.jobs[i].totalBlocks << " ";
    }

    cout << "\n\nCalculation speed of each job:\n\t";
    for (int i = 0; i < rs.totalJob; i++) {
        rs.jobs[i].speed = gnSpeedJob(rng);
        cout << rs.jobs[i].speed << " ";
    }

    cout << "\n\nBlock data size in each job:\n";
    for (int i = 0; i < rs.totalJob; i++) {
        rs.jobs[i].blocks.resize(rs.jobs[i].totalBlocks);
        cout << "\tJob " << i << ": ";
        for (int j = 0; j < rs.jobs[i].totalBlocks; j++) {
            rs.jobs[i].blocks[j].dataSize = gnSize(rng);
            rs.jobs[i].blocks[j].jobId = i;
            rs.jobs[i].blocks[j].jobBlockId = j;
            cout << rs.jobs[i].blocks[j].dataSize << " ";
        }
        cout << "\n";
    }

    cout << "\nInitial host location for each block in jobs:\n";
    for (int i = 0; i < rs.totalJob; i++) {
        cout << "\tJob " << i << ": ";
        for (int j = 0; j < rs.jobs[i].totalBlocks; j++) {
            rs.jobs[i].blocks[j].initHostLocation = gnInitHostLocation(rng);
            cout << rs.jobs[i].blocks[j].initHostLocation << " ";
        }
        cout << "\n";
    }

    // Write output to file if specified
    writeTestCaseToFile(rs, config);

    cout << "\n\n-----------Generator ends.--------------\n\n";
}

// Write to output if specified
void writeTestCaseToFile(ResourceScheduler &rs, const GeneratorConfig& config) {
    if (!config.writeToOutput) return;

    string fileName = "../../input/task" + to_string(config.taskType) + "_case" + to_string(config.caseID) + ".txt";
    ofstream fs(fileName, std::ios::binary | std::ios::ate);
    if (!fs) {
        cout << "Cannot write test case data to " << fileName << '\n';
        return;
    }

    // total job, host, alpha, speedOfTransfer (task2)
    fs << rs.totalJob << " " << rs.totalHost << " " << rs.alpha;
    if (config.taskType == 2) fs << " " << rs.speedOfTransfer;
    fs << '\n';

    // Core in each host
    for (int i = 0; i < rs.totalHost; i++)
        fs << rs.hostCore[i].size() << " ";
    fs << '\n';

    // block size of each job
    for (int i = 0; i < rs.totalJob; i++)
        fs << rs.jobs[i].totalBlocks << " ";
    fs << "\n";

    // Speed of each job
    for (int i = 0; i < rs.totalJob; i++)
        fs << rs.jobs[i].speed << " ";
    fs << "\n";

    // Block size in each job
    for (int i = 0; i < rs.totalJob; i++) {
        for (int j = 0; j < rs.jobs[i].totalBlocks; j++) {
            fs << rs.jobs[i].blocks[j].dataSize << " ";
        }
        fs << "\n";
    }

    // init host location of each job
    for (int i = 0; i < rs.totalJob; i++) {
        for (int j = 0; j < rs.jobs[i].totalBlocks; j++) {
            fs << rs.jobs[i].blocks[j].initHostLocation << " ";
        }
        fs << "\n";
    }
}



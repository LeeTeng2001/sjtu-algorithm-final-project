#include <iostream>
#include <fstream>
#include <numeric>
#include <queue>
#include "def.hpp"
#include "json.hpp"

using std::cerr;
using std::cout;
using std::endl;
using std::priority_queue;
using std::fstream;

ResourceScheduler::ResourceScheduler() {
    loadData();
}

void ResourceScheduler::loadData() {
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
    for (auto &singleHostCores: hostCore) {
        int totalCore;
        inputFile >> totalCore;
        singleHostCores.resize(totalCore);
        for (int i = 0; i < totalCore; ++i)
            singleHostCores[i].coreId = i;
    }

    // Initialise job with job size and id
    for (int i = 0; i < totalJob; ++i) {
        int totalBlocks;
        inputFile >> totalBlocks;
        jobs.push_back({i,totalBlocks});
    }

    // Get base process speed for each job
    for (int i = 0; i < totalJob; ++i) {
        inputFile >> jobs[i].speed;
    }

    // Initialise job block for each job
    for (int i = 0; i < totalJob; ++i) {
        jobs[i].blocks.resize(jobs[i].totalBlocks);
        for (int j = 0; j < jobs[i].blocks.size(); ++j) {
            inputFile >> jobs[i].blocks[j].dataSize;
            jobs[i].blocks[j].jobId = i;
            jobs[i].blocks[j].jobBlockId = j;
        }
    }

    // TODO: run loc?
}

double ResourceScheduler::getFinishTimeDeviation(int host) {
    auto getCoreFinishTime = [](double b, const Core& co) {
        return b + co.getFinishTime();
    };
    auto meanEnd = std::accumulate(hostCore[host].begin(), hostCore[host].end(), 0.0, getCoreFinishTime);

    auto varianceFormula = [meanEnd, host, this](double res, const Core& co) {
        auto diff = co.getFinishTime() - meanEnd;
        return res + diff * diff / hostCore[host].size();
    };
    auto variance = std::accumulate(hostCore[host].begin(), hostCore[host].end(), 0.0, varianceFormula);

    return sqrt(variance);
}

double ResourceScheduler::getLongestFinishTime(int host) {
    auto maxFinishTime = std::max_element(hostCore[host].begin(), hostCore[host].end());
    return maxFinishTime == hostCore[host].end() ? 0 : maxFinishTime->getFinishTime();
}

void ResourceScheduler::schedule() {
    const int host = 0;

    // Simple algorithm single thread
    for (auto &item: jobs)
        item.calculateSingleCoreExecTime();

    // Sort by longest single thread execution time
    auto cmpFinishTime = [](const Job& jo1, const Job& jo2) {
        return jo1.totalSingleCoreExecTime > jo2.totalSingleCoreExecTime;
    };
    sort(jobs.begin(), jobs.end(), cmpFinishTime);

    // Add core into min priority queue
    priority_queue<Core, vector<Core>, std::greater<>> priority(hostCore[host].begin(), hostCore[host].end());

    // Select min core, add all blocks to the core
    for (const auto &item: jobs) {
        auto selectedCore = priority.top();
        priority.pop();

        for (const auto &jobBlock: item.blocks) {
            BlockExecInfo execInfo{
                selectedCore.getFinishTime(),
                selectedCore.getFinishTime() + jobBlock.executionTime(1, item.speed, alpha)
            };

            selectedCore.blocks.push_back(jobBlock);
            selectedCore.blockInfos.push_back(execInfo);
        }

        priority.push(selectedCore);
    }

    // put core back to result
    while (!priority.empty()) {
        auto selectedCore = priority.top();
        priority.pop();

        hostCore[host][selectedCore.coreId] = selectedCore;
    }
}

void ResourceScheduler::printResultText() {
    for (int i = 0; i < totalHost; ++i) {

        for (int j = 0; j < hostCore[i].size(); ++j) {
            cout << "Host " << i << ", core" << j << '\n';

            auto totalBlocks = hostCore[i][j].blocks.size();
            for (int k = 0; k < totalBlocks; ++k) {
                // print block info
                cout << "\tjob = " << hostCore[i][j].blocks[k].jobId;
                cout << ", block = " << hostCore[i][j].blocks[k].jobBlockId;
                cout << ", size = " << hostCore[i][j].blocks[k].dataSize;

                // print time
                cout << "\tstart = " << hostCore[i][j].blockInfos[k].startTime;
                cout << ", end =" << hostCore[i][j].blockInfos[k].endTime << '\n';
            }
        }

        // TODO: Calculate time efficiency

        // Calculate host efficiency and other stats
        cout << "Finish time for each core: " << '\n';

        for (int j = 0; j < hostCore[i].size(); ++j) {
            cout << "\tCore " << j << ": " << hostCore[i][j].getFinishTime() << '\n';
        }

        cout << "Finish time deviation for host " << i << " = " << getFinishTimeDeviation(i) << '\n';
        cout << "Longest finish time for host " << i << " = " << getLongestFinishTime(i) << '\n';
    }
}

void ResourceScheduler::exportData() {
    using nlohmann::json;
    json jFile;
    fstream saveFile("data-export.json", fstream::out | fstream::trunc);

    for (int i = 0; i < totalHost; ++i) {
        for (int j = 0; j < hostCore[i].size(); ++j) {
            string id = "Host" + std::to_string(i) + "Core" + std::to_string(j);
            // TODO: 1 based or 0 based?
            jFile[id]["host"] = i + 1;
            jFile[id]["core"] = j + 1;
            jFile[id]["current"] = 0;

            auto totalBlocks = hostCore[i][j].blocks.size();
            for (int k = 0; k < totalBlocks; ++k) {
                jFile[id]["deals"].push_back({
                     {"job", hostCore[i][j].blocks[k].jobId},
                     {"block", hostCore[i][j].blocks[k].jobBlockId},
                     {"from", static_cast<int>(hostCore[i][j].blockInfos[k].startTime * 1000)},
                     {"to", static_cast<int>(hostCore[i][j].blockInfos[k].endTime * 1000)},
                });
            }
        }

        // // Calculate host efficiency and other stats
        // cout << "Finish time for each core: " << '\n';
        //
        // for (int j = 0; j < hostCore[i].size(); ++j) {
        //     cout << "\tCore " << j << ": " << hostCore[i][j].getFinishTime() << '\n';
        // }
        //
        // cout << "Finish time deviation for host " << i << " = " << getFinishTimeDeviation(i) << '\n';
        // cout << "Longest finish time for host " << i << " = " << getLongestFinishTime(i) << '\n';
    }

    saveFile << jFile;
}


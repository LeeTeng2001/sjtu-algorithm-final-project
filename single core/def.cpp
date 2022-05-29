#include <iostream>
#include <iomanip>
#include <fstream>
#include <numeric>
#include <queue>
#include "def.hpp"
#include "color.hpp"
#include "json.hpp"

using std::cerr;
using std::cout;
using std::endl;
using std::priority_queue;
using std::fstream;
using std::pair;

// Manip
using std::fixed;
using std::setprecision;
using std::setw;
using std::left;

constexpr double EPSILON = 0.000001;
constexpr bool VERBOSE_REPORT = false;

ResourceScheduler::ResourceScheduler() = default;

void ResourceScheduler::loadData(const string &path, bool isMode2) {
    ifstream inputFile(path);
    if (!inputFile) {
        cerr << "Cannot open input file for path: " << path << endl;
        return;
    }

    // Get total job, host and alpha, speed of transfer (2nd task)
    inputFile >> totalJob >> totalHost >> alpha;
    if (isMode2) inputFile >> speedOfTransfer;

    // Initialise k cores for each host
    hostCore.resize(totalHost);
    for (auto &singleHostCores: hostCore) {
        int totalCore;
        inputFile >> totalCore;
        singleHostCores.resize(totalCore);
        for (int i = 0; i < totalCore; ++i) {
            singleHostCores[i].coreId = i;
            singleHostCores[i].blocks.clear();
            singleHostCores[i].blockInfos.clear();
        }
    }

    // Initialise job with job size and id
    jobs.resize(totalJob);
    for (int i = 0; i < totalJob; ++i) {
        int totalBlocks;
        inputFile >> totalBlocks;
        jobs[i] = {i, totalBlocks};
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

    // Initial host location for each job
    for (int i = 0; i < totalJob; ++i) {
        for (int j = 0; j < jobs[i].blocks.size(); ++j) {
            inputFile >> jobs[i].blocks[j].initHostLocation;
        }
    }
}

double ResourceScheduler::getFinishTimeDeviation(const vector<Core> &cores) {
    auto getCoreFinishTime = [](double b, const Core& co) {
        return b + co.getFinishTime();
    };
    auto meanEnd = std::accumulate(cores.begin(), cores.end(), 0.0, getCoreFinishTime);

    auto varianceFormula = [meanEnd, &cores](double res, const Core& co) {
        auto diff = co.getFinishTime() - meanEnd;
        return res + diff * diff / cores.size();
    };
    auto variance = std::accumulate(cores.begin(), cores.end(), 0.0, varianceFormula);

    return sqrt(variance);
}

double ResourceScheduler::getLongestFinishTime(const vector<Core> &cores) {
    auto maxFinishTime = std::max_element(cores.begin(), cores.end());
    return maxFinishTime == cores.end() ? 0 : maxFinishTime->getFinishTime();
}

vector<Core> ResourceScheduler::splitJobBlocksToNCore(Job &job, int n) {
    // Create new core & add them into min priority queue
    priority_queue<Core, vector<Core>, std::greater<>> priority;
    for (int i = 0; i < n; ++i)
        priority.emplace();

    // Sort block by largest data size (the longest process time)
    sort(job.blocks.begin(), job.blocks.end(), std::greater<>());

    // Select min core, add the longest block to the core (LPT algorithm)
    for (const auto &jobBlock: job.blocks) {
        auto selectedCore = priority.top();
        priority.pop();

        BlockExecInfo execInfo{
            selectedCore.getFinishTime(),
            selectedCore.getFinishTime() + jobBlock.executionTime(n, job.speed, alpha)
        };

        selectedCore.blocks.push_back(jobBlock);
        selectedCore.blockInfos.push_back(execInfo);
        priority.push(selectedCore);
    }

    // Add result
    vector<Core> seperatedCores;
    while (!priority.empty()) {
        seperatedCores.push_back(priority.top());
        priority.pop();
    }

    return seperatedCores;  // [min, .. ,max]
}

vector<Core> ResourceScheduler::bruteForceMultiCore(const vector<Core> &cores, Job& job) {
    // Build dummy cores
    vector<Core> originalCores = cores;
    sort(originalCores.begin(), originalCores.end());  // min finish time ... max finish time

    // Brute force try to find the most balanced partition (by weighted scores)
    const double finishWeight = 1.5, splitBalanceWeight = 1;
    double originalLongestFinishTime = originalCores.back().getFinishTime();
    vector<Core> minFinishTimeCores;
    double globMinPartitionScore = std::numeric_limits<double>::max();  // update at least 1 time

    // cout << FMAG("Trying to partition job: ") << job.id << FMAG(", total block: ") << job.totalBlocks << '\n';
    for (int i = 1; i <= std::min(job.totalBlocks, (int) cores.size()); ++i) {
        vector<Core> newCores = originalCores;  // new partition
        vector<Core> jobToNCores = splitJobBlocksToNCore(job, i);

        // Time padding for synchronisation
        double syncTimeStart = originalCores[i - 1].getFinishTime();
        double syncTimeEnd = syncTimeStart + jobToNCores.back().getFinishTime();

        // Fill cores with our new combination
        for (int j = 0; j < i; ++j) {
            // Add front sync padding
            if (fabs(newCores[j].getFinishTime() - syncTimeStart) > EPSILON) {
                newCores[j].blocks.push_back({-1, -1, -1});
                newCores[j].blockInfos.push_back({newCores[j].getFinishTime(), syncTimeStart});
            }

            // Add blocks
            for (int k = 0; k < jobToNCores[j].blocks.size(); ++k) {
                newCores[j].blocks.push_back(jobToNCores[j].blocks[k]);
                newCores[j].blockInfos.push_back({
                    newCores[j].getFinishTime(),
                    newCores[j].getFinishTime() + jobToNCores[j].blockInfos[k].endTime - jobToNCores[j].blockInfos[k].startTime
                });
            }

            // Add back sync padding
            if (fabs(newCores[j].getFinishTime() - syncTimeEnd) > EPSILON) {
                newCores[j].blocks.push_back({-1, -1, -1});
                newCores[j].blockInfos.push_back({newCores[j].getFinishTime(), syncTimeEnd});
            }
        }

        // Update global min
        double extraFinishTime = std::max(0.0, syncTimeEnd - originalLongestFinishTime);
        double localSplitDeviation = i == 1 ? 1.5 : getFinishTimeDeviation(jobToNCores);  // special case for single job
        double partitionScore = finishWeight * extraFinishTime + splitBalanceWeight * localSplitDeviation;
        // cout << "\tPartition to " << i <<  " core, extraFinishTime: " << extraFinishTime << ", localSplitDev: " << localSplitDeviation << '\n';

        if (partitionScore < globMinPartitionScore) {
            // cout << FRED("\tNew score: ") << partitionScore << ", original: " << globMinPartitionScore << '\n';
            globMinPartitionScore = partitionScore;
            minFinishTimeCores = newCores;
        }
    }

    return minFinishTimeCores;
}

void ResourceScheduler::scheduleSingleHostLPTOnly() {
    const int host = 0;

    // Calculate total process time for single job in one core
    for (auto &item: jobs)
        item.calculateSingleCoreExecTime();

    // Sort by longest single thread execution time (the longest processing time)
    sort(jobs.begin(), jobs.end(), std::greater<>());

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

void ResourceScheduler::scheduleSingleHostLPTWithBFMulticores() {
    const int host = 0;

    // Calculate total process time for single job in one core
    for (auto &item: jobs)
        item.calculateSingleCoreExecTime();

    // Sort by longest single thread execution time
    sort(jobs.begin(), jobs.end(), std::greater<>());

    // Partition all by longest processing job
    vector<Core> dummyCores = hostCore[host];
    for (auto &job: jobs) {
        dummyCores = bruteForceMultiCore(dummyCores, job);
    }

    // put core back to result
    for (const auto &dummyCore: dummyCores) {
        hostCore[host][dummyCore.coreId] = dummyCore;
    }
}


void ResourceScheduler::printResultText(const string &evalTitle) {
    cout << FCYN("--------Evaluating performance for method: " << evalTitle << "--------\n");
    for (int i = 0; i < totalHost; ++i) {
        if (VERBOSE_REPORT) {
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
        }

        // Print host efficiency and other interesting stats
        // cout << "Finish time for each core: " << '\n';
        // for (int j = 0; j < hostCore[i].size(); ++j) {
        //     cout << "\tCore " << j << ": " << hostCore[i][j].getFinishTime() << '\n';
        // }

        PerformanceReportSingleHost performanceReport = evaluatePerformanceSingleHost();

        const int floatPrecision = 3;
        const int textWidth = 32;
        cout << FCYN("" << left << setw(textWidth) << "Performance info for host: ") << i << '\n';
        cout << FGRN("" << left << setw(textWidth) << "Finish time deviation: ") << setprecision(floatPrecision) << fixed<< performanceReport.finishTimeStd << '\n';
        cout << FGRN("" << left << setw(textWidth) << "Longest finish time: ") << setprecision(floatPrecision) << fixed<< performanceReport.longestFinishTime << "s\n";
        cout << FGRN("" << left << setw(textWidth) << "Average process time: ") << setprecision(floatPrecision) << fixed<< performanceReport.averageRealProcessingTime << "s\n";
        cout << FGRN("" << left << setw(textWidth) << "Average idle time: ") << setprecision(floatPrecision) << fixed<< performanceReport.averageFragmentTime << "s\n";
        cout << FGRN("" << left << setw(textWidth) << "CPU utilisation: ") << setprecision(floatPrecision) << fixed<< performanceReport.utilisationPercentage << "%\n";
    }
}

void ResourceScheduler::exportData(const string &filePath) {
    using nlohmann::json;
    json jFile;
    fstream saveFile(filePath, fstream::out | fstream::trunc);

    for (int i = 0; i < totalHost; ++i) {
        for (int j = 0; j < hostCore[i].size(); ++j) {
            string id = "Host" + std::to_string(i) + "Core" + std::to_string(j);
            // TODO: 1 based or 0 based?
            jFile[id]["host"] = i + 1;
            jFile[id]["core"] = j + 1;
            jFile[id]["current"] = 0;

            // Process each non-idle block
            auto totalBlocks = hostCore[i][j].blocks.size();
            for (int k = 0; k < totalBlocks; ++k) {
                if (hostCore[i][j].blocks[k].jobId == -1)
                    continue;

                jFile[id]["deals"].push_back({
                     {"job", hostCore[i][j].blocks[k].jobId},
                     {"block", hostCore[i][j].blocks[k].jobBlockId},
                     {"from", static_cast<int>(hostCore[i][j].blockInfos[k].startTime * 1000)},
                     {"to", static_cast<int>(hostCore[i][j].blockInfos[k].endTime * 1000)},
                });
            }
        }
    }

    saveFile << jFile;
}

PerformanceReportSingleHost ResourceScheduler::evaluatePerformanceSingleHost() {
    PerformanceReportSingleHost report{};
    report.finishTimeStd = getFinishTimeDeviation(hostCore[0]);
    report.longestFinishTime = getLongestFinishTime(hostCore[0]);

    // Calculate process time
    for (int j = 0; j < hostCore[0].size(); ++j) {
        auto totalBlocks = hostCore[0][j].blocks.size();
        for (int k = 0; k < totalBlocks; ++k) {
            // Check internal fragment/real processing time
            auto processTime = hostCore[0][j].blockInfos[k].endTime - hostCore[0][j].blockInfos[k].startTime;
            if (hostCore[0][j].blocks[k].jobId == -1)
                report.fragmentTimeInternal += processTime;
            else
                report.totalRealProcessingTime += processTime;
        }
        // Check end fragment
        if (fabs(hostCore[0][j].getFinishTime() - report.longestFinishTime) > EPSILON) {
            report.fragmentTimeEnd += fabs(hostCore[0][j].getFinishTime() - report.longestFinishTime);
        }
    }

    // Calculate utilisation percentage
    report.averageFragmentTime = (report.fragmentTimeEnd + report.fragmentTimeInternal) / (double) hostCore[0].size();
    report.averageRealProcessingTime = report.totalRealProcessingTime / (double) hostCore[0].size();
    report.utilisationPercentage = report.averageRealProcessingTime * 100 / (report.averageRealProcessingTime + report.averageFragmentTime);

    return report;
}

void ResourceScheduler::scheduleMultiHostsNoTransmission() {
    // // Calculate total process time for single job in one core
    // for (auto &item: jobs)
    //     item.calculateSingleCoreExecTime();
    //
    // // Sort by longest single thread execution time
    // sort(jobs.begin(), jobs.end(), std::greater<>());
    //
    // // Partition all by longest processing job
    // vector<Core> dummyCores = hostCore[host];
    // for (auto &job: jobs) {
    //     dummyCores = bruteForceMultiCore(dummyCores, job);
    // }
    //
    // // put core back to result
    // for (const auto &dummyCore: dummyCores) {
    //     hostCore[host][dummyCore.coreId] = dummyCore;
    // }
}

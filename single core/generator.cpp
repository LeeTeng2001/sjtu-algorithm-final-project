#include"../hFiles/Util.h"

#include <random>

struct GeneratorInfo {
    int numHost;
    int numJob;
    double alpha;

    int minCore, maxCore;
    int minBlock, maxBlock;
    int minSize, maxSize;
    double minSpeed, maxSpeed;
};

// Random generator
// For testing, you can modify the range of the next parameters and get a new test case.
void generator(ResourceScheduler &rs, GeneratorInfo &info, int taskType, int caseID) {
    srand((int) time(0));
    rs.numJob = info.numJob, rs.numHost = info.numHost, rs.alpha = info.alpha;
    if (taskType == 2) rs.St = 400;
    int minCore = info.minCore, maxCore = info.maxCore;      // The number of cores in a host.
    int minBlock = info.minBlock, maxBlock = info.maxBlock;   // The number of blocks in a job.
    int minSize = info.minSize, maxSize = info.maxSize;     // The size of a block in a job.
    double minSpeed = info.minSpeed, maxSpeed = info.maxSpeed;  // Calculating speed.
    rs.hostCore.resize(rs.numHost);
    rs.jobBlock.resize(rs.numJob);
    rs.Sc.resize(rs.numJob);
    rs.dataSize.resize(rs.numJob);
    rs.location.resize(rs.numJob);

    cout << "\n\n-----------Generator starts.--------------\n\n";

    cout << "numJob = " << rs.numJob << ", numHost = " << rs.numHost << ", alpha = " << rs.alpha;
    if (taskType == 2) cout << ", St = " << rs.St;
    cout << "\n\n";

    cout << "hostCore:\n";
    for (int i = 0; i < rs.numHost; i++) {
        rs.hostCore[i] = rand() % (maxCore - minCore + 1) + minCore;
        cout << rs.hostCore[i] << " ";
    }

    cout << "\n\njobBlockNumber:\n";
    for (int i = 0; i < rs.numJob; i++) {
        rs.jobBlock[i] = rand() % (maxBlock - minBlock + 1) + minBlock;
        cout << rs.jobBlock[i] << " ";
    }

    cout << "\n\njobCalculatingSpeed:\n";
    for (int i = 0; i < rs.numJob; i++) {
        rs.Sc[i] = rand() % int(maxSpeed - minSpeed + 1) + minSpeed;
        cout << rs.Sc[i] << " ";
    }

    cout << "\n\nblockDataSize:\n";
    for (int i = 0; i < rs.numJob; i++) {
        rs.dataSize[i].resize(rs.jobBlock[i]);
        for (int j = 0; j < rs.jobBlock[i]; j++) {
            rs.dataSize[i][j] = rand() % (maxSize - minSize + 1) + minSize;
            cout << rs.dataSize[i][j] << " ";
        }
        cout << endl;
    }

    cout << "\njobBlockInitialLocation:\n";
    for (int i = 0; i < rs.numJob; i++) {
        rs.location[i].resize(rs.jobBlock[i]);
        for (int j = 0; j < rs.jobBlock[i]; j++) {
            rs.location[i][j] = rand() % rs.numHost;
            cout << rs.location[i][j] << " ";
        }
        cout << endl;
    }

    rs.jobFinishTime.resize(rs.numJob, 0);
    rs.jobCore.resize(rs.numJob);

    rs.runLoc.resize(rs.numJob);
    for (int i = 0; i < rs.numJob; i++)
        rs.runLoc[i].resize(rs.jobBlock[i]);

    rs.hostCoreTask.resize(rs.numHost);
    for (int i = 0; i < rs.numHost; i++)
        rs.hostCoreTask[i].resize(rs.hostCore[i]);

    rs.hostCoreFinishTime.resize(rs.numHost);
    for (int i = 0; i < rs.numHost; i++)
        rs.hostCoreFinishTime[i].resize(rs.hostCore[i], 0);

    string fileName = "../../input/task" + to_string(taskType) + "_case" + to_string(caseID) + ".txt";
    string text = to_string(rs.numJob) + " " + to_string(rs.numHost) + " " + to_string(rs.alpha);
    if (rs.taskType == 2)text += " " + to_string(rs.St);
    text += "\n";

    for (int i = 0; i < rs.numHost; i++)
        text += to_string(rs.hostCore[i]) + " ";
    text += "\n";

    for (int i = 0; i < rs.numJob; i++)
        text += to_string(rs.jobBlock[i]) + " ";
    text += "\n";

    for (int i = 0; i < rs.numJob; i++)
        text += to_string(rs.Sc[i]) + " ";
    text += "\n";

    for (int i = 0; i < rs.numJob; i++) {
        for (int j = 0; j < rs.jobBlock[i]; j++)
            text += to_string(rs.dataSize[i][j]) + " ";
        text += "\n";
    }

    for (int i = 0; i < rs.numJob; i++) {
        for (int j = 0; j < rs.jobBlock[i]; j++)
            text += to_string(rs.location[i][j]) + " ";
        text += "\n";
    }
    WriteData(fileName, text);

    cout << "\n\n-----------Generator ends.--------------\n\n";
}

void WriteData(const string& fileName, const string& text) {
//	ifstream ifs(fileName);
//	if (!ifs)
//		cout << "Create file failed!" << endl;
//	string content((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>())); // old file content
//	ifs.close();

    ofstream fileStream;
    fileStream.open(fileName.c_str(), ios::binary | ios::ate);
    fileStream.seekp(0, ios::beg);
    fileStream << text << endl;
    fileStream << flush;
    fileStream.close();
}

int main() {
    // Needs to ensure there is a file named "task_$taskType_case_$caseID.txt" in 'input' filefolder.
    // You can test more cases if you want.
    int taskType = 1, caseID = 10;

    // Load data from file "task_$taskType_case_$caseID.txt" to class ResourceScheduler instance object.
    ResourceScheduler rs(taskType, caseID);

    // TODO: Generate test cases in different conditions

    // 1.
    GeneratorInfo g1 {
            // host, job, alpha
            1, 8, 0.05,
            // range of core, block, size, speed
            5, 6,
            1, 10,
            1, 10,
            10, 20,
    };

    generator(rs, g1, taskType, caseID);
}
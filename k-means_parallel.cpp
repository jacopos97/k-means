#include <iostream>
#include "INIReader.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <cmath>

using namespace std;
using namespace chrono;

static const string DATASET_PATH = "../datasets/generated_blob_dataset_400k.csv";
static const string CONFIG_FILE_PATH = "../config_files/config_sets.ini";
static const string DESIRED_CONFIG = "4_cluster";
static const int ITERATION_NUMBER = 10;
static const int THREAD_NUMBER = 16;

struct DataPoints {
    std::vector<float>
            xs;
    std::vector<float> ys;
    std::vector<float> zs;
};

void printCentroids(DataPoints& centroids) {
    for (int i=0; i<centroids.xs.size(); i++) {
        cout << "(" << centroids.xs[i] << ", " << centroids.ys[i] << ", " << centroids.zs[i] << ")" << endl;
    }
}

bool readDatasetFromFile(DataPoints& dataset, const string& fullPath) {
    ifstream file(fullPath);
    if (file.is_open()) {
        string line;
        cout << "Reading the dataset..." << endl;
        while (getline(file, line)) {
            istringstream coordinates(line);
            float x;
            float y;
            float z;
            char delimiter1;
            char delimiter2;
            if (coordinates >> x >> delimiter1 >> y >> delimiter2 >> z) {
                dataset.xs.push_back(x);
                dataset.ys.push_back(y);
                dataset.zs.push_back(z);
            }
        }
        file.close();
        cout << "Dataset loaded from " << fullPath << endl;
        return true;
    } else {
        cerr << "Error: Unable to open file " << fullPath << endl;
        return false;
    }
}

bool initializeCentroids(DataPoints& centroids, int& clusterNum, const string& configFilePath, const string& desiredConfig) {
    INIReader reader(configFilePath);
    if (reader.ParseError() < 0) {
        cerr << "Error loading config file\n";
        return false;
    }
    clusterNum = reader.GetInteger(desiredConfig, "cluster_num", 0);
    for(int i=0; i < clusterNum; i++)  {
        istringstream coordinates(reader.Get(desiredConfig, "centroid" + to_string(i), ""));
        float x;
        float y;
        float z;
        char delimiter1;
        char delimiter2;
        if (coordinates >> x >> delimiter1 >> y >> delimiter2 >> z){
            centroids.xs.push_back(x);
            centroids.ys.push_back(y);
            centroids.zs.push_back(z);
        }
    }
    return true;
}

int main() {

    DataPoints dataPoints;
    if(!readDatasetFromFile(dataPoints, DATASET_PATH)) return -1;
    DataPoints centroids;
    int clusterNum;
    if (!initializeCentroids(centroids, clusterNum, CONFIG_FILE_PATH, DESIRED_CONFIG)) return -1;
    vector<int> totalClustersSize(clusterNum);


    printCentroids(centroids);

    auto startTime = high_resolution_clock::now();
#pragma omp parallel num_threads(THREAD_NUMBER) default(none) shared(dataPoints,centroids,clusterNum,cout,totalClustersSize)
    {
        for (int iteration = 0; iteration < ITERATION_NUMBER; iteration++) {
#pragma omp master
            cout << endl << "Iteration " << iteration + 1 << ":" << endl;

            DataPoints newCentroids;
            vector<int> clustersSize(clusterNum);
            vector<float> defaultCoordinate(clusterNum);
            newCentroids.xs.insert(newCentroids.xs.end(), defaultCoordinate.begin(), defaultCoordinate.end());
            newCentroids.ys.insert(newCentroids.ys.end(), defaultCoordinate.begin(), defaultCoordinate.end());
            newCentroids.zs.insert(newCentroids.zs.end(), defaultCoordinate.begin(), defaultCoordinate.end());

#pragma omp for schedule(static)
            for (int i = 0; i < dataPoints.xs.size(); i++) {
                float shortestDistance = sqrt(
                        pow(centroids.xs[0] - dataPoints.xs[i], 2) + pow(centroids.ys[0] - dataPoints.ys[i], 2) +
                        pow(centroids.zs[0] - dataPoints.zs[i], 2));
                int clusterType = 0;
                for (int j = 1; j < clusterNum; j++) {
                    float centroidDistance = sqrt(
                            pow(centroids.xs[j] - dataPoints.xs[i], 2) + pow(centroids.ys[j] - dataPoints.ys[i], 2) +
                            pow(centroids.zs[j] - dataPoints.zs[i], 2));
                    if (centroidDistance < shortestDistance) {
                        shortestDistance = centroidDistance;
                        clusterType = j;
                    }
                }
                newCentroids.xs[clusterType] += dataPoints.xs[i];
                newCentroids.ys[clusterType] += dataPoints.ys[i];
                newCentroids.zs[clusterType] += dataPoints.zs[i];
                clustersSize[clusterType]++;
            }

#pragma omp single
            {
                for(int i = 0; i < clusterNum; i++){
                    centroids.xs[i] = 0;
                    centroids.ys[i] = 0;
                    centroids.zs[i] = 0;
                }
            }

            for (int i = 0; i < clusterNum; i++) {
#pragma omp atomic
                centroids.xs[i] += newCentroids.xs[i];
#pragma omp atomic
                centroids.ys[i] += newCentroids.ys[i];
#pragma omp atomic
                centroids.zs[i] += newCentroids.zs[i];
#pragma omp atomic
                totalClustersSize[i] += clustersSize[i];
            }
#pragma omp barrier
#pragma omp single
            {
                cout << endl;
                for (int i = 0; i < clusterNum; i++) {
                    cout << "Cluster" << i + 1 << " size: " << totalClustersSize[i] << endl;
                    centroids.xs[i] = centroids.xs[i] / totalClustersSize[i];
                    centroids.ys[i] = centroids.ys[i] / totalClustersSize[i];
                    centroids.zs[i] = centroids.zs[i] / totalClustersSize[i];
                    totalClustersSize[i] = 0;
                }
                cout << endl;
                printCentroids(centroids);
            }
        }
    }
    auto endTime = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(endTime - startTime).count() / 1000.f;
    cout << "Duration: " << time << " ms" << endl;

    return 0;
}
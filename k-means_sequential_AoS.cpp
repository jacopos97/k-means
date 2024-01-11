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

struct DataPoint {
    float x;
    float y;
    float z;

    DataPoint() : x(0.0), y(0.0), z(0.0) {}

    DataPoint(float x, float y, float z) : x(x), y(y), z(z) {}
};

void printCentroids(vector<DataPoint>& centroids) {
    for (const auto& element : centroids) {
        cout << "(" << (element).x << ", " << (element).y << ", " << (element).z << ")" << endl;
    }
}

bool readDatasetFromFile(vector<DataPoint>& dataset, const string& datasetPath) {
    ifstream file(datasetPath);
    if (file.is_open()) {
       string line;
       cout << "Reading the dataset..." << endl;
        while (getline(file, line)) {
            istringstream coordinates(line);
            DataPoint point;
            char delimiter1;
            char delimiter2;
            if (coordinates >> point.x >> delimiter1 >> point.y >> delimiter2 >> point.z) {
                dataset.push_back(point);
            }
        }
        file.close();
        cout << "Dataset loaded from " << datasetPath << endl;
        return true;
    } else {
         cerr << "Error: Unable to open file " << datasetPath << endl;
        return false;
    }
}

bool initializeCentroids(vector<DataPoint>& centroids, int& clusterNum, const string& configFilePath, const string& desiredConfig) {
    INIReader reader(configFilePath);
    if (reader.ParseError() < 0) {
        cerr << "Error loading config file\n";
        return false;
    }
    clusterNum = reader.GetInteger(desiredConfig, "cluster_num", 0);
    for(int i=0; i < clusterNum; i++)  {
        istringstream coordinates(reader.Get(desiredConfig, "centroid" + to_string(i), ""));
        DataPoint centroid;
        char delimiter1;
        char delimiter2;
        if (coordinates >> centroid.x >> delimiter1 >> centroid.y >> delimiter2 >> centroid.z)
            centroids.push_back(centroid);
    }
    return true;
}

int main() {

    vector<DataPoint> points;
    if(!readDatasetFromFile(points, DATASET_PATH)) return -1;
    vector<DataPoint> centroids;
    int clusterNum;
    if (!initializeCentroids(centroids, clusterNum, CONFIG_FILE_PATH, DESIRED_CONFIG)) return -1;
    vector<vector<DataPoint*>> clusters(clusterNum);

    printCentroids(centroids);

    auto startTime = high_resolution_clock::now();
    for (int iteration=0; iteration<ITERATION_NUMBER  ; iteration++) {
        cout << endl << "Iteration " << iteration+1 << ":" << endl;

        vector<DataPoint> newCentroids(clusterNum);
        vector<int> clustersSize(clusterNum);
        for (int i=0; i < clusterNum; i++){
            DataPoint datapoint;
            newCentroids.emplace_back(datapoint);
        }

        for (int i=0; i < points.size(); i++) {
            float shortestDistance = sqrt(pow(centroids[0].x - points[i].x, 2) + pow(centroids[0].y - points[i].y, 2) + pow(centroids[0].z - points[i].z, 2));
            int clusterType = 0;
            for (int j=1; j<centroids.size(); j++) {
                float centroidDistance = sqrt(pow(centroids[j].x - points[i].x, 2) + pow(centroids[j].y - points[i].y, 2) + pow(centroids[j].z - points[i].z, 2));
                if (centroidDistance < shortestDistance) {
                    shortestDistance = centroidDistance;
                    clusterType = j;
                }
            }
            newCentroids[clusterType].x += points[i].x;
            newCentroids[clusterType].y += points[i].y;
            newCentroids[clusterType].z += points[i].z;
            clustersSize[clusterType]++;
        }

        for (int i=0; i<centroids.size(); i++) {
            centroids[i].x = newCentroids[i].x / clustersSize[i];
            centroids[i].y = newCentroids[i].y / clustersSize[i];
            centroids[i].z = newCentroids[i].z / clustersSize[i];
        }

        cout << endl;
        for (int i=0; i < clusterNum; i++) {
            cout << "Cluster" << i+1 << " size: " << clustersSize[i] << endl;
        }

        cout << endl;
        printCentroids(centroids);

    }
    auto endTime = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(endTime - startTime).count() / 1000.f;
    cout << "Duration: " << time << " ms" << endl;

    return 0;
}

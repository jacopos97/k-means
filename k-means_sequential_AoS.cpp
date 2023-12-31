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

void print_centroids(vector<DataPoint>& centroids) {
    for (const auto& element : centroids) {
        cout << "(" << (element).x << ", " << (element).y << ", " << (element).z << ")" << endl;
    }
}

bool readDatasetFromFile(vector<DataPoint>& dataset, const string& dataset_path) {
    ifstream file(dataset_path);
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
        cout << "Dataset loaded from " << dataset_path << endl;
        return true;
    } else {
         cerr << "Error: Unable to open file " << dataset_path << endl;
        return false;
    }
}

bool initialize_centroids(vector<DataPoint>& centroids, int& cluster_num, const string& config_file_path, const string& desired_config) {
    INIReader reader(config_file_path);
    if (reader.ParseError() < 0) {
        cerr << "Error loading config file\n";
        return false;
    }
    cluster_num = reader.GetInteger(desired_config, "cluster_num", 0);
    for(int i=0; i<cluster_num; i++)  {
        istringstream coordinates(reader.Get(desired_config, "centroid"+ to_string(i), ""));
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
    int cluster_num;
    if (!initialize_centroids(centroids,cluster_num,CONFIG_FILE_PATH,DESIRED_CONFIG)) return -1;
    vector<vector<DataPoint*>> clusters(cluster_num);

    print_centroids(centroids);

    auto start_time = high_resolution_clock::now();
    for (int iteration=0; iteration<ITERATION_NUMBER  ; iteration++) {
        cout << endl << "Iteration " << iteration+1 << ":" << endl;

        vector<DataPoint> new_centroids(cluster_num);
        vector<int> clusters_size(cluster_num);
        for (int i=0; i<cluster_num; i++){
            DataPoint datapoint;
            new_centroids.emplace_back(datapoint);
        }

        for (int i=0; i < points.size(); i++) {
            float shortest_distance = sqrt(pow(centroids[0].x - points[i].x, 2) + pow(centroids[0].y - points[i].y, 2) + pow(centroids[0].z - points[i].z, 2));
            int cluster_type = 0;
            for (int j=1; j<centroids.size(); j++) {
                float centroid_distance = sqrt(pow(centroids[j].x - points[i].x, 2) + pow(centroids[j].y - points[i].y, 2) + pow(centroids[j].z - points[i].z, 2));
                if (centroid_distance < shortest_distance) {
                    shortest_distance = centroid_distance;
                    cluster_type = j;
                }
            }
            new_centroids[cluster_type].x += points[i].x;
            new_centroids[cluster_type].y += points[i].y;
            new_centroids[cluster_type].z += points[i].z;
            clusters_size[cluster_type]++;
        }

        for (int i=0; i<centroids.size(); i++) {
            centroids[i].x = new_centroids[i].x/clusters_size[i];
            centroids[i].y = new_centroids[i].y/clusters_size[i];
            centroids[i].z = new_centroids[i].z/clusters_size[i];
        }

        cout << endl;
        for (int i=0; i < cluster_num; i++) {
            cout << "Cluster" << i+1 << " size: " << clusters_size[i] << endl;
        }

        cout << endl;
        print_centroids(centroids);

    }
    auto end_time = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(end_time - start_time).count() / 1000.f;
    cout << "Duration: " << time << " ms" << endl;

    return 0;
}

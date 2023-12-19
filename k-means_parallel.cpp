#include <iostream>
#include "INIReader.h"
//#include <numeric>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <cmath>

using namespace std;
using namespace chrono;

static const string DATASET_PATH = "../datasets/generated_blob_dataset_400k.csv";
static const string CONFIG_FILE_PATH = "../config_files/config_sets.ini";
static const string DESIRED_CONFIG = "4_cluster";
static const int ITERATION_NUMBER = 10;
static const int THREAD_NUMBER = 1;

struct DataPoints {
    std::vector<float> xs;
    std::vector<float> ys;
    std::vector<float> zs;
};

void print_centroids(DataPoints& centroids) {
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

bool initialize_centroids(DataPoints& centroids, int& cluster_num, const string& config_file_path, const string& desired_config) {
    INIReader reader(config_file_path);
    if (reader.ParseError() < 0) {
        cerr << "Error loading config file\n";
        return false;
    }
    cluster_num = reader.GetInteger(desired_config, "cluster_num", 0);
    for(int i=0; i<cluster_num; i++)  {
        istringstream coordinates(reader.Get(desired_config, "centroid"+ to_string(i), ""));
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
    int cluster_num;
    if (!initialize_centroids(centroids,cluster_num,CONFIG_FILE_PATH,DESIRED_CONFIG)) return -1;
    vector<int> total_clusters_size(cluster_num);


    print_centroids(centroids);

    auto start_time = high_resolution_clock::now();
#pragma omp parallel num_threads(THREAD_NUMBER) default(none) shared(dataPoints,centroids,cluster_num,cout,total_clusters_size)
    {
        for (int iteration = 0; iteration < ITERATION_NUMBER; iteration++) {
#pragma omp master
            cout << endl << "Iteration " << iteration + 1 << ":" << endl;

            DataPoints new_centroids;
            vector<int> clusters_size(cluster_num);
            vector<float> default_coordinate(cluster_num);
            new_centroids.xs.insert(new_centroids.xs.end(), default_coordinate.begin(), default_coordinate.end());
            new_centroids.ys.insert(new_centroids.ys.end(), default_coordinate.begin(), default_coordinate.end());
            new_centroids.zs.insert(new_centroids.zs.end(), default_coordinate.begin(), default_coordinate.end());

#pragma omp for schedule(static)
            for (int i = 0; i < dataPoints.xs.size(); i++) {
                float shortest_distance = sqrt(
                        pow(centroids.xs[0] - dataPoints.xs[i], 2) + pow(centroids.ys[0] - dataPoints.ys[i], 2) +
                        pow(centroids.zs[0] - dataPoints.zs[i], 2));
                int cluster_type = 0;
                for (int j = 1; j < cluster_num; j++) {
                    float centroid_distance = sqrt(
                            pow(centroids.xs[j] - dataPoints.xs[i], 2) + pow(centroids.ys[j] - dataPoints.ys[i], 2) +
                            pow(centroids.zs[j] - dataPoints.zs[i], 2));
                    if (centroid_distance < shortest_distance) {
                        shortest_distance = centroid_distance;
                        cluster_type = j;
                    }
                }
                new_centroids.xs[cluster_type] += dataPoints.xs[i];
                new_centroids.ys[cluster_type] += dataPoints.ys[i];
                new_centroids.zs[cluster_type] += dataPoints.zs[i];
                clusters_size[cluster_type]++;
            }

#pragma omp single
            {
                for(int i = 0; i < cluster_num; i++){
                    centroids.xs[i] = 0;
                    centroids.ys[i] = 0;
                    centroids.zs[i] = 0;
                }
            }

            for (int i = 0; i < cluster_num; i++) {
#pragma omp critical (sum_xs)
                centroids.xs[i] += new_centroids.xs[i];
#pragma omp critical (sum_ys)
                centroids.ys[i] += new_centroids.ys[i];
#pragma omp critical (sum_zs)
                centroids.zs[i] += new_centroids.zs[i];
#pragma omp critical (sum_cluster_size)
                total_clusters_size[i] += clusters_size[i];
            }
#pragma omp barrier
#pragma omp single
            {
                cout << endl;
                for (int i = 0; i < cluster_num; i++) {
                    cout << "Cluster" << i + 1 << " size: " << total_clusters_size[i] << endl;
                    centroids.xs[i] = centroids.xs[i]/total_clusters_size[i];
                    centroids.ys[i] = centroids.ys[i]/total_clusters_size[i];
                    centroids.zs[i] = centroids.zs[i]/total_clusters_size[i];
                    total_clusters_size[i] = 0;
                }
                cout << endl;
                print_centroids(centroids);
            }

            /*for (int i=0; i < dataPoints.xs.size(); i++) {
                float shortest_distance = sqrt(pow(centroids.xs[0] - dataPoints.xs[i], 2) + pow(centroids.ys[0] - dataPoints.ys[i], 2) + pow(centroids.zs[0] - dataPoints.zs[i], 2));
                int cluster_type = 0;
                for (int j=1; j<CLUSTERS_NUMBER; j++) {
                    float centroid_distance = sqrt(pow(centroids.xs[j] - dataPoints.xs[i], 2) + pow(centroids.ys[j] - dataPoints.ys[i], 2) + pow(centroids.zs[j] - dataPoints.zs[i], 2));
                    if (centroid_distance < shortest_distance) {
                        shortest_distance = centroid_distance;
                        cluster_type = j;
                    }
                }
                clusters[cluster_type].xs.push_back(dataPoints.xs[i]);
                clusters[cluster_type].ys.push_back(dataPoints.ys[i]);
                clusters[cluster_type].zs.push_back(dataPoints.zs[i]);
            }

            cout << endl;
            for (int i=0; i<clusters.size(); i++) {
                cout << "Cluster size" << i+1 << ": " << clusters[i].xs.size() << endl;
            }

            for (int i=0; i<CLUSTERS_NUMBER; i++) {
                float new_centroid_x = 0;
                float new_centroid_y = 0;
                float new_centroid_z = 0;
                int cluster_size = clusters[i].xs.size();
                for (int j=0; j<cluster_size; j++) {
                    new_centroid_x += clusters[i].xs[j];
                    new_centroid_y += clusters[i].ys[j];
                    new_centroid_z += clusters[i].zs[j];
                }
                centroids.xs[i] = new_centroid_x/cluster_size;
                centroids.ys[i] = new_centroid_y/cluster_size;
                centroids.zs[i] = new_centroid_z/cluster_size;
            }

            for (int i=0; i<CLUSTERS_NUMBER; i++) {
                clusters[i].xs.clear();
                clusters[i].ys.clear();
                clusters[i].zs.clear();
            }*/
        }
    }
    auto end_time = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(end_time - start_time).count() / 1000.f;
    cout << "Duration: " << time << " ms" << endl;

    return 0;
}
#include <omp.h>
#include <iostream>
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

static const int CLUSTERS_NUMBER = 4;
static const int ITERATION_NUMBER = 10;

struct DataPoints {
    std::vector<float> xs;
    std::vector<float> ys;
    std::vector<float> zs;
};

void readDatasetFromFile(DataPoints& dataset, const string& fullPath) {
    ifstream file(fullPath);
    if (file.is_open()) {
        string line;
        cout << "Reading the dataset..." << endl;
        while (getline(file, line)) {
            istringstream iss(line);
            float x;
            float y;
            float z;
            char delimiter1;
            char delimiter2;
            if (iss >> x >> delimiter1 >> y >> delimiter2 >> z) {
                dataset.xs.push_back(x);
                dataset.ys.push_back(y);
                dataset.zs.push_back(z);
            }/* else {
                cerr << "Error: Invalid line in file " << fullPath << endl;
            }*/
        }
        file.close();
        cout << "Dataset loaded from " << fullPath << endl;
    } else {
        cerr << "Error: Unable to open file " << fullPath << endl;
    }
}

int main() {

    DataPoints dataPoints;
    readDatasetFromFile(dataPoints, "C:/Users/jaco3/OneDrive/Documenti/Unifi/Magistrale/ParallelProgramming/Elaborati/k-means/datasets/generated_blob_dataset_40k.csv");

    DataPoints centroids;
    vector<DataPoints> clusters(CLUSTERS_NUMBER);
    vector<float> centroid_xs = {10,10,10,10};
    vector<float> centroid_ys = {6,3,-3,-6};
    vector<float> centroid_zs = {0,0,0,0};
    centroids.xs.insert(centroids.xs.end(),centroid_xs.begin(),centroid_xs.end());
    centroids.ys.insert(centroids.ys.end(),centroid_ys.begin(),centroid_ys.end());
    centroids.zs.insert(centroids.zs.end(),centroid_zs.begin(),centroid_zs.end());

    for (int i=0; i<CLUSTERS_NUMBER; i++) {
        cout << "(" << centroids.xs[i] << ", " << centroids.ys[i] << ", " << centroids.zs[i] << ")" << endl;
    }

    auto start_time = high_resolution_clock::now();
#pragma omp parallel num_threads(5)
    {
        for (int iteration = 0; iteration < ITERATION_NUMBER; iteration++) {
            cout << endl << "Iteration " << iteration + 1 << ":" << endl;

            DataPoints new_centroids;
            vector<int> clusters_size(CLUSTERS_NUMBER);
            vector<float> default_coordinate = {0, 0, 0, 0};
            new_centroids.xs.insert(new_centroids.xs.end(), centroid_xs.begin(), centroid_xs.end());
            new_centroids.ys.insert(new_centroids.ys.end(), centroid_ys.begin(), centroid_ys.end());
            new_centroids.zs.insert(new_centroids.zs.end(), centroid_zs.begin(), centroid_zs.end());

            for (int i = 0; i < dataPoints.xs.size(); i++) {
                float shortest_distance = sqrt(
                        pow(centroids.xs[0] - dataPoints.xs[i], 2) + pow(centroids.ys[0] - dataPoints.ys[i], 2) +
                        pow(centroids.zs[0] - dataPoints.zs[i], 2));
                int cluster_type = 0;
                for (int j = 1; j < CLUSTERS_NUMBER; j++) {
                    float centroid_distance = sqrt(
                            pow(centroids.xs[j] - dataPoints.xs[i], 2) + pow(centroids.ys[j] - dataPoints.ys[i], 2) +
                            pow(centroids.zs[j] - dataPoints.zs[i], 2));
                    if (centroid_distance < shortest_distance) {
                        shortest_distance = centroid_distance;
                        cluster_type = j;
                    }
                }
                //parte problematica per la parallelizzazione perché ci vuole sincronia
                new_centroids.xs[cluster_type] += dataPoints.xs[i];
                new_centroids.ys[cluster_type] += dataPoints.ys[i];
                new_centroids.zs[cluster_type] += dataPoints.zs[i];
                clusters_size[cluster_type]++;
            }

            for (int i = 0; i < CLUSTERS_NUMBER; i++) {
                centroids.xs[i] = new_centroids.xs[i] / clusters_size[i];
                centroids.ys[i] = new_centroids.ys[i] / clusters_size[i];
                centroids.zs[i] = new_centroids.zs[i] / clusters_size[i];
            }

            cout << endl;
            for (int i = 0; i < CLUSTERS_NUMBER; i++) {
                cout << "Cluster" << i + 1 << " size: " << clusters_size[i] << endl;
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

            cout << endl;
            for (int i = 0; i < CLUSTERS_NUMBER; i++) {
                cout << "(" << centroids.xs[i] << ", " << centroids.ys[i] << ", " << centroids.zs[i] << ")" << endl;
            }
        }
    }
    auto end_time = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(end_time - start_time).count() / 1000.f;
    cout << "Duration: " << time << " ms" << endl;

    return 0;
}
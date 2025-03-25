//Vidám Szabolcs 514/2 vsim2414

#include <iostream>
#include <climits>
#include <queue>
#include <stack>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

#define RED_COEFFICIENT 0.2989
#define GREEN_COEFFICIENT 0.5870
#define BLUE_COEFFICIENT 0.1140

int ConvertToGrayscale(Vec3b pixel)
{
    int red = (int)pixel[2];
    int green = (int)pixel[1];
    int blue = (int)pixel[0];

    int gray = RED_COEFFICIENT * red + GREEN_COEFFICIENT * green + BLUE_COEFFICIENT * blue;
    return gray;
}

int CalcCapacity(Vec3b pixel1, Vec3b pixel2)
{
    const int p1 = ConvertToGrayscale(pixel1);
    const int p2 = ConvertToGrayscale(pixel2);

    if (p1 == p2) return 255;
    return pow(255 / abs(p1 - p2), 2);
}

void LoadImage(Mat& image, int& nodeNum, vector<vector<int>>& adjMatrix)
{
    // Load PNG image
    image = imread("test.png", IMREAD_UNCHANGED);

    if (image.empty())
    {
        cerr << "Error: Unable to load image." << endl;
        exit(1);
    }

    nodeNum = image.rows * image.cols + 2;
    adjMatrix.resize(nodeNum, vector<int>(nodeNum, 0));

    int currentPixel = 0;
    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            Vec3b pixel1 = image.at<Vec3b>(i, j);

            if (j + 1 != image.cols)
            {
                Vec3b pixel2 = image.at<Vec3b>(i, j + 1);
                adjMatrix[currentPixel][currentPixel + 1] = CalcCapacity(pixel1, pixel2);
            }

            if (i + 1 != image.rows)
            {
                Vec3b pixel2 = image.at<Vec3b>(i + 1, j);
                adjMatrix[currentPixel][currentPixel + image.cols] = CalcCapacity(pixel1, pixel2);
            }

            currentPixel++;
        }
    }

    int source = image.rows * image.cols;
    int sink = image.rows * image.cols + 1;
    for (int k = 0; k < image.cols; k++)
    {
        adjMatrix[source][k] = INT_MAX;
        adjMatrix[(image.rows - 1) * image.cols + k][sink] = INT_MAX;
    }
}

bool BreadthFirstSearch(vector<vector<int>> adjMatrix, int nodeNum, int source, int sink, vector<int>& parent)
{
    vector<bool> visited(nodeNum, false);

    queue<int> q;
    q.push(source);
    visited[source] = true;
    parent[source] = -1;

    while (!q.empty())
    {
        int current = q.front();
        q.pop();

        for (int i = 0; i < nodeNum; i++)
        {
            if (adjMatrix[current][i] != 0 && !visited[i])
            {
                if (i == sink)
                {
                    parent[i] = current;
                    return true;
                }
                q.push(i);
                visited[i] = true;
                parent[i] = current;
            }
        }
    }

    return false; //can't reach sink
}

void DepthFirstSearch(vector<vector<int>> adjMatrix, int nodeNum, int source, vector<bool>& visited)
{
    stack<int> s;
    s.push(source);
    visited[source] = true;

    while (!s.empty())
    {
        int current = s.top();
        s.pop();

        for (int i = 0; i < nodeNum; i++)
        {
            if (adjMatrix[current][i] != 0 && !visited[i])
            {
                s.push(i);
                visited[i] = true;
            }
        }
    }
}

vector<bool> FordFulkerson(vector<vector<int>>& adjMatrix, int nodeNum, int source, int sink)
{
    vector<vector<int>> residualGraph = adjMatrix;

    vector<int> parent(nodeNum);
    int max_flow = 0;

    while (BreadthFirstSearch(residualGraph, nodeNum, source, sink, parent))
    {
        int path_flow = INT_MAX;
        int current = sink;
        int aux;
        while (current != source)
        {
            aux = parent[current];
            path_flow = min(path_flow, residualGraph[aux][current]);
            current = parent[current];
        }

        current = sink;
        while (current != source)
        {
            aux = parent[current];
            residualGraph[aux][current] -= path_flow;
            residualGraph[current][aux] += path_flow;
            current = parent[current];
        }

        max_flow += path_flow;
    }

    cout << max_flow << endl;
    vector<bool> visited(nodeNum, false);
    DepthFirstSearch(residualGraph, nodeNum, source, visited);
    cout << "Minimum Cut Vertices:" << endl;
    for (int i = 0; i < nodeNum; i++)
    {
        if (visited[i])
        {
            cout << i << " ";
        }
    }
    cout << endl;

    return visited;
}

void SaveImage(Mat image, vector<bool>& visited)
{
    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            if (visited[i * image.rows + j])
            {
                image.at<Vec4b>(i, j) = Vec4b(0, 0, 0, 255); //black
            }
            else
            {
                image.at<Vec4b>(i, j) = Vec4b(255, 255, 255, 255); //white
            }
        }
    }

    imwrite("result.png", image);
}

int main()
{
    Mat image;
    int nodeNum;
    vector<vector<int>> adjMatrix;
    LoadImage(image, nodeNum, adjMatrix);

    vector<bool> visited = FordFulkerson(adjMatrix, nodeNum, nodeNum - 2, nodeNum - 1);

    SaveImage(image, visited);

    return 0;
}
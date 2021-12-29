/*
* IPA 2020/2021 – Projekt – varianta 1
* Autor (předloha):     Tomáš Goldmann, igoldmann@fit.vutbr.cz
* Autor (implementace): Ondřej Ondryáš, xondry02@stud.fit.vutbr.cz
*/

#include <opencv2/world.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <intrin.h>
#include <inttypes.h>
#include <windows.h>
#include <random>
#include <cstdlib>
#include "ipa_tool.h"

#define WIN_WIDTH 800.0f
#define PROJECT_NAME "IPA projekt 2020 – varianta 1"
#define DEBUG 0
#define USE_C_KMEANS 0
#define USE_C_IMGPROC 0

using namespace cv;
using namespace std;

struct centroid
{
	float R;
	float G;
	float B;

	float empty; // alignment

	float newR;
	float newG;
	float newB;

	float newPointsCount;
};

centroid *createCentroidMemory(int centroidCount, mt19937 &rnd)
{
	if (centroidCount < 2)
		centroidCount = 2;

	centroid *ptr = static_cast<centroid *>(_aligned_malloc(centroidCount * sizeof(centroid), 16));

	uniform_int_distribution distrib(0, 255);

	for (int i = 0; i < centroidCount; i++)
	{
		ptr[i].B = distrib(rnd);
		ptr[i].G = distrib(rnd);
		ptr[i].R = distrib(rnd);
		ptr[i].empty = 0;
		ptr[i].newR = 0;
		ptr[i].newG = 0;
		ptr[i].newB = 0;
		ptr[i].newPointsCount = 0;
	}

	return ptr;
}

#if DEBUG == 1
centroid *centroidArrayGlob;
extern "C"
{
	void printCentroid(centroid *c, int x)
	{
		if (c == NULL)
		{
			printf("Null centroid why?\n");
			return;
		}
		printf("#%d:\nR: %d\nG: %d\nB: %d\nNewR: %d\nNewG: %d\nNewB: %d\nPC: %d\n", x, (int)c->R, (int)c->G, (int)c->B, (int)c->newR, (int)c->newG, (int)c->newB, (int)c->newPointsCount);
		fflush(stdout);
	}

	void printCentroids(long pixel)
	{
		static int counter = 0;

		if (counter++ > 506880)
		{
			counter = 506881;
			return;
		}

		printf("After pixels %ld to %ld:\n", pixel, pixel + 3);
		for (int i = 0; i < 8; i++)
		{
			printCentroid(centroidArrayGlob + i, i);
		}
	}
}
#endif

#if USE_C_KMEANS == 0
extern "C" void algorithm_kmeans(uchar *imageData, centroid *centroidMemory, long centroidCount, int *results, long pixelCount);
#else
void algorithm_kmeans(uchar *imageData, centroid *centroidMemory, int centroidCount, int *results, long pixelCount)
{
	centroid *mem = centroidMemory;
	cout << "Running the unoptimised version!\n";

	/*
	For each pixel:
		- keep pair centroidIndex, distance for minimal centroid
		- calculate distance for each centroid (stored in centroidMemory)
		- if distance is less then minimal distance, update values
	*/

	int a = 0;
	while (true)
	{
		bool hadChange = false;

		for (int p = 0; p < pixelCount; p++)
		{
			int minCentroidI = 0;
			float minDistance = 999999;

			float pB = (float)imageData[p * 3];
			float pG = (float)imageData[p * 3 + 1];
			float pR = (float)imageData[p * 3 + 2];

			for (int ci = 0; ci < centroidCount; ci++)
			{
				float cR = mem[ci].R;
				float cG = mem[ci].G;
				float cB = mem[ci].B;

				float distance = sqrt((pR - cR) * (pR - cR) + (pG - cG) * (pG - cG) + (pB - cB) * (pB - cB));
				if (distance < minDistance)
				{
					minDistance = distance;
					minCentroidI = ci;
				}
			}

			mem[minCentroidI].newR += pR;
			mem[minCentroidI].newG += pG;
			mem[minCentroidI].newB += pB;
			mem[minCentroidI].newPointsCount += 1;
			results[p] = minCentroidI;

			/*
			// Debugging code – prints out the current values of the centroids
			if (p % 4 == 3) {
				printCentroids(p - 3);
			}
			*/
		}

		for (int ci = 0; ci < centroidCount; ci++)
		{
			centroid &c = mem[ci];

			if (c.newPointsCount == 0)
			{
				// c.newPointsCount = 1;
				continue;
			}

			c.newR /= c.newPointsCount;
			c.newG /= c.newPointsCount;
			c.newB /= c.newPointsCount;

			int oldR = (int)c.R;
			int oldG = (int)c.G;
			int oldB = (int)c.B;

			int newR = (int)c.newR;
			int newG = (int)c.newG;
			int newB = (int)c.newB;

			hadChange |= oldR != newR || oldB != newB || oldG != newG;

			c.R = c.newR;
			c.G = c.newG;
			c.B = c.newB;
			c.newR = 0;
			c.newG = 0;
			c.newB = 0;
			c.newPointsCount = 0;
		}

		//printCentroids(a++);

		if (!hadChange)
		{
			break;
		}
	}
}
#endif

#if USE_C_IMGPROC == 0
extern "C" void algorithm_imgproc(uchar *imageData, uchar *outputData, int *results, int centroidIndex, long pixelCount);
#else
void algorithm_imgproc(uchar *imageData, uchar *outputData, int *results, int centroidIndex, long pixelCount)
{
	for (int p = 0; p < pixelCount; p++)
	{
		if (results[p] != centroidIndex)
		{
			float g = 0.299 * imageData[p * 3 + 2] + 0.587 * imageData[p * 3 + 1] + 0.114 * imageData[p * 3];
			if (g > 255.0f)
				g = 255.0f;

			uchar gI = (uchar)g;
			outputData[p * 3] = gI;
			outputData[p * 3 + 1] = gI;
			outputData[p * 3 + 2] = gI;
		}
		else
		{
			memcpy(outputData + p * 3, imageData + p * 3, 3);
		}
	}
}
#endif

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		cout << "Usage: " << argv[0] << " input_image centroids_count centroid_index [randomization_seed] [output_image]" << std::endl;
		return -1;
	}

	int centroidCount = 3;
	int centroidIndex = 0;

	if (argc >= 3)
	{
		try
		{
			centroidCount = stoi(argv[2]);
		}
		catch (invalid_argument)
		{
			cout << "Invalid centroid count." << endl;
			return 1;
		}

		if (centroidCount < 2)
		{
			cout << "Two centroids will be used." << endl;
			centroidCount = 2;
		}
	}

	cout << "Using centroid count " << centroidCount << endl;

	if (argc >= 4)
	{
		try
		{
			centroidIndex = stoi(argv[3]);
		}
		catch (invalid_argument)
		{
			cout << "Invalid centroid index." << endl;
			return 1;
		}

		if (centroidIndex < 0 || centroidIndex >= centroidCount)
		{
			cout << "Invalid centroid index." << endl;
			return 1;
		}
	}

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	mt19937 randomGen(seed);

	if (argc >= 5)
	{
		try
		{
			randomGen = mt19937(stoi(argv[4]));
		}
		catch (invalid_argument)
		{
			cout << "Ignoring invalid seed." << endl;
		}
	}

	cout << "Using target centroid index " << centroidIndex << endl
		 << endl;

	Mat image = imread(argv[1], IMREAD_COLOR);

	if (!image.data)
	{
		cout << "Could not open or find the image '" << argv[1] << "'" << endl;
		return 2;
	}

	// Resize by width
	cv::resize(image, image, cv::Size(640, image.rows * ((float)640 / image.cols)));

	auto size = image.total() * image.elemSize();
	if (size % 8 != 0)
	{
		size += 8 - (size % 8);
	}

	uchar *outData = new uchar[size];

	InstructionCounter counter;

	int pixelCount = image.cols * image.rows;

	auto centroidPtr = createCentroidMemory(centroidCount, randomGen);
	int *resultsPtr = static_cast<int *>(_aligned_malloc(sizeof(int) * pixelCount, 16));

#if DEBUG == 1
	centroidArrayGlob = centroidPtr;
#endif

	counter.start();
	algorithm_kmeans(image.data, centroidPtr, centroidCount, resultsPtr, pixelCount);
	cout << "K-means cycles: ";
	cerr << counter.getCyclesCount() << endl;
	counter.print();

	counter.start();
	algorithm_imgproc(image.data, outData, resultsPtr, centroidIndex, pixelCount);
	cout << "Image processing cycles: ";
	cerr << counter.getCyclesCount() << endl;
	counter.print();

	Mat output(image.rows, image.cols, CV_8UC3, outData);

	if (argc >= 6)
	{
		cv::imwrite(argv[5], output);
	}
	else
	{
		cv::imshow("Projekt – výstup", output);
	}

	_aligned_free(centroidPtr);
	_aligned_free(resultsPtr);

	waitKey(0);
	output.release();
	return 0;
}
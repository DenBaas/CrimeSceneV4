#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <time.h>
#include <ctime>
#include "MapObject.h"

using namespace std;

enum DataFormat{
	DEBUG = 0,
	READABLE = 1
};

class ResultsFileIO
{
public:
	ResultsFileIO();
	~ResultsFileIO();

	int saveEvidenceItems(const DataFormat format, const std::vector<MapObject*> * retrievedObjects);
	int saveRouteAndActions(const DataFormat format);
private:
	string directory;
};


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

class InventoryFileIO
{
public:
	InventoryFileIO();
	~InventoryFileIO();

	int save(const DataFormat format, const std::vector<MapObject*> * retrievedObjects);
private:
	string directory;
};


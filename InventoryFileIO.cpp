#include "InventoryFileIO.h"


InventoryFileIO::InventoryFileIO()
{
	directory = "";
}


InventoryFileIO::~InventoryFileIO()
{
}


int InventoryFileIO::save(const DataFormat format, const std::vector<MapObject*> * retrievedObjects){
	std::ofstream outputFile;
	time_t now = time(NULL);
	outputFile.open(directory + "//" + std::to_string(now) +".txt");
	if (!outputFile.is_open())
		return -1;
	switch (format)
	{
	case DataFormat::DEBUG:
		for each (MapObject* evidence in *retrievedObjects)
		{
			outputFile << evidence->getDescription()+ "\n";
		}
		break;
	case DataFormat::READABLE:

		break;
	default:
		break;
	}
	outputFile.close();
	return 1;
}

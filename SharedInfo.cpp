#include "SharedInfo.h"


SharedInfo::SharedInfo()
{
}

void SharedInfo::writeObject(BinaryStream &writer)
{
	writer << position << rotation;
}

void SharedInfo::readObject(BinaryStream &reader)
{
	reader >> position >> rotation;
}

int SharedInfo::getEstimatedSize()
{
	return (1024 * 1024 * 6) ;
}


SharedInfo::~SharedInfo()
{
}

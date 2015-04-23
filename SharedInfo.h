#pragma once
#include <glm/glm.hpp>
#include <VrLib/BinaryStream.h>

class SharedInfo : public SerializableObject
{
public:
	glm::vec4 position;
	float rotation;
	SharedInfo();
	~SharedInfo();
	int getEstimatedSize();
	void writeObject(BinaryStream &writer);
	void readObject(BinaryStream &reader);
};


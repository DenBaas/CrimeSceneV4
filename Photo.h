#pragma once

#include <gl/glew.h>
#include <windows.h>
#include <vector>
#include <CaveLib\FBO.h>

class Photo
{
	FBO *fbo;
	int width;
	int height;
	int photosMade;

	// Name of the folder where the Crime Scene pictures are to be added
	std::wstring outputFolder = L"Crime scene pictures"; 

	void Photo::writeToPng(int width, int height, std::vector<float> pixels);

public:
	Photo::Photo(int width, int height, bool depth = true);
	~Photo();

	void generateImage();

	void bind();
	void unbind();
};

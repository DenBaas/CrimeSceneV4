#pragma once

#include <gl/glew.h>
#include <windows.h>
#include <vector>
#include <CaveLib\FBO.h>
#include <ctime>

class Photo
{
	FBO *fbo;
	int width;
	int height;
	int photosMade;

	// Name of the folder where the Crime Scene pictures are to be added
	

	

public:
	std::wstring outputFolder = L"Crime scene resultaten";

	Photo::Photo(bool depth = true);
	~Photo();

	void generateImage();
	void Photo::writeToPng(std::vector<float> pixels);

	void bind();
	void unbind();
};

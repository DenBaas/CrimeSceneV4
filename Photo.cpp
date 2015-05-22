#include "Photo.h"

#include <string>
#include <iostream>
#include <thread>
#include <libpng\png++\png.hpp>

#include <VrLib\Kernel.h>

using namespace std;
/*
Constructor for Photo. Initializes an FBO for creating pictures.
width = prefered width of the photo;
height = prefered height of the photo;
depth = optional; toggle of depth buffering.
Author: Ricardo Blommers - 05-2014
Last edit: <name> - dd-mm-yyyy
*/
Photo::Photo(bool depth)
{
	Kernel * kernel = Kernel::getInstance();
	width = kernel->getWindowWidth();
	height = kernel->getWindowHeight();
	fbo = new FBO(width, height, depth);
	photosMade = 0;
	this->width = width;
	this->height = height;

	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y %m %d - %H %M %S", &tstruct);
	std::string timeStarted = buf;
	outputFolder = outputFolder + L"\\" + std::wstring(timeStarted.begin(),timeStarted.end());
}

/*
Destructor
Author: Ricardo Blommers - 05-2014
Last edit: <name> - dd-mm-yyyy
*/
Photo::~Photo()
{
}

/*
Generates an image based on the information contained in the FBO.
Preferably called after binding and unbinding of the FBO.
Author: Ricardo Blommers - 05-2014
Last edit: <name> - dd-mm-yyyy
*/
void Photo::generateImage()
{
	Kernel * kernel = Kernel::getInstance();
	if (width != kernel->getWindowWidth() || height != kernel->getWindowHeight()){
		width = kernel->getWindowWidth();
		height = kernel->getWindowHeight();
		fbo = new FBO(width, height);
	}
	std::vector<float> pixels(width * height * 4);

	bind();
	glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, &pixels[0]);
	unbind();
	thread saveAsync([&](Photo * p, std::vector<float> * p2){p->writeToPng(*p2); }, this, &pixels);
	saveAsync.join();
}

/*
Writes a vector of pixels to a png-file.
width = prefered width of the screenshot;
height = the prefered height of the screenshot;
pixels = a vector of floats containing information of an image.
Author: Ricardo Blommers - 05-2014
Last edit: Ricardo Blommers - 25-06-2014
*/
void Photo::writeToPng(std::vector<float> pixels)
{
	std::string filename = "Crime_Scene_Picture_";

	// Comment the next line of code out to overwrite the same file;
	// This helps with testing as Windows Preview automatically
	// updates when a new picture is taken.
	filename.append(std::to_string(photosMade));
	filename.append(".png");
	photosMade++;

	std::cout << "Creating photo: " << filename << std::endl;

	png::image<png::rgb_pixel> image(width, height);
	for (int i = 0; i < pixels.size(); i += 4)
	{
		// The data is always between 0 and 1. Multiply by 255 to get RGB values.
		float red = pixels[i] * 255;
		float green = pixels[i + 1] * 255;
		float blue = pixels[i + 2] * 255;

		// glReadPixel, which is the origin of the data, starts bottom left with
		// the image data. The method 'set_pixel', however, starts top left.
		// Ergo, flip rows while keeping width/collumns consistent.
		int deltaWidth = ((i / 4) % width);
		int row = ((i / 4) / width);

		png::rgb_pixel pixelColour = png::rgb_pixel(red, green, blue);

		image.set_pixel(deltaWidth, height-row-1, pixelColour);
	}

	// A rudimentary check to see if a directory exists, and if not, creates one.
	if (CreateDirectory(outputFolder.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
	{
		// Bizarrely, CreateDirectory requires a LPCWSTR, which is incompatible with 
		// the std::string required to write the image. While wstring is kind of
		// capable of converting to either, it can't do it by itself. What the heck, C++?
		// If you can simplify this, please do so.
		std::string outputPath(outputFolder.begin(), outputFolder.end());
		outputPath.append("\\" + filename);
		image.write(outputPath); 
	}
	else
	{
		// Failed to create directory.
		std::cout << "Error creating directory - aborting writing image" << std::endl;
	}

}

/*
Binds the internal FBO.
Author: Ricardo Blommers - 05-2014
Last edit: <name> - dd-mm-yyyy
*/
void Photo::bind()
{
	fbo->bind();
}

/*
Unbinds the internal FBO.
Author: Ricardo Blommers - 05-2014
Last edit: <name> - dd-mm-yyyy
*/
void Photo::unbind()
{
	fbo->unbind();
}

#pragma once
class GameInfo
{
public:
	int gamemode = 0;//zie indeling knoppen op dropbox wat waar voor staat
	int MAXMODES = 3;
	float rotationHorizontal = 0;
	float rotationVertical= 0;
	float zoomfactor = 0;
	bool flashlightEnabled = false;
	bool takeScreenshot = false;
	int status;//

	GameInfo();
	~GameInfo();
};


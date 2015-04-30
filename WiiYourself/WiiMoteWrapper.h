#pragma once
#include "wiimote.h"
class WiiMoteWrapper
{
public:
	bool continueGame;	
	void start();
	int status;
	wiimote_state::buttons buttonsPressed;
	wiimote_state::nunchuk nunchukInfo;

	WiiMoteWrapper();
	~WiiMoteWrapper();
private:
	int wiiMoteMainStart();
};


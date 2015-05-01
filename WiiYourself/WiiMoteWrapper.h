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
	int batteryLevel;
	bool WIIMOTE = false,
		NUNCHUK = false,
		MOTIONPLUS = false;

	void setleds(int state);
	void rumble(int duration);
	void enableRumble(bool enable);
	void enableSound(bool enable);
	
	WiiMoteWrapper();
	~WiiMoteWrapper();
private:
	int wiiMoteMainStart();
};



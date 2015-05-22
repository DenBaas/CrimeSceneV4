#pragma once
#include "wiimote.h"
#include "../GameInfo.h"
class WiiMoteWrapper
{
public:
	bool continueGame;
	void start();
	wiimote_state::buttons buttonsPressed;
	wiimote_state::buttons previousButtons;
	wiimote_state::nunchuk nunchukInfo;
	int batteryLevel;
	bool WIIMOTE = false,
		NUNCHUK = false,
		MOTIONPLUS = false;

	GameInfo * infoForGame;

	void setleds(int state);
	void rumble(int duration);
	void enableRumble(bool enable);
	void enableSound(bool enable);
	
	WiiMoteWrapper(GameInfo * g);
	~WiiMoteWrapper();
private:
	int wiiMoteMainStart();
};



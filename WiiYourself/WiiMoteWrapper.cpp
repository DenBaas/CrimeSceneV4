#include "WiiMoteWrapper.h"
#include <mmsystem.h>

using namespace std;

WiiMoteWrapper::WiiMoteWrapper(GameInfo * g)
{
	continueGame = true;
	infoForGame = g;
	infoForGame->status = 0; //0 = wachten 1 = succes -1 = niet succes	
}

void WiiMoteWrapper::start(){
	wiiMoteMainStart();

}

WiiMoteWrapper::~WiiMoteWrapper()
{
	
}



// configs:
#define USE_BEEPS_AND_DELAYS			// undefine to test library works without them
#define LOOK_FOR_ADDITIONAL_WIIMOTES	// tries to connect any extra wiimotes

// ------------------------------------------------------------------------------------
//  state-change callback example (we use polling for everything else):
// ------------------------------------------------------------------------------------
void on_state_change(wiimote			  &remote,
	state_change_flags  changed,
	const wiimote_state &new_state)
{
	// we use this callback to set report types etc. to respond to key events
	//  (like the wiimote connecting or extensions (dis)connecting).

	// NOTE: don't access the public state from the 'remote' object here, as it will
	//		  be out-of-date (it's only updated via RefreshState() calls, and these
	//		  are reserved for the main application so it can be sure the values
	//		  stay consistent between calls).  Instead query 'new_state' only.

	// the wiimote just connected
	if (changed & CONNECTED)
	{
		// ask the wiimote to report everything (using the 'non-continous updates'
		//  default mode - updates will be frequent anyway due to the acceleration/IR
		//  values changing):

		// note1: you don't need to set a report type for Balance Boards - the
		//		   library does it automatically.

		// note2: for wiimotes, the report mode that includes the extension data
		//		   unfortunately only reports the 'BASIC' IR info (ie. no dot sizes),
		//		   so let's choose the best mode based on the extension status:
		remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT); // no IR dots
	}
	// a MotionPlus was detected
	if (changed & MOTIONPLUS_DETECTED)
	{
		// enable it if there isn't a normal extension plugged into it
		// (MotionPlus devices don't report like normal extensions until
		//  enabled - and then, other extensions attached to it will no longer be
		//  reported (so disable the M+ when you want to access them again).
		if (remote.ExtensionType == wiimote_state::NONE) {
			bool res = remote.EnableMotionPlus();
			_ASSERT(res);
		}
	}
	// an extension is connected to the MotionPlus
	else if (changed & MOTIONPLUS_EXTENSION_CONNECTED)
	{
		// We can't read it if the MotionPlus is currently enabled, so disable it:
		if (remote.MotionPlusEnabled())
			remote.DisableMotionPlus();
	}
	// an extension disconnected from the MotionPlus
	else if (changed & MOTIONPLUS_EXTENSION_DISCONNECTED)
	{
		// enable the MotionPlus data again:
		if (remote.MotionPlusConnected())
			remote.EnableMotionPlus();
	}
	// another extension was just connected:
	else if (changed & EXTENSION_CONNECTED)
	{
#ifdef USE_BEEPS_AND_DELAYS
		//Beep(1000, 200);
#endif
		remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT);
	}
	// extension was just disconnected:
	else if (changed & EXTENSION_DISCONNECTED)
	{
#ifdef USE_BEEPS_AND_DELAYS
		//Beep(200, 300);
#endif
	}
}

int WiiMoteWrapper::wiiMoteMainStart()
{
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	// create a wiimote object
	wiimote remote;

	// in this demo we use a state-change callback to get notified of
	//  extension-related events, and polling for everything else
	// (note you don't have to use both, use whatever suits your app):
	remote.ChangedCallback = on_state_change;
	//  notify us only when the wiimote connected sucessfully, or something
	//   related to extensions changes
	remote.CallbackTriggerFlags = (state_change_flags)(CONNECTED | EXTENSION_CHANGED | MOTIONPLUS_CHANGED);
reconnect:
	unsigned count = 0;
	while (!remote.Connect(wiimote::FIRST_AVAILABLE) && continueGame) {
		count++;
#ifdef USE_BEEPS_AND_DELAYS
		//Beep(500, 30); 
		Sleep(1000);
#endif
	}
	if (!continueGame){
		return 0;
	}
	infoForGame->status = 1;
	WIIMOTE = true;
#ifdef USE_BEEPS_AND_DELAYS
	//Beep(1000, 300); 
	Sleep(2000);
#endif
	while (continueGame && !remote.ConnectionLost())
	{
		NUNCHUK = remote.NunchukConnected();
		MOTIONPLUS = remote.MotionPlusConnected();
		// IMPORTANT: the wiimote state needs to be refreshed each pass
		while (remote.RefreshState() == NO_CHANGE)
			Sleep(1); // // don't hog the CPU if nothing changed
		if (remote.ConnectionLost())
		{
			Sleep(2000);
			goto reconnect;
		}
		// TEMP: Minus button disables MotionPlus (if connected) to allow its
		//        own extension port to work
		/*static bool last_minus = false;
		if (remote.Button.Minus() && !last_minus &&
			(remote.ExtensionType == wiimote_state::MOTION_PLUS))
			remote.DisableMotionPlus();
		last_minus = remote.Button.Minus();*/
#define ON_PRESS_RELEASE(button, pressed_action, released_action)	\
			{ bool pressed = remote.Button.button();						\
			  if(pressed)													\
			  			  			  			 	{				   /* just pressed? */						\
				if(!last_##button) pressed_action;							\
			  			  			  				}															\
									else if(last_##button) /* just released */						\
				released_action;											\
			/* remember the current button state for next time */			\
			last_##button = pressed; }
		for (unsigned bit = 0; bit < 16; bit++)
		{
			WORD mask = (WORD)(1 << bit);
			// skip unused bits
			if ((wiimote_state::buttons::ALL & mask) == 0)
				continue;
			const TCHAR* button_name = wiimote::ButtonNameFromBit[bit];
			bool		 pressed = ((remote.Button.Bits & mask) != 0);
			previousButtons = buttonsPressed;
			buttonsPressed = remote.Button;
			nunchukInfo = remote.Nunchuk;
			batteryLevel = remote.BatteryRaw;
			if (remote.MotionPlusConnected()){
				if (remote.MotionPlusEnabled())
					remote.EnableMotionPlus();
				printf("%f\n", remote.MotionPlus.Raw.Pitch);
			}
		}
	}
	WIIMOTE = NUNCHUK = MOTIONPLUS = false;
	// disconnect (auto-happens on wiimote destruction anyway, but let's play nice)
	remote.Disconnect();
	CloseHandle(console);
	return 0;
}
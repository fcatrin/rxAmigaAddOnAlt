package org.ab.controls.vinput;

import android.view.KeyEvent;

public class GenericJoystick {
	public static String eventNames[] = { 
		"UP", "DOWN", "LEFT", "RIGHT", 
		"BTN_A", "BTN_B", "BTN_X", "BTN_Y", 
		"TL", "TR", "TL2", "TR2",
		"TL3", "TR3", "SELECT", "START"
	};

	public int originCodes[] = {
		KeyEvent.KEYCODE_DPAD_UP, KeyEvent.KEYCODE_DPAD_DOWN, KeyEvent.KEYCODE_DPAD_LEFT, KeyEvent.KEYCODE_DPAD_RIGHT,
		KeyEvent.KEYCODE_BUTTON_A, KeyEvent.KEYCODE_BUTTON_B, KeyEvent.KEYCODE_BUTTON_X, KeyEvent.KEYCODE_BUTTON_Y,
		KeyEvent.KEYCODE_BUTTON_L1, KeyEvent.KEYCODE_BUTTON_R1, KeyEvent.KEYCODE_BUTTON_L2, KeyEvent.KEYCODE_BUTTON_R2,
		KeyEvent.KEYCODE_BUTTON_THUMBL, KeyEvent.KEYCODE_BUTTON_THUMBR, KeyEvent.KEYCODE_BUTTON_SELECT, KeyEvent.KEYCODE_BUTTON_START
	};

	public int translatedCodes[] = new int[originCodes.length];

	public VirtualEvent virtualEvents[] = new VirtualEvent[eventNames.length];
	
	public GenericJoystick() {
		for(int i=0; i<originCodes.length; i++) translatedCodes[i] = originCodes[i];
	}
	
	public int getOriginCode(int keyCode) {
		for(int i=0; i<translatedCodes.length; i++) {
			if (translatedCodes[i] == keyCode) return originCodes[i];
		}
		return 0;
	}
	
}

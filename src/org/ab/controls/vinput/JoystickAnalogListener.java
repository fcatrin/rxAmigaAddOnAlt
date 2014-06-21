package org.ab.controls.vinput;

import org.ab.controls.vinput.JoystickAnalog.Axis;

public interface JoystickAnalogListener {
	public void onAxisChange(float axisx, float axisy);
	public void onMouseMove(int mousex, int mousey);
	public void onMouseMoveRelative(float mousex, float mousey);
	public void onDigitalX(Axis axis, boolean on);
	public void onDigitalY(Axis axis, boolean on);
}

package org.ab.controls.vinput;

import android.view.InputDevice;
import android.view.MotionEvent;

public class JoystickAnalog {
	public enum Axis {MIN, CENTER, MAX};
	float deadzone = 0.2f;
	
	Axis axisX = Axis.CENTER;
	Axis axisY = Axis.CENTER;
	
	static int maxGamepadX = 640;
	static int maxGamepadY = 480;
	static int lastGamepadX = 0;
	static int lastGamepadY = 0;
	static int gamepadX = 0;
	static int gamepadY = 0;
	static float gamepadMouseMoveX = 0;
	static float gamepadMouseMoveY = 0;
	boolean gamepadMouseThreadRunning = true;
	private JoystickAnalogListener listener;
	
	public JoystickAnalog(int maxX, int maxY, JoystickAnalogListener listener) {
		this.listener = listener;
		maxGamepadX = maxX;
		maxGamepadY = maxY;
		lastGamepadX = maxX / 2;
		lastGamepadY = maxY / 2;
	}

	public void stopGamepadMouseMoveThread() {
		gamepadMouseThreadRunning = false;
	}
	
	public void startGamepadMouseMoveThread() {
		if (gamepadMouseThreadRunning) return;
		
		Thread t = new Thread() {
			@Override
			public void run() {
				while (gamepadMouseThreadRunning) {
					gamepadX += gamepadMouseMoveX;
					gamepadY += gamepadMouseMoveY;
	
					if (gamepadX<0) gamepadX = 0;
					if (gamepadY<0) gamepadY = 0;
	
					if (gamepadX>maxGamepadX) gamepadX = maxGamepadX;
					if (gamepadY>maxGamepadY) gamepadY = maxGamepadY;
					
					if (lastGamepadX != gamepadX || lastGamepadY != gamepadY) {
						lastGamepadX = gamepadX;
						lastGamepadY = gamepadY;
						listener.onMouseMove(gamepadX, gamepadY);
					}
					listener.onMouseMoveRelative(gamepadMouseMoveX, gamepadMouseMoveY);
					try {
						Thread.sleep(40);
					} catch (InterruptedException e) {}
				}
			}
		};
		gamepadMouseThreadRunning = true;
		t.start();
	}
	
	public boolean onGenericMotionEvent (final MotionEvent event) {
		if (event.getAction() == MotionEvent.ACTION_MOVE && (event.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) == InputDevice.SOURCE_CLASS_JOYSTICK) {
			float moveX = event.getAxisValue(MotionEvent.AXIS_Z);
			float moveY = event.getAxisValue(MotionEvent.AXIS_RZ);
			if (Math.abs(moveX) < 0.1) {
				moveX = 0;
			}
			if (Math.abs(moveY) < 0.1) {
				moveY = 0;
			}

			gamepadMouseMoveX = moveX * 2;
			gamepadMouseMoveY = moveY * 2;
			
			float hatx = event.getAxisValue(MotionEvent.AXIS_HAT_X);
			float haty = event.getAxisValue(MotionEvent.AXIS_HAT_Y);
			
			float axisx = hatx!=0?hatx:event.getAxisValue(MotionEvent.AXIS_X);
			float axisy = haty!=0?haty:event.getAxisValue(MotionEvent.AXIS_Y);
			
			listener.onAxisChange(axisx, axisy);

			return true;

		}
		return false;
	}
	
	public void analogToDigital(float x, float y) {
		if (Math.abs(x)<deadzone) x = 0;
		if (Math.abs(y)<deadzone) y = 0;
		
		Axis newAxisX = x == 0? Axis.CENTER: (x<0?Axis.MIN:Axis.MAX);
		Axis newAxisY = y == 0? Axis.CENTER: (y<0?Axis.MIN:Axis.MAX);
		
		if (axisX!=newAxisX) {
			if (axisX != Axis.CENTER) listener.onDigitalX(axisX, false);
			axisX = newAxisX;
			if (axisX != Axis.CENTER) listener.onDigitalX(axisX, true);
		}
		if (axisY!=newAxisY) {
			if (axisY != Axis.CENTER) listener.onDigitalY(axisY, false);
			axisY = newAxisY;
			if (axisY != Axis.CENTER) listener.onDigitalY(axisY, true);
		}
	}
	
}

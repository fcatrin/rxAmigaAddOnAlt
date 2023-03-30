package org.ab.uae;

import java.nio.ByteBuffer;
import java.nio.ShortBuffer;

import retrobox.vinput.AnalogGamepad;
import retrobox.vinput.AnalogGamepad.Axis;
import retrobox.vinput.AnalogGamepadListener;
import retrobox.vinput.GamepadDevice;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.PaintFlagsDrawFilter;
import android.os.Build;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class MainSurfaceView  extends SurfaceView implements SurfaceHolder.Callback, Runnable {
	
	private static int MOUSE_LEFT = 1; // legacy mouse events
	private static int MOUSE_MOVE = 2; 
	private static float refMouseWidth = 640f;
	private static float refMouseHeight = 480f;
	
	DemoRenderer mRenderer;
	SurfaceHolder mSurfaceHolder;
	
	AnalogGamepad analogGamepad;
	private boolean invertRGB;
	
	private static final int heights[] = {200, 216, 240, 256, 262, 270, 200, 200};
	private static final int widths[] = {320, 352, 384};
	
    public MainSurfaceView(Context context, AttributeSet set) {
        super(context, set);
        mParent = (DemoActivity)context;
        
        Intent intent = mParent.getIntent();
        int presetModeId = intent.getIntExtra("presetModeId", 2);
        invertRGB = intent.getBooleanExtra("invertRGB", false);
        
        int surfaceWidth = widths[(presetModeId / 20) * 2];
        int surfaceHeight = heights[presetModeId % 10];
        boolean hires = ((presetModeId / 20) % 2) == 1;
       
        mRenderer = new DemoRenderer(mParent, surfaceWidth * (hires?2:1), surfaceHeight, invertRGB);
        
        mSurfaceHolder = getHolder();
        mSurfaceHolder.addCallback(this);
        
        analogGamepad = new AnalogGamepad((int)refMouseWidth, (int)refMouseWidth, new AnalogGamepadListener() {
			
			@Override
			public void onMouseMoveRelative(float mousex, float mousey) {}
			
			@Override
			public void onMouseMove(int mousex, int mousey) {
				float evx = (mousex / refMouseWidth) * mRenderer.bufferWidth;
				float evy = (mousex / refMouseHeight) * mRenderer.bufferHeight;
	    		int x = (int) (evx / scaleX);
	    		int y = (int) (evy / scaleY);
	    		if (x > mRenderer.bufferWidth)
	    			y = mRenderer.bufferWidth;
	    		if (y > mRenderer.bufferHeight)
	    			y = mRenderer.bufferHeight;
	    		nativeMouse(x , y, MOUSE_MOVE, 0, 0 );
			}
			
			@Override
			public void onAxisChange(GamepadDevice gamepad, float axisx, float axisy, float hatX, float hatY, float raxisx, float raxisy) {
				if (Math.abs(axisx) < 0.005) axisx = hatX;
				if (Math.abs(axisy) < 0.005) axisy = hatY;

				analogGamepad.analogToDigital(gamepad, axisx, axisy);
			}

			@Override
			public void onDigitalX(GamepadDevice gamepad, Axis axis, boolean on) {
				if (axis == Axis.MIN) DemoActivity.vinputDispatcher.sendKey(gamepad, KeyEvent.KEYCODE_DPAD_LEFT, on);
				if (axis == Axis.MAX) DemoActivity.vinputDispatcher.sendKey(gamepad, KeyEvent.KEYCODE_DPAD_RIGHT, on);
			}

			@Override
			public void onDigitalY(GamepadDevice gamepad,Axis axis, boolean on) {
				if (axis == Axis.MIN) DemoActivity.vinputDispatcher.sendKey(gamepad, KeyEvent.KEYCODE_DPAD_UP, on);
				if (axis == Axis.MAX) DemoActivity.vinputDispatcher.sendKey(gamepad, KeyEvent.KEYCODE_DPAD_DOWN, on);
			}

			@Override
			public void onTriggers(String deviceName, int deviceId, boolean left, boolean right) {
				DemoActivity.mapper.handleTriggerEventByDeviceName(deviceName, deviceId, left, right);
			}

			@Override
			public void onTriggersAnalog(GamepadDevice gamepad, int deviceId, float left, float right) {}
			
		});
        analogGamepad.startGamepadMouseMoveThread();
    }

    @Override
    public boolean onTouchEvent(final MotionEvent event) 
    {
    	
    	if (((DemoActivity) getContext()).vKeyPad != null && ((DemoActivity) getContext()).touch && DemoActivity.currentKeyboardLayout == 0) {
			boolean b = ((DemoActivity) getContext()).vKeyPad.onTouch(event, false);
			
			if (b)
				return true;
		}
    	
        // TODO: add multitouch support (added in Android 2.0 SDK)
        int action = -1;
        /*if( event.getAction() == MotionEvent.ACTION_DOWN )
        	action = 0;
        if( event.getAction() == MotionEvent.ACTION_UP )
        	action = 1;*/
        if( event.getAction() == MotionEvent.ACTION_MOVE )
        	action = 2;
        if (  action >= 0 ) {
    		int x = (int) (event.getX() / scaleX);
    		int y = (int) (event.getY() / scaleY);
    		if (x > mRenderer.bufferWidth)
    			y = mRenderer.bufferWidth;
    		if (y > mRenderer.bufferHeight)
    			y = mRenderer.bufferHeight;
    		nativeMouse(x , y, action, MOUSE_LEFT, 0 );
       	}
    
        return true;
    }
    
    @Override
	public boolean onGenericMotionEvent(MotionEvent event) {
		if (analogGamepad != null && analogGamepad.onGenericMotionEvent(event)) return true;
		return super.onGenericMotionEvent(event);
	}

    private static final float FACTOR = 15;
    
    @Override
	public boolean onTrackballEvent(MotionEvent event) {
    	int action = -1;
        if( event.getAction() == MotionEvent.ACTION_DOWN )
        	action = 0;
        if( event.getAction() == MotionEvent.ACTION_UP )
        	action = 1;
        if( event.getAction() == MotionEvent.ACTION_MOVE )
        	action = 2;
        //Log.i("UAE", "" + event.getX() + "/" + event.getY());
        if (  action >= 0 ) {
           nativeMouse( (int) (FACTOR*event.getX()), (int) (FACTOR*event.getY()), action, MOUSE_LEFT, 1);
           actionKey(null, action == 0, KeyEvent.KEYCODE_DPAD_CENTER);
        }
        return true;
	}

     public void exitApp() {
    	 if (mRenderer != null)
    		 mRenderer.nativeDone();
     };
     
     public void actionKey(KeyEvent event, boolean down, int keyCode) {
    	 if (down)
    		 keyDown(event, keyCode);
    	 else
    		 keyUp(event, keyCode);
     }
    
	public boolean keyDown(KeyEvent event, int keyCode) {
		if (DemoActivity.mapper.isSystemKey(event, keyCode)) return false;
		
		int joystick_nr = 1;
		
		if (keyCode >= 1000) {
			nativeKey( keyCode-1000, 1, 0, joystick_nr );
			//try { Thread.sleep(50); } catch (InterruptedException e) {}
			return true;
		}
		
		int h [] = mParent.getRealKeyCode(keyCode);
		keyCode = h[0];
		joystick_nr = h[1];
		
		if (keyCode == KeyEvent.KEYCODE_O || keyCode == (500+KeyEvent.KEYCODE_O)) {
			mParent.setRightMouse(0);
			mParent.mouse_button = 0;
			nativeMouse(0, 0, 0, MOUSE_LEFT, 1);
			return true;
		}
		
		if (keyCode == KeyEvent.KEYCODE_L || keyCode == (500+KeyEvent.KEYCODE_L)) {
			mParent.setRightMouse(1);
			mParent.mouse_button = 1;
			nativeMouse(0, 0, 0, MOUSE_LEFT, 1);
			//mParent.setRightMouse(0);
			return true;
		}
		
		
		nativeKey( keyCode, 1, mParent.joystick, joystick_nr );
         return true;
     }
	
	public boolean keyUp(KeyEvent event, int keyCode) {
		if (DemoActivity.mapper.isSystemKey(event, keyCode)) return false;
		
		int joystick_nr = 1;
		
		if (keyCode >= 2000) {
			nativeKey( keyCode-2000, 0, 1, joystick_nr );
			//try { Thread.sleep(50); } catch (InterruptedException e) {}
			return true;
		} else if (keyCode >= 1000) {
			nativeKey( keyCode-1000, 0, 0, joystick_nr );
			//try { Thread.sleep(50); } catch (InterruptedException e) {}
			return true;
		}
		
		int h [] = mParent.getRealKeyCode(keyCode);
		keyCode = h[0];
		joystick_nr = h[1];
		
		if (keyCode == KeyEvent.KEYCODE_O || keyCode == (500+KeyEvent.KEYCODE_O)) {
			mParent.setRightMouse(0);
			mParent.mouse_button = 0;
			try { Thread.sleep(100); } catch (InterruptedException e) {} // if down + up is too fast, it's not recognized in the emu
			nativeMouse(0, 0, 1, MOUSE_LEFT, 1);
			return true;
		}
		
		if (keyCode == KeyEvent.KEYCODE_L || keyCode == (500+KeyEvent.KEYCODE_L)) {
			mParent.setRightMouse(1);
			mParent.mouse_button = 1;
			try { Thread.sleep(100); } catch (InterruptedException e) {} // if down + up is too fast, it's not recognized in the emu
			nativeMouse(0, 0, 1, MOUSE_LEFT, 1);
			//mParent.setRightMouse(0);
			return true;
		}
		
		
		nativeKey( keyCode, 0, mParent.joystick, joystick_nr );
         return true;
     }

    DemoActivity mParent;

    public static native void nativeMouse( int x, int y, int action, int button, int relative );
    public static native void nativeKey( int keyCode, int down, int joystick, int joystick_nr );
    public static native void setNumJoysticks(int numJoysticks);
  
	public void shiftImage(int leftDPIs) {
		if (leftDPIs > 0) {
			DisplayMetrics metrics = new DisplayMetrics();
			mParent.getWindowManager().getDefaultDisplay().getMetrics(metrics);
	        pixels = (int) (leftDPIs * metrics.density + 0.5f);
	        Log.i("uae", "pixels: " + pixels);
	        scaleX = (float) (width-pixels)/mRenderer.bufferWidth;
			scaleY = (float) height/mRenderer.bufferHeight;
			coordsChanged = true;
			
		} else {
			pixels = 0;
			scaleX = (float) width/mRenderer.bufferWidth;
			scaleY = (float) height/mRenderer.bufferHeight;
			coordsChanged = true;
			
		}
		Log.i("UAE", "new scale: " + scaleX + "-" + scaleY + "-" + pixels);
		if (width < height) {
			scaleY = scaleX;
		}
		
		initMatrix();
    	
	}
	
	public void run() {
		Log.i("Renderer", "nativeInit");
		if (mRenderer != null)
   		 mRenderer.nativeInit(mParent, buffer, 1, invertRGB);
	}

	public void surfaceChanged(SurfaceHolder holder, int format, int w,
			int h) {
		if (mRenderer != null)
   		 mRenderer.nativeResize(w, h);
       
        coordsChanged = true;
        
		this.width = w;
		this.height = h;
		scaleX = (float) (width-pixels)/mRenderer.bufferWidth;
		scaleY = (float) height/mRenderer.bufferHeight;
		if (width < height) {
			scaleY = scaleX;
		}
		
		initMatrix();
		
		Log.i("UAE", "new onSurfaceChanged: " + scaleX + "-" + scaleY + "-" + pixels);
		
		if (((DemoActivity) getContext()).vKeyPad != null)
			((DemoActivity) getContext()).vKeyPad.resize(width, height);
		
	}
	
	public void initMatrix() {
		/* matrixScreen = new Matrix();
		 matrixScreen.setScale(scaleX, scaleY);
		 matrixScreen.postTranslate(pixels, 0);
    	 */
		
		if (width < height) {
			scaleY = scaleX;
		} else {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getContext());
		String scale = prefs.getString("scale", "stretched");
		 pixels = 0;
		 pixelsH = 0;
		 if ("scaled".equals(scale)) {
			 scaleX = scaleY;
			 pixels = (int) ((width - mRenderer.bufferWidth*scaleX) / 2);
		 } else if ("1x".equals(scale)) {
			 scaleX = 1.0f;
			 scaleY = 1.0f;
			 pixels = (int) ((width - mRenderer.bufferWidth*scaleX) / 2);
			 pixelsH = (int) ((height - mRenderer.bufferHeight*scaleX) / 2);
		 } else if ("2x".equals(scale)) {
			 scaleX = 2.0f;
			 scaleY = 2.0f;
			 pixels = (int) ((width - mRenderer.bufferWidth*scaleX) / 2);
			 pixelsH = (int) ((height - mRenderer.bufferHeight*scaleX) / 2);
		 }
		}
		 matrixScreen = new Matrix();
		 matrixScreen.setScale(scaleX, scaleY);
		 matrixScreen.postTranslate(pixels, pixelsH);
	}
	
	protected float scaleX;
	protected float scaleY;
	protected boolean coordsChanged;
	protected int width;
	protected int height;
	protected int pixels;
	protected int pixelsH;
	 Matrix matrixScreen;
	 
	ShortBuffer buffer;
	
	 PaintFlagsDrawFilter setfil = new PaintFlagsDrawFilter(0, 
			 Paint.FILTER_BITMAP_FLAG); 
	 
	 
	public void surfaceCreated(SurfaceHolder holder) {
		if (buffer == null) {
			Log.i("UAE", "surfaceCreated");
	    	ByteBuffer bb = ByteBuffer.allocateDirect(mRenderer.bufferWidth*mRenderer.bufferHeight*2);
	    	buffer = bb.asShortBuffer();
	    	 mainScreen = Bitmap.createBitmap(mRenderer.bufferWidth, mRenderer.bufferHeight, Bitmap.Config.RGB_565);
	    	 
	    	 //updater =new ScreenUpdater(this);
	    	 //updater.start();
	    	 
	    	if (DemoActivity.nativeThread == null || !DemoActivity.nativeThread.isAlive()) {
	    		DemoActivity.nativeThread = new Thread(this);
	    		DemoActivity.nativeThread.start();
	    	}
    	}
	}
	
	public Bitmap mainScreen;
	
	long t = System.currentTimeMillis();
	int frames;
	public void checkFPS() {

        frames++;
        
        if (frames % 20 == 0) {
        	long t2 = System.currentTimeMillis();
        	Log.i("uae", "FPS: " + 20000 / (t2 - t));
        	t = t2;
        }
        
	}
	@SuppressLint("NewApi")
	public void requestRender() {
		
        //checkFPS();
		Canvas c = null;
        try {
        	if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
        		c = mSurfaceHolder.lockHardwareCanvas();
        	} else {
        		c = mSurfaceHolder.lockCanvas(null);
        	}
            synchronized (mSurfaceHolder) {
            	buffer.position(0);
            	 mainScreen.copyPixelsFromBuffer(buffer);
            	 if (c != null && matrixScreen != null) {
            		 // disabled. It is too slow on some devices
 	            	// if (DemoActivity.aliased) c.setDrawFilter(setfil);
 	            	c.drawBitmap(mainScreen, matrixScreen, null);
 	            	if (mParent.vKeyPad != null && mParent.touch && DemoActivity.currentKeyboardLayout == 0)
 	            		mParent.vKeyPad.draw(c);
            	 
            	}
            	 
            }
        } finally {
            // do this in a finally so that if an exception is thrown
            // during the above, we don't leave the Surface in an
            // inconsistent state
            if (c != null) {
                mSurfaceHolder.unlockCanvasAndPost(c);
            }
        }
		
	}

	public void surfaceDestroyed(SurfaceHolder holder) {
        analogGamepad.stopGamepadMouseMoveThread();

	}
	
	public void onPause() {
		if (mRenderer != null)
   		 mRenderer.nativePause();
		analogGamepad.stopGamepadMouseMoveThread();
	}

	public void onResume() {
		if (mRenderer != null)
   		 mRenderer.nativeResume();
        analogGamepad.startGamepadMouseMoveThread();
	}



	/*protected boolean readyToRenderAgain = true;
    protected Runnable renderRunnable = new Runnable() {
        public void run() {
            readyToRenderAgain = true;
        }
    };

    public void requestRender() {
        if (readyToRenderAgain) {
            super.requestRender();
            queueEvent(renderRunnable);
        }
        readyToRenderAgain = false;
    }*/

}


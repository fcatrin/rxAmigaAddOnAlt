#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

#include <SDL.h>

#include <png.h>

#include <retrobox.h>
#include "screenshot.h"

#include <android/log.h>

char  screenshot_dir[1024] = "";
char  screenshot_name[256] = "";

#define  systemRedShift      (prSDLScreen->format->Rshift)
#define  systemGreenShift    (prSDLScreen->format->Gshift)
#define  systemBlueShift     (prSDLScreen->format->Bshift)
#define  systemRedMask       (prSDLScreen->format->Rmask)
#define  systemGreenMask     (prSDLScreen->format->Gmask)
#define  systemBlueMask      (prSDLScreen->format->Bmask)

extern SDL_Surface *prSDLScreen;

int save_png(SDL_Surface* surface,char *path)
{
  int w = surface->w;
  int h = surface->h;
  unsigned char * pix = (unsigned char *)surface->pixels;
  unsigned char writeBuffer[512 * 3];
  FILE *f  = fopen(path,"wb");
  if(!f) return 0;
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                NULL,
                                                NULL,
                                                NULL);
  if(!png_ptr) {
    fclose(f);
    return 0;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if(!info_ptr) {
    png_destroy_write_struct(&png_ptr,NULL);
    fclose(f);
    return 0;
  }

  png_init_io(png_ptr,f);

  png_set_IHDR(png_ptr,
               info_ptr,
               w,
               h,
               8,
               PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr,info_ptr);

  unsigned char *b = writeBuffer;

  int sizeX = w;
  int sizeY = h;
  int y;
  int x;

unsigned short *p = (unsigned short *)pix;
for(y = 0; y < sizeY; y++)
{
   for(x = 0; x < sizeX; x++)
   {
     unsigned short v = p[x];

     *b++ = ((v & systemRedMask  ) >> systemRedShift  ) << 3; // R
     *b++ = ((v & systemGreenMask) >> systemGreenShift) << 2; // G
     *b++ = ((v & systemBlueMask ) >> systemBlueShift ) << 3; // B
   }
   p += surface->pitch / 2;
   png_write_row(png_ptr,writeBuffer);
   b = writeBuffer;
}

  png_write_end(png_ptr, info_ptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);

  fclose(f);
  return 1;
}


char screenshot_path[2048] = "";

static char *build_screenshot_path() {
	for(int i=0; i<1000; i++) {
		sprintf(screenshot_path, "%s/%s.shot.%i.png", screenshot_dir, screenshot_name, i);
		if (access(screenshot_path, F_OK) == -1) {
			return screenshot_path;
		}
	}
	return NULL;
}

int save_thumb(int code,char *path)
{
	 SDL_Surface *thumb_screen;
	 int scaling,xstart,ystart,xend,yend,w,h;

	 if (path == NULL) {
		 path = build_screenshot_path();
	 }

	 if (path == NULL) {
		 __android_log_print(ANDROID_LOG_ERROR, "UAE4ALL2","reached maximum screenshots at %s", screenshot_dir);
		 return 0;
	 }

if (code==SCREENSHOT){
		scaling=1;
		xstart=0;
		ystart=0;
		yend=prSDLScreen->h;
		xend=prSDLScreen->w;
		w=prSDLScreen->w;
		h=prSDLScreen->h;
	}
	else {
		scaling=7;
		xstart=48;
		ystart=8;
		xend=320-48;
		yend=240-8;
		w=32;
		h=32;
	}
	thumb_screen=SDL_CreateRGBSurface(prSDLScreen->flags,w,h,prSDLScreen->format->BitsPerPixel,prSDLScreen->format->Rmask,prSDLScreen->format->Gmask,prSDLScreen->format->Bmask,prSDLScreen->format->Amask);

int x;
int y=ystart;
 unsigned short * src_pixel = (unsigned short*)prSDLScreen->pixels;
 unsigned short * dst_pixel = (unsigned short*)thumb_screen->pixels;
 unsigned short * scan_src_pixel = 0;
 for (y=ystart; y < yend; y+=scaling) {
	scan_src_pixel = src_pixel + (prSDLScreen->w * y);
    for (x = xstart; x < xend; x+=scaling)*dst_pixel++ = scan_src_pixel[x];
 }
 	 __android_log_print(ANDROID_LOG_INFO, "UAE4ALL2","save screenshot png to %s", path);
	int ret=save_png(thumb_screen,path);
 	printf("SAVED \n");
	SDL_FreeSurface(thumb_screen);
	return ret;
}

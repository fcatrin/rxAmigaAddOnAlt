int kickstart=1;
int oldkickstart=-1;	/* reload KS at startup */
char kickstarts_dir[1024];

extern char launchDir[300];

extern "C" int main( int argc, char *argv[] );

/*
  * UAE - The Un*x Amiga Emulator
  *
  * Main program
  *
  * Copyright 1995 Ed Hanway
  * Copyright 1995, 1996, 1997 Bernd Schmidt
  */
#include "sysconfig.h"
#include "sysdeps.h"
#include <assert.h>
#include "vkbd.h"
#include "config.h"
#include "uae.h"
#include "options.h"
#include "thread.h"
#include "debug_uae4all.h"
#include "gensound.h"
#include "events.h"
#include "memory-uae.h"
#include "audio.h"
#include "memory-uae.h"
#include "sound.h"
#include "custom.h"
#include "m68k/m68k_intrf.h"
#include "disk.h"
#include "debug.h"
#include "xwin.h"
#include "joystick.h"
#include "keybuf.h"
#include "gui.h"
#include "zfile.h"
#include "autoconf.h"
#include "osemu.h"
#include "exectasks.h"
#include "compiler.h"
#include "bsdsocket.h"
#include "drawing.h"
#include "menu.h" 
#include "gp2xutil.h"
#include "savestate.h"
#include "menu_config.h"
#include "retrobox.h"

#ifdef __WINS__
#include "target.h"
#endif
/* PocketUAE */
#include "native2amiga.h"

#ifdef USE_SDL
#include "SDL.h"
#endif
#ifdef DREAMCAST
#include<SDL_dreamcast.h>
#endif
#ifdef GP2X
#include "gp2xutil.h"
#endif
long int version = 256*65536L*UAEMAJOR + 65536L*UAEMINOR + UAESUBREV;

struct uae_prefs currprefs, changed_prefs; 

int no_gui = 0;
int joystickpresent = 0;
int cloanto_rom = 0;

extern int gfxHeight;
extern int hwScaled;

struct gui_info gui_data;

char warning_buffer[256];

/* If you want to pipe printer output to a file, put something like
 * "cat >>printerfile.tmp" above.
 * The printer support was only tested with the driver "PostScript" on
 * Amiga side, using apsfilter for linux to print ps-data.
 *
 * Under DOS it ought to be -p LPT1: or -p PRN: but you'll need a
 * PostScript printer or ghostscript -=SR=-
 */

/* Slightly stupid place for this... */
/* ncurses.c might use quite a few of those. */
const char *colormodes[] = { "256 colors", "32768 colors", "65536 colors",
    "256 colors dithered", "16 colors dithered", "16 million colors",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
};

void discard_prefs ()
{
}

void default_prefs ()
{
    produce_sound = 2;
    prefs_gfx_framerate = 2;

    

	/* 1MB */
    prefs_chipmem_size = 0x00100000;
    prefs_bogomem_size = 0;
	changed_prefs.fastmem_size = 0;
}

int quit_program = 0;
int pause_program = 0;
int right_mouse;

int mainMenu_drives = DEFAULT_DRIVES;
int mainMenu_floppyspeed = 100;
int mainMenu_CPU_model = DEFAULT_CPU_MODEL;
int mainMenu_chipset = DEFAULT_CHIPSET_SELECT;
int mainMenu_sound = DEFAULT_SOUND;
int mainMenu_soundStereo = 1;
int mainMenu_CPU_speed = 0;

int mainMenu_cpuSpeed = 600;

int mainMenu_joyConf = 0;
int mainMenu_autofireRate = 8;
int mainMenu_showStatus = DEFAULT_STATUSLN;
int mainMenu_showFPS = false;
int mainMenu_mouseMultiplier = 1;
int mainMenu_stylusOffset = 0;
int mainMenu_tapDelay = 10;
int mainMenu_customControls = 0;
int mainMenu_custom_dpad = 0;
int mainMenu_custom_up = 0;
int mainMenu_custom_down = 0;
int mainMenu_custom_left = 0;
int mainMenu_custom_right = 0;
int mainMenu_custom_A = 0;
int mainMenu_custom_B = 0;
int mainMenu_custom_X = 0;
int mainMenu_custom_Y = 0;
int mainMenu_custom_L = 0;
int mainMenu_custom_R = 0;

int mainMenu_displayedLines = 240;
int mainMenu_displayHires = 0;
char presetMode[20] = "320x240 upscaled";
int presetModeId = 2;
int mainMenu_cutLeft = 0;
int mainMenu_cutRight = 0;
int mainMenu_ntsc = DEFAULT_NTSC;
int mainMenu_frameskip = 0;
int mainMenu_autofire = DEFAULT_AUTOFIRE;

// The following params in use, but can't be changed with gui
int mainMenu_throttle = 0;
int mainMenu_autosave = DEFAULT_AUTOSAVE;
int mainMenu_button1 = 0;
int mainMenu_button2 = 0;
int mainMenu_autofireButton1 = 0;
int mainMenu_jump = -1;

// The following params not in use, but stored to write them back to the config file
int mainMenu_enableHWscaling = DEFAULT_SCALING;
int gp2xClockSpeed = -1;
int mainMenu_scanlines = 0;
int mainMenu_ham = 1;
int mainMenu_enableScreenshots = DEFAULT_ENABLESCREENSHOTS;
int mainMenu_enableScripts = DEFAULT_ENABLESCRIPTS;

void SetPresetMode(int mode) {}
void update_display() {}
void menu_raise(void) {}
void menu_unraise(void) {}
void init_text(int splash) {}
void quit_text(void) {}
void inputmode_init(void) {}
void inputmode_redraw(void) {}
int run_mainMenu() {}

int saveMenu_n_savestate=0;
int gp2xButtonRemappingOn=0;
int hasGp2xButtonRemapping=0;
int gp2xMouseEmuOn=0;
int switch_autofire=0;
const char *statusmessages[] = { "AUTOFIRE ON\0", "AUTOFIRE OFF\0","SCREENSHOT SAVED\0","SCRIPT SAVED\0","SCRIPT AND SCREENSHOT SAVED\0"};
int showmsg=0;

int mainMenu_chipMemory = DEFAULT_CHIPMEM_SELECT;
int mainMenu_slowMemory = 0;    /* off */
int mainMenu_fastMemory = 0;    /* off */

int mainMenu_bootHD = DEFAULT_ENABLE_HD;
int mainMenu_filesysUnits = 0;
int hd_dir_unit_nr = -1;
int hd_file_unit_nr = -1;

void UpdateCPUModelSettings(struct uae_prefs *p)
{
    switch (mainMenu_CPU_model)
    {
        case 1: p->cpu_level = M68020; break;
        default: p->cpu_level = M68000; break;
    }
}


void UpdateMemorySettings(struct uae_prefs *p)
{
    prefs_chipmem_size = 0x000080000 << mainMenu_chipMemory;

    /* >2MB chip memory => 0 fast memory */
    if ((mainMenu_chipMemory > 2) && (mainMenu_fastMemory > 0))
    {
        mainMenu_fastMemory = 0;
        p->fastmem_size = 0;
    }

    switch (mainMenu_slowMemory) 
    {
        case 1: case 2:
            prefs_bogomem_size = 0x00080000 << (mainMenu_slowMemory - 1);
            break;
        case 3:
            prefs_bogomem_size = 0x00180000;    /* 1.5M */
            break;
        default:
            prefs_bogomem_size = 0;
    }

    switch (mainMenu_fastMemory) 
    {
        case 0:
            p->fastmem_size = 0;
            break;
        default:
            p->fastmem_size = 0x00080000 << mainMenu_fastMemory;
    }

}


void UpdateChipsetSettings(struct uae_prefs *p)
{
    switch (mainMenu_chipset) 
    {
        case 1: p->chipset_mask = CSMASK_ECS_AGNUS | CSMASK_ECS_DENISE; break;
        case 2: p->chipset_mask = CSMASK_ECS_AGNUS | CSMASK_ECS_DENISE | CSMASK_AGA; break;
        default: p->chipset_mask = CSMASK_ECS_AGNUS; break;
    }
}


#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)

extern "C" void
JAVA_EXPORT_NAME(DemoRenderer_nativePause) ( JNIEnv*  env, jobject  thiz) {
    pause_program = 1;
}

extern "C" void
JAVA_EXPORT_NAME(DemoRenderer_nativeResume) ( JNIEnv*  env, jobject  thiz) {
    pause_program = 0;
}

extern "C" void
JAVA_EXPORT_NAME(DemoActivity_nativeReset) ( JNIEnv*  env, jobject  thiz) {
    uae_reset();
}

extern "C" void
JAVA_EXPORT_NAME(DemoActivity_nativeQuit) ( JNIEnv*  env, jobject  thiz) {
    uae_quit();
    exit(0);
}

extern "C" void
JAVA_EXPORT_NAME(DemoActivity_setRightMouse) ( JNIEnv*  env, jobject  thiz, jint right) {
    right_mouse = right;
}

extern "C" void
JAVA_EXPORT_NAME(DemoActivity_setPrefs) ( JNIEnv*  env, jobject  thiz, jstring config) {

    default_prefs_uae (&currprefs);
    default_prefs();

	prefs_gfx_framerate = 1;

	char configFileName[1024] = "default.config";

    if (config)
    {
        const char *sconfig = (env)->GetStringUTFChars(config, 0);
        strcpy(configFileName, sconfig);
        (env)->ReleaseStringUTFChars(config, sconfig);
    }

    FILE *f=fopen(configFileName,"rt");
    if (!f){
    	__android_log_print(ANDROID_LOG_INFO, "UAE4DROID", "No config file %s!",configFileName);
    } else {
    	__android_log_print(ANDROID_LOG_INFO, "UAE4DROID", "Config file %s",configFileName);
		char line[1024];
		while (fgets(line, sizeof(line), f)) {
			sscanf(line, "df0=%s\n", prefs_df[0]);
			sscanf(line, "df1=%s\n", prefs_df[1]);
			sscanf(line, "kickstart=%d\n",&kickstart);
			sscanf(line, "kickstarts_dir=%s\n",kickstarts_dir);
			sscanf(line, "presetModeId=%d\n",&presetModeId); // resolution to render on
			sscanf(line, "showstatus=%d\n",&mainMenu_showStatus); // 1 = show leds
			sscanf(line, "showfps=%d\n",&mainMenu_showFPS); // 1 = show leds
			sscanf(line, "floppyspeed=%d\n",&mainMenu_floppyspeed); // floppy speed in percent (100 = 100% Amiga)
			sscanf(line, "drives=%d\n",&mainMenu_drives); // restrict number of drives
			sscanf(line, "frameskip=%d\n",&prefs_gfx_framerate); // restrict number of drives
		}

		__android_log_print(ANDROID_LOG_INFO, "UAE4DROID", "df0 %s", prefs_df[0]);
		__android_log_print(ANDROID_LOG_INFO, "UAE4DROID", "df1 %s", prefs_df[1]);
		__android_log_print(ANDROID_LOG_INFO, "UAE4DROID", "kickstart %i", kickstart);
		__android_log_print(ANDROID_LOG_INFO, "UAE4DROID", "kickstarts_dir %s", kickstarts_dir);
		snprintf(romfile, 256, "%s/%s",kickstarts_dir,kickstarts_rom_names[kickstart]);
		__android_log_print(ANDROID_LOG_INFO, "UAE4DROID", "rom %s", romfile);
		__android_log_print(ANDROID_LOG_INFO, "UAE4DROID", "presetModeId %d", presetModeId);

    }

	produce_sound = 2;
	changed_produce_sound = produce_sound;

	changed_gfx_framerate = prefs_gfx_framerate;

	mainMenu_CPU_model = DEFAULT_CPU_MODEL;
	mainMenu_chipMemory = 1;
    mainMenu_slowMemory = 0;
    mainMenu_fastMemory = 0;
    mainMenu_chipset = DEFAULT_CHIPSET_SELECT; // aga
    mainMenu_CPU_speed = 0; // 500/5T/a1200/12T/12T2

    SetPresetMode(presetModeId);

    UpdateCPUModelSettings(&changed_prefs);
    UpdateMemorySettings(&changed_prefs);
    UpdateChipsetSettings(&changed_prefs);
    m68k_speed = 0;
    check_prefs_changed_cpu();
    check_prefs_changed_audio();
}

extern "C" void
JAVA_EXPORT_NAME(DemoActivity_setScreenshotDir) ( JNIEnv*  env, jobject  thiz,  jstring jdir) {
	const char *dir = (env)->GetStringUTFChars(jdir , NULL ) ;
	strcpy(screenshot_dir, dir);
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "set screenshot dir to %s", dir );
	(env)->ReleaseStringUTFChars(jdir, dir);
}

extern "C" void
JAVA_EXPORT_NAME(DemoActivity_setScreenshotName) ( JNIEnv*  env, jobject  thiz,  jstring jname) {
	const char *name = (env)->GetStringUTFChars(jname , NULL ) ;
	strcpy(screenshot_name, name);
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "set screenshot name to %s", name );
	(env)->ReleaseStringUTFChars(jname, name);
}

static void prepare_savestate_name(const char *srom, int slot) {
	strcpy(savestate_filename, srom);

	if (slot == 0) {
		strcat(savestate_filename, ".asf");
	} else {
		char buffer[256] = "";
		sprintf(buffer, "-%d.asf", slot);
		strcat(savestate_filename, buffer);
	}
}

extern "C" void
JAVA_EXPORT_NAME(DemoActivity_saveState) ( JNIEnv*  env, jobject  thiz,  jstring filename, jint slot) {

    const char *srom = (env)->GetStringUTFChars(filename, 0);
    
    prepare_savestate_name(srom, slot);

    (env)->ReleaseStringUTFChars(filename, srom);
        
    savestate_state = STATE_DOSAVE;
    __android_log_print(ANDROID_LOG_INFO, "UAE", "Saved %s", savestate_filename);

}


char *diskimages[] = {prefs_df[0], prefs_df[1], prefs_df[2], prefs_df[3]};

char *basename(char *filename) {
	int i = 0;
	int pos = 0;
	while (filename[i]) {
		if (filename[i] == '/') pos = i+1;
		i++;
	}
	return &filename[pos];
}

void disk_insert(int fd, char *image) {
	strcpy(diskimages[fd], image);
	strcpy(changed_df[fd], image);
	real_changed_df[fd]=1;
}

void disk_swap() {
	char tmp[300];
	strcpy(tmp, diskimages[0]);

	int i=1;
	while(i<4 && diskimages[i]!=NULL && strlen(diskimages[i])>0) {
		disk_insert(i-1, diskimages[i]);
		i++;
	}
	disk_insert(i-1, tmp);
}

extern "C" jstring
JAVA_EXPORT_NAME(DemoActivity_diskSwap) ( JNIEnv*  env, jobject  thiz) {
	disk_swap();

	char usermsg[300];
	sprintf(usermsg, "%s", basename(changed_df[0]));

	return env->NewStringUTF(usermsg);
}


extern "C" void
JAVA_EXPORT_NAME(DemoActivity_loadState) ( JNIEnv*  env, jobject  thiz,  jstring filename, jint slot) {

    // shagrath : don't ask
    int hackEnableSound = 0;
    if (!produce_sound)
    {
        changed_produce_sound = 1;
        check_prefs_changed_audio();
        hackEnableSound = 1;
    }

    //

    const char *srom = (env)->GetStringUTFChars(filename, 0);

    prepare_savestate_name(srom, slot);

    (env)->ReleaseStringUTFChars(filename, srom);
        
    savestate_state = STATE_DORESTORE;
    __android_log_print(ANDROID_LOG_INFO, "UAE", "Loaded %s", savestate_filename);

    if (hackEnableSound)
    {
        changed_produce_sound = 0;
        check_prefs_changed_audio();
    }

}

void uae_reset (void)
{
    gui_purge_events();
    black_screen_now();
    quit_program = 2;
    set_special (SPCFLAG_BRK);
}

void uae_quit (void)
{
    if (quit_program != -1)
	quit_program = -1;
}

void reset_all_systems (void)
{
    init_eventtab ();
    memory_reset ();
    filesys_reset ();
    filesys_start_threads ();
}

/* Okay, this stuff looks strange, but it is here to encourage people who
 * port UAE to re-use as much of this code as possible. Functions that you
 * should be using are do_start_program() and do_leave_program(), as well
 * as real_main(). Some OSes don't call main() (which is braindamaged IMHO,
 * but unfortunately very common), so you need to call real_main() from
 * whatever entry point you have. You may want to write your own versions
 * of start_program() and leave_program() if you need to do anything special.
 * Add #ifdefs around these as appropriate.
 */
void do_start_program (void)
{
    __android_log_print(ANDROID_LOG_INFO, "UAE", "do_start_program");
	quit_program = 2;
	reset_frameskip();
	m68k_go (1);
}

void do_leave_program (void)
{
    graphics_leave ();
    close_joystick ();
    close_sound ();
    dump_counts ();
    zfile_exit ();
#ifdef USE_SDL
    SDL_Quit ();
#endif
    memory_cleanup ();
}

void start_program (void)
{
    do_start_program ();
}

void leave_program (void)
{
    do_leave_program ();
}

void real_main (int argc, char **argv)
{
#ifdef USE_SDL
    SDL_Init (SDL_INIT_VIDEO | SDL_INIT_JOYSTICK 
#if !defined(NO_SOUND) && !defined(GP2X)
 			| SDL_INIT_AUDIO
#endif
	);
#endif
	
    /* PocketUAE prefs */
   
#ifdef GP2X
    gp2x_init(argc, argv);
#endif
    //loadconfig (1);

    if (! graphics_setup ()) {
		exit (1);
    }

    rtarea_init ();

	hardfile_install();

    if (! setup_sound ()) {
		write_log ("Sound driver unavailable: Sound output disabled\n");
		produce_sound = 0;
    }
    init_joystick ();

    int err = gui_init ();
	if (err == -1) {
	    write_log ("Failed to initialize the GUI\n");
	} else if (err == -2) {
	    exit (0);
	}
    if (sound_available && produce_sound > 1 && ! init_audio ()) {
		write_log ("Sound driver unavailable: Sound output disabled\n");
		produce_sound = 0;
    }

    /* Install resident module to get 8MB chipmem, if requested */
    rtarea_setup ();

    keybuf_init (); /* Must come after init_joystick */

#ifdef USE_AUTOCONFIG
    expansion_init ();
#endif

    memory_init ();

    filesys_install (); 
    native2amiga_install ();

    custom_init (); /* Must come after memory_init */
    DISK_init ();

    init_m68k();
#ifndef USE_FAME_CORE
    compiler_init ();
#endif
    //gui_update ();

#ifdef GP2X
    switch_to_hw_sdl(1);
#endif
    if (graphics_init())
	{
		start_program ();
	}
    leave_program ();
}

#ifndef NO_MAIN_IN_MAIN_C
int main (int argc, char *argv[])
{
	gfxHeight = 240;
	hwScaled = 1;

    real_main (argc, argv);
    return 0;
}

void lanada(void)
{
}

void default_prefs_uae (struct uae_prefs *p)
{
    p->chipset_mask = CSMASK_ECS_AGNUS;
    
    p->cpu_level = M68000;
    
    p->fastmem_size = 0x00000000;
    p->z3fastmem_size = 0x00000000;
    p->gfxmem_size = 0x00000000;

    p->mountinfo = alloc_mountinfo ();
}

void discard_prefs_uae (struct uae_prefs *p)
{
    free_mountinfo (p->mountinfo);
}
#endif

void SetPresetModeDisabled(int mode)
{
	int screenWidth = 0; // ignore property until resolved

	presetModeId = mode;

	__android_log_print(ANDROID_LOG_INFO, "SetPresetMode", "mode %d", mode);

	switch(mode)
	{
		case 0:
			mainMenu_displayedLines = 200;
			screenWidth = 768;
			strcpy(presetMode, "320x200 upscaled");
			break;

		case 1:
			mainMenu_displayedLines = 216;
			screenWidth = 716;
			strcpy(presetMode, "320x216 upscaled");
			break;

		case 2:
			mainMenu_displayedLines = 240;
			screenWidth = 640;
			strcpy(presetMode, "320x240 upscaled");
			break;

		case 3:
			mainMenu_displayedLines = 256;
			screenWidth = 600;
			strcpy(presetMode, "320x256 upscaled");
			break;

		case 4:
			mainMenu_displayedLines = 262;
			screenWidth = 588;
			strcpy(presetMode, "320x262 upscaled");
			break;

		case 5:
			mainMenu_displayedLines = 270;
			screenWidth = 570;
			strcpy(presetMode, "320x270 upscaled");
			break;

		case 6:
			mainMenu_displayedLines = 200;
			screenWidth = 640;
			strcpy(presetMode, "320x200 NTSC");
			break;

		case 7:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "320x200 fullscreen");
			break;

		case 10:
			mainMenu_displayedLines = 200;
			screenWidth = 768;
			strcpy(presetMode, "640x200 upscaled");
			break;

		case 11:
			mainMenu_displayedLines = 216;
			screenWidth = 716;
			strcpy(presetMode, "640x216 upscaled");
			break;

		case 12:
			mainMenu_displayedLines = 240;
			screenWidth = 640;
			strcpy(presetMode, "640x240 upscaled");
			break;

		case 13:
			mainMenu_displayedLines = 256;
			screenWidth = 600;
			strcpy(presetMode, "640x256 upscaled");
			break;

		case 14:
			mainMenu_displayedLines = 262;
			screenWidth = 588;
			strcpy(presetMode, "640x262 upscaled");
			break;

		case 15:
			mainMenu_displayedLines = 270;
			screenWidth = 570;
			strcpy(presetMode, "640x270 upscaled");
			break;

		case 16:
			mainMenu_displayedLines = 200;
			screenWidth = 640;
			strcpy(presetMode, "640x200 NTSC");
			break;

		case 17:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "640x200 fullscreen");
			break;

		case 20:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "352x200 upscaled");
			break;

		case 21:
			mainMenu_displayedLines = 216;
			screenWidth = 784;
			strcpy(presetMode, "352x216 upscaled");
			break;

		case 22:
			mainMenu_displayedLines = 240;
			screenWidth = 704;
			strcpy(presetMode, "352x240 upscaled");
			break;

		case 23:
			mainMenu_displayedLines = 256;
			screenWidth = 660;
			strcpy(presetMode, "352x256 upscaled");
			break;

		case 24:
			mainMenu_displayedLines = 262;
			screenWidth = 640;
			strcpy(presetMode, "352x262 upscaled");
			break;

		case 25:
			mainMenu_displayedLines = 270;
			screenWidth = 624;
			strcpy(presetMode, "352x270 upscaled");
			break;

		case 26:
			mainMenu_displayedLines = 200;
			screenWidth = 704;
			strcpy(presetMode, "352x200 NTSC");
			break;

		case 27:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "352x200 fullscreen");
			break;

		case 30:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "704x200 upscaled");
			break;

		case 31:
			mainMenu_displayedLines = 216;
			screenWidth = 784;
			strcpy(presetMode, "704x216 upscaled");
			break;

		case 32:
			mainMenu_displayedLines = 240;
			screenWidth = 704;
			strcpy(presetMode, "704x240 upscaled");
			break;

		case 33:
			mainMenu_displayedLines = 256;
			screenWidth = 660;
			strcpy(presetMode, "704x256 upscaled");
			break;

		case 34:
			mainMenu_displayedLines = 262;
			screenWidth = 640;
			strcpy(presetMode, "704x262 upscaled");
			break;

		case 35:
			mainMenu_displayedLines = 270;
			screenWidth = 624;
			strcpy(presetMode, "704x270 upscaled");
			break;

		case 36:
			mainMenu_displayedLines = 200;
			screenWidth = 704;
			strcpy(presetMode, "704x200 NTSC");
			break;

		case 37:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "704x200 fullscreen");
			break;

		case 40:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "384x200 upscaled");
			break;

		case 41:
			mainMenu_displayedLines = 216;
			screenWidth = 800;
			strcpy(presetMode, "384x216 upscaled");
			break;

		case 42:
			mainMenu_displayedLines = 240;
			screenWidth = 768;
			strcpy(presetMode, "384x240 upscaled");
			break;

		case 43:
			mainMenu_displayedLines = 256;
			screenWidth = 720;
			strcpy(presetMode, "384x256 upscaled");
			break;

		case 44:
			mainMenu_displayedLines = 262;
			screenWidth = 704;
			strcpy(presetMode, "384x262 upscaled");
			break;

		case 45:
			mainMenu_displayedLines = 270;
			screenWidth = 684;
			strcpy(presetMode, "384x270 upscaled");
			break;

		case 46:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "384x200 NTSC");
			break;

		case 47:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "384x200 fullscreen");
			break;

		case 50:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "768x200 upscaled");
			break;

		case 51:
			mainMenu_displayedLines = 216;
			screenWidth = 800;
			strcpy(presetMode, "768x216 upscaled");
			break;

		case 52:
			mainMenu_displayedLines = 240;
			screenWidth = 768;
			strcpy(presetMode, "768x240 upscaled");
			break;

		case 53:
			mainMenu_displayedLines = 256;
			screenWidth = 720;
			strcpy(presetMode, "768x256 upscaled");
			break;

		case 54:
			mainMenu_displayedLines = 262;
			screenWidth = 704;
			strcpy(presetMode, "768x262 upscaled");
			break;

		case 55:
			mainMenu_displayedLines = 270;
			screenWidth = 684;
			strcpy(presetMode, "768x270 upscaled");
			break;

		case 56:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "768x200 NTSC");
			break;

		case 57:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "768x200 fullscreen");
			break;

  	default:
			mainMenu_displayedLines = 240;
			screenWidth = 640;
			strcpy(presetMode, "320x240 upscaled");
			presetModeId = 2;
			break;
	}

	switch(presetModeId / 10)
	{
		case 0:
			mainMenu_displayHires = 0;
			break;

		case 1:
			mainMenu_displayHires = 1;
			break;

		case 2:
			mainMenu_displayHires = 0;
			break;

		case 3:
			mainMenu_displayHires = 1;
			break;

		case 4:
			mainMenu_displayHires = 0;
			break;

		case 5:
			mainMenu_displayHires = 1;
			break;
	}
}

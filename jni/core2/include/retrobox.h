#ifndef _RETROBOX_H
#define _RETROBOX_H

extern char screenshot_dir[];
extern char screenshot_name[];

#define AUDIO_STEREO_SEPARATION_BASE 64
extern float        audio_stereo_separation;
extern unsigned int audio_stereo_main;
extern unsigned int audio_stereo_secondary;
extern int          audio_filter_enabled;

#endif

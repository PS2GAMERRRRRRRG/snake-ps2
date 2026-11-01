#ifndef SOUND_H
#define SOUND_H

#include <tamtypes.h>

/* Initialize the sound subsystem (audsrv + IOP modules).
   Safe to call even if audio hardware is missing — functions below
   will silently do nothing if init failed. */
void sound_init(void);

/* Play the short "food eaten" ding (non-blocking, returns immediately). */
void sound_play_ding(void);

/* Optional: start / stop background music loop. */
void sound_play_music(void);
void sound_stop(void);

/* Toggle mute (music + effects). Starts unmuted. */
void sound_toggle_mute(void);
int  sound_is_muted(void);

#endif
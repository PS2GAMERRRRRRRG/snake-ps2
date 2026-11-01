#ifndef SAVE_H
#define SAVE_H

/* Load high scores from memory card into hi[0..3].
   If no memory card, no file, or read error → hi[] is left untouched. */
void save_load_hiscore(int *hi);

/* Store the 4 high scores (one per difficulty) to the memory card.
   If no memory card or write error → silently does nothing. */
void save_store_hiscore(const int *hi);

#endif
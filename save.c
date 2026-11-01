#include "save.h"
#include <libmc.h>
#include <loadfile.h>
#include <sifrpc.h>
#include <tamtypes.h>
#include <stdio.h>
#include <string.h>

/* Memory card paths */
#define MC_SLOT     0                 /* mc0: -> slot 1 */
#define MC_FOLDER   "/SNAKE"
#define MC_FILE     "/SNAKE/scores.dat"

/* Set to 1 after mcInit() and mcGetInfo() succeed */
static int mc_ready = 0;

/* ---- Init: load IOP modules, check card present ---- */
static int mc_setup(void) {
    if (mc_ready) return 1;

    /* Load memory-card IOP modules (safe even if called twice) */
    SifLoadModule("rom0:XSIO2MAN", 0, NULL);
    SifLoadModule("rom0:MCMAN",   0, NULL);
    SifLoadModule("rom0:MCSERV",  0, NULL);

    /* Init libmc for PS2 memory card */
    mcInit(MC_TYPE_XMC);

    /* Probe the card. mcGetInfo returns >= 0 on success. */
    int type = 0, free_ = 0, format = 0;
    int r = mcGetInfo(MC_SLOT, 0, &type, &free_, &format);
    if (r < 0) return 0;

    /* Wait for the IOP to finish the request */
    mcSync(0, NULL, NULL);

    mc_ready = 1;
    return 1;
}

/* ---- Load ---- */
void save_load_hiscore(int *hi) {
    if (!mc_setup()) return;

    int fd = mcOpen(MC_SLOT, 0, MC_FILE, O_RDONLY);
    if (fd < 0) return;   /* file doesn't exist yet -> keep defaults */

    int buf[4];
    int n = mcRead(fd, buf, sizeof(buf));
    mcClose(fd);
    mcSync(0, NULL, NULL);

    if (n == (int)sizeof(buf)) {
        /* Sanity: clamp negative / absurd values to 0 */
        for (int i = 0; i < 4; i++) {
            if (buf[i] < 0)          buf[i] = 0;
            if (buf[i] > 99999999)   buf[i] = 99999999;
            hi[i] = buf[i];
        }
    }
}

/* ---- Store ---- */
void save_store_hiscore(const int *hi) {
    if (!mc_setup()) return;

    /* Make sure folder exists. mcMkDir returns <0 if it already exists,
       which is fine — we ignore that. */
    mcMkDir(MC_SLOT, 0, MC_FOLDER);
    mcSync(0, NULL, NULL);

    int fd = mcOpen(MC_SLOT, 0, MC_FILE, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) return;

    int buf[4] = { hi[0], hi[1], hi[2], hi[3] };
    mcWrite(fd, buf, sizeof(buf));
    mcClose(fd);
    mcSync(0, NULL, NULL);
}
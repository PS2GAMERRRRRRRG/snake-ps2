/* ---- Screen: Intro ---- */
void render_intro(void) {
    apply_theme(0);
    gsKit_clear(gsGlobal, C_BG); // کل صفحه رو سبز می‌کنه

    /* کشیدن 3 تا مربع بزرگ تو رنگ تیره برای تست */
    rect(100, 100, 100, 100, C_DARK);
    rect(300, 100, 100, 100, C_DARK);
    rect(500, 100, 100, 100, C_DARK);

    /* کشیدن یک مار ساده وسط صفحه */
    for(int i=0; i<5; i++) {
        rect(200 + (i*20), 250, 20, 20, C_DARK); // بدنه مار
    }
    rect(300, 250, 20, 20, C_DARK); // سر مار

    /* به جای متن، یه بار دیگه از تایمر استفاده می‌کنیم که بعد از 3 ثانیه بره صفحه بعد */
    static u64 last = 0;
    static int first_frame = 1;
    u64 now = GetTimerSystemTime();
    if (!first_frame) {
        u64 diff = now - last;
        if (diff < 294912000ULL) {
            g_intro_time += (double)diff / 294912000.0;
        }
    }
    first_frame = 0;
    last = now;

    if (g_intro_time >= 3.0) { // بعد از 3 ثانیه میره صفحه بعد
        state = ST_REQS;
        g_intro_time = 0.0;
    }
}

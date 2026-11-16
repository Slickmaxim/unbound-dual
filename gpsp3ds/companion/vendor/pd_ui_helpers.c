#include "pd_ui_internal.h"
#define HG_OUTLINE 0x414141
#define HG_RING 0xFBFBFB
#define GB_LOOK (pd_cur->titleShadow == PD_NO_SHADOW)
static uint32_t mix(uint32_t a, uint32_t b, int pct) {
    uint32_t out = 0;
    for (int sh = 0; sh <= 16; sh += 8) {
        int x = (int) ((a >> sh) & 0xFF), y = (int) ((b >> sh) & 0xFF);
        out |= (uint32_t) (x + (y - x) * pct / 100) << sh;
    }
    return out;
}
void ui_fade_rect(struct pd_canvas* c, const uint32_t* before, int x, int y, int w, int h, int alpha) {
    for (int yy = 0; yy < h; yy++) {
        int sy = y + yy;
        if (sy < c->cy0 || sy >= c->cy1) continue;
        for (int xx = 0; xx < w; xx++) {
            int sx = x + xx;
            if (sx < c->cx0 || sx >= c->cx1) continue;
            uint32_t* d = &c->px[sy * c->w + sx];
            uint32_t out = 0, was = before[yy * w + xx];
            for (int sh = 0; sh <= 16; sh += 8) {
                uint32_t a = (*d >> sh) & 0xFF, b = (was >> sh) & 0xFF;
                out |= ((a * (uint32_t) alpha + b * (255 - (uint32_t) alpha)) / 255) << sh;
            }
            *d = out;
        }
    }
}
void ui_platinum(struct pd_ui* ui, struct pd_canvas* c, int x, int y, int w, int h, uint32_t fill, uint32_t frame, int id) {
    bool pressed = ui->pressedId == id && id >= 0;
    if (GB_LOOK) {
        pd_layered_box(c, x, y, w, h, 2, pd_cur->chip, pd_cur->chipCount, pressed ? PD_ROW_SELECTED : PD_TITLE_FILL);
        if (id >= 0) ui_add_hit(ui, x, y, w, h, id);
        return;
    }
    if (pressed) {
        fill = mix(fill, 0, 18);
        frame = mix(frame, 0, 18);
    }
    const struct pd_layer layers[3] = { { HG_OUTLINE, 2 }, { HG_RING, 1 }, { frame, 1 } };
    pd_layered_two_tone(c, x, y, w, h, 4, layers, 3, fill, frame);
    if (id >= 0) ui_add_hit(ui, x, y, w, h, id);
}
void ui_platinum_label(struct pd_canvas* c, int x, int y, int w, int h, const char* text) {
    pd_text(c, x + (w - pd_text_width(text)) / 2, pd_text_y(y, h), text, GB_LOOK ? PD_TITLE_TEXT : 0xFFFFFF,
            GB_LOOK ? PD_NO_SHADOW : 0x383838);
}
void ui_platinum_back(struct pd_canvas* c, int x, int y, int w, int h) {
    int ax = x + (w - PD_ICON_BACK.w) / 2, ay = y + (h - PD_ICON_BACK.h) / 2;
    if (!GB_LOOK) ui_mono(c, &PD_ICON_BACK, ax + 1, ay + 1, 1, 0x383838);
    ui_mono(c, &PD_ICON_BACK, ax, ay, 1, GB_LOOK ? PD_TITLE_TEXT : 0xFFFFFF);
}
void ui_box_message(struct pd_ui* ui, struct pd_canvas* c) {
    int w = c->w - 2 * MARGIN, h = 52, x = MARGIN, y = (c->h - h) / 2 - 16;
    int f = pd_title_box(c, x, y, w, h, PD_TITLE_FILL);
    pd_text_wrap(c, x + f + 6, y + f + 2, w - 2 * f - 52, 15, 3, ui->boxMessage, PD_TITLE_TEXT, PD_TITLE_SHADOW);
    ui_button(ui, c, x + w - f - 40, y + (h - 22) / 2, 36, 22, "OK", HIT_BOX_MESSAGE);
}

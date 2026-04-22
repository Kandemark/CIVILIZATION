/**
 * @file nuklear_impl.c
 * @brief Nuklear SDL3 renderer + input backend + Dominion theme
 *
 * Adapted from Nuklear's demo/sdl_renderer/nuklear_sdl_renderer.h
 * for SDL3. Uses SDL_RenderGeometryRaw for anti-aliased rendering.
 */
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "ui/nuklear_ui.h"
#include "display/theme.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static bool nk_ready = false;

/* Clipboard stubs — must be after nuklear.h include for nk_handle type */
static void nk_clip_copy(nk_handle usr, const char *text, int len)
    { (void)usr; (void)text; (void)len; }
static void nk_clip_paste(nk_handle usr, struct nk_text_edit *edit)
    { (void)usr; (void)edit; }

/* ── Device state ─────────────────────────────────────────────── */
struct nk_sdl_vertex {
    float position[2];
    float uv[2];
    float col[4];     /* SDL_FColor = float RGBA for SDL3 */
};

struct nk_sdl_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture tex_null;
    SDL_Texture *font_tex;
};

static struct {
    SDL_Window   *win;
    SDL_Renderer *renderer;
    struct nk_sdl_device dev;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    Uint64 time_of_last_frame;
} nksdl;

/* ── Font atlas upload ────────────────────────────────────────── */
static void device_upload_atlas(const void *image, int width, int height) {
    SDL_Texture *tex = SDL_CreateTexture(nksdl.renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);
    if (!tex) return;
    SDL_UpdateTexture(tex, NULL, image, 4 * width);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    nksdl.dev.font_tex = tex;
}

/* ── Init ─────────────────────────────────────────────────────── */
struct nk_context *nk_ui_init(SDL_Window *win, SDL_Renderer *renderer) {
    nksdl.win = win;
    nksdl.renderer = renderer;
    nksdl.time_of_last_frame = SDL_GetTicks();

    nk_init_default(&nksdl.ctx, NULL);
    nksdl.ctx.clip.copy = nk_clip_copy;
    nksdl.ctx.clip.paste = nk_clip_paste;
    nksdl.ctx.clip.userdata = nk_handle_ptr(NULL);

    nk_buffer_init_default(&nksdl.dev.cmds);

    /* Build font atlas */
    nk_font_atlas_init_default(&nksdl.atlas);
    nk_font_atlas_begin(&nksdl.atlas);

    /* Try loading a system TTF via SDL_RWops */
    struct nk_font *nk_font = NULL;
    const char *ttf_paths[] = {
        "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/liberation/LiberationMono-Regular.ttf",
        NULL
    };
    for (int i = 0; ttf_paths[i] && !nk_font; i++) {
        SDL_IOStream *io = SDL_IOFromFile(ttf_paths[i], "rb");
        if (!io) { fprintf(stderr, "[NK] Cannot open: %s\n", ttf_paths[i]); continue; }
        Sint64 sz = SDL_GetIOSize(io);
        if (sz <= 0) { SDL_CloseIO(io); continue; }
        void *ttf_data = SDL_malloc((size_t)sz);
        if (!ttf_data) { SDL_CloseIO(io); continue; }
        SDL_ReadIO(io, ttf_data, (size_t)sz);
        SDL_CloseIO(io);

        struct nk_font_config cfg = nk_font_config(14.0f);
        cfg.range = nk_font_default_glyph_ranges();
        cfg.oversample_h = 2; cfg.oversample_v = 2;
        nk_font = nk_font_atlas_add_from_memory(&nksdl.atlas, ttf_data, (int)sz, 14.0f, &cfg);
        if (nk_font) fprintf(stderr, "[NK] Loaded TTF: %s\n", ttf_paths[i]);
        SDL_free(ttf_data);
    }

    /* Fallback: built-in default font */
    if (!nk_font) {
        fprintf(stderr, "[NK] Trying built-in default font...\n");
        struct nk_font_config cfg = nk_font_config(13.0f);
        cfg.range = nk_font_default_glyph_ranges();
        nk_font = nk_font_atlas_add_default(&nksdl.atlas, 13.0f, &cfg);
    }

    const void *image; int w, h;
    image = nk_font_atlas_bake(&nksdl.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    if (image && w > 0 && h > 0) {
        device_upload_atlas(image, w, h);
        nk_font_atlas_end(&nksdl.atlas, nk_handle_id((int)nksdl.dev.font_tex),
                          &nksdl.dev.tex_null);
        fprintf(stderr, "[NK] Atlas end OK. font_count=%d default_font=%p\n",
            nksdl.atlas.font_num, (void*)nksdl.atlas.default_font);
    } else {
        nk_font_atlas_end(&nksdl.atlas, nk_handle_id(0), &nksdl.dev.tex_null);
        fprintf(stderr, "[NK] Atlas bake returned NULL image (w=%d h=%d)\n", w, h);
    }

    if (nk_font) {
        nk_style_set_font(&nksdl.ctx, &nk_font->handle);
        nk_ui_theme_dominion(&nksdl.ctx);
        nk_ready = true;
        fprintf(stderr, "[NK] Init OK — using TTF font\n");
    } else if (nksdl.atlas.default_font) {
        nk_style_set_font(&nksdl.ctx, &nksdl.atlas.default_font->handle);
        nk_ui_theme_dominion(&nksdl.ctx);
        nk_ready = true;
        fprintf(stderr, "[NK] Init OK — using default font\n");
    } else {
        nk_ready = false;
        fprintf(stderr, "[NK] Init FAILED — no font\n");
    }
    return &nksdl.ctx;
}

/* ── Input ────────────────────────────────────────────────────── */
int nk_ui_handle_event(const SDL_Event *evt) {
    if (!nk_ready) return 0;
    struct nk_context *ctx = &nksdl.ctx;
    switch (evt->type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        int down = (evt->type == SDL_EVENT_KEY_DOWN);
        switch (evt->key.key) {
        case SDLK_LSHIFT: case SDLK_RSHIFT:
            nk_input_key(ctx, NK_KEY_SHIFT, down); break;
        case SDLK_DELETE:
            nk_input_key(ctx, NK_KEY_DEL, down); break;
        case SDLK_RETURN:
            nk_input_key(ctx, NK_KEY_ENTER, down); break;
        case SDLK_TAB:
            nk_input_key(ctx, NK_KEY_TAB, down); break;
        case SDLK_BACKSPACE:
            nk_input_key(ctx, NK_KEY_BACKSPACE, down); break;
        case SDLK_HOME:
            nk_input_key(ctx, NK_KEY_TEXT_START, down);
            nk_input_key(ctx, NK_KEY_SCROLL_START, down); break;
        case SDLK_END:
            nk_input_key(ctx, NK_KEY_TEXT_END, down);
            nk_input_key(ctx, NK_KEY_SCROLL_END, down); break;
        case SDLK_PAGEDOWN:
            nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down); break;
        case SDLK_PAGEUP:
            nk_input_key(ctx, NK_KEY_SCROLL_UP, down); break;
        case SDLK_Z:
            nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && (evt->key.mod & SDL_KMOD_CTRL)); break;
        case SDLK_R:
            nk_input_key(ctx, NK_KEY_TEXT_REDO, down && (evt->key.mod & SDL_KMOD_CTRL)); break;
        case SDLK_C:
            nk_input_key(ctx, NK_KEY_COPY, down && (evt->key.mod & SDL_KMOD_CTRL)); break;
        case SDLK_V:
            nk_input_key(ctx, NK_KEY_PASTE, down && (evt->key.mod & SDL_KMOD_CTRL)); break;
        case SDLK_X:
            nk_input_key(ctx, NK_KEY_CUT, down && (evt->key.mod & SDL_KMOD_CTRL)); break;
        case SDLK_B:
            nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && (evt->key.mod & SDL_KMOD_CTRL)); break;
        case SDLK_E:
            nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && (evt->key.mod & SDL_KMOD_CTRL)); break;
        case SDLK_UP:
            nk_input_key(ctx, NK_KEY_UP, down); break;
        case SDLK_DOWN:
            nk_input_key(ctx, NK_KEY_DOWN, down); break;
        case SDLK_LEFT: {
            if (down && (evt->key.mod & SDL_KMOD_CTRL))
                nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, 1);
            else nk_input_key(ctx, NK_KEY_LEFT, down);
        } break;
        case SDLK_RIGHT: {
            if (down && (evt->key.mod & SDL_KMOD_CTRL))
                nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, 1);
            else nk_input_key(ctx, NK_KEY_RIGHT, down);
        } break;
        }
    } break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        int down = (evt->type == SDL_EVENT_MOUSE_BUTTON_DOWN);
        if (evt->button.button == SDL_BUTTON_LEFT)
            nk_input_button(ctx, NK_BUTTON_LEFT, (int)evt->button.x, (int)evt->button.y, down);
        if (evt->button.button == SDL_BUTTON_RIGHT)
            nk_input_button(ctx, NK_BUTTON_RIGHT, (int)evt->button.x, (int)evt->button.y, down);
        if (evt->button.button == SDL_BUTTON_MIDDLE)
            nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)evt->button.x, (int)evt->button.y, down);
    } break;
    case SDL_EVENT_MOUSE_MOTION:
        nk_input_motion(ctx, (int)evt->motion.x, (int)evt->motion.y); break;
    case SDL_EVENT_MOUSE_WHEEL:
        nk_input_scroll(ctx, nk_vec2((float)evt->wheel.x, (float)evt->wheel.y)); break;
    case SDL_EVENT_TEXT_INPUT:
        nk_input_glyph(ctx, evt->text.text); break;
    }
    return 0;
}

/* Global context for scenes to reference */
struct nk_context *g_nk_ctx = NULL;

struct nk_context *nk_ui_begin(void) {
    if (!nk_ready) return NULL;
    if (g_nk_ctx) return g_nk_ctx;
    struct nk_context *ctx = &nksdl.ctx;
    Uint64 now = SDL_GetTicks();
    ctx->delta_time_seconds = (float)(now - nksdl.time_of_last_frame) / 1000.0f;
    nksdl.time_of_last_frame = now;
    nk_input_begin(ctx);
    g_nk_ctx = ctx;
    return ctx;
}

/* ── Render ───────────────────────────────────────────────────── */
void nk_ui_end(void) {
    if (!g_nk_ctx) return;
    struct nk_context *ctx = &nksdl.ctx;
    struct nk_sdl_device *dev = &nksdl.dev;
    nk_input_end(ctx);
    g_nk_ctx = NULL;

    const struct nk_draw_command *cmd;
    const nk_draw_index *offset = NULL;
    struct nk_buffer vbuf, ebuf;
    struct nk_convert_config config;
    static const struct nk_draw_vertex_layout_element vertex_layout[] = {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, (size_t)offsetof(struct nk_sdl_vertex, position)},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, (size_t)offsetof(struct nk_sdl_vertex, uv)},
        {NK_VERTEX_COLOR, NK_FORMAT_FLOAT, (size_t)offsetof(struct nk_sdl_vertex, col)},
        {NK_VERTEX_LAYOUT_END}
    };
    int vs = sizeof(struct nk_sdl_vertex);
    size_t vp = offsetof(struct nk_sdl_vertex, position);
    size_t vt = offsetof(struct nk_sdl_vertex, uv);
    size_t vc = offsetof(struct nk_sdl_vertex, col);

    memset(&config, 0, sizeof(config));
    config.vertex_layout = vertex_layout;
    config.vertex_size = sizeof(struct nk_sdl_vertex);
    config.vertex_alignment = NK_ALIGNOF(struct nk_sdl_vertex);
    config.tex_null = dev->tex_null;
    config.circle_segment_count = 22;
    config.curve_segment_count = 22;
    config.arc_segment_count = 22;
    config.global_alpha = 1.0f;
    config.shape_AA = NK_ANTI_ALIASING_ON;
    config.line_AA = NK_ANTI_ALIASING_ON;

    nk_buffer_init_default(&vbuf);
    nk_buffer_init_default(&ebuf);
    nk_convert(ctx, &dev->cmds, &vbuf, &ebuf, &config);

    SDL_Rect saved_clip;
    bool clipping = SDL_RenderClipEnabled(nksdl.renderer);
    SDL_GetRenderClipRect(nksdl.renderer, &saved_clip);

    offset = (const nk_draw_index*)nk_buffer_memory_const(&ebuf);
    nk_draw_foreach(cmd, ctx, &dev->cmds) {
        if (!cmd->elem_count) continue;
        SDL_Rect r = { (int)cmd->clip_rect.x, (int)cmd->clip_rect.y,
                       (int)cmd->clip_rect.w, (int)cmd->clip_rect.h };
        if (r.x < 0) { r.w += r.x; r.x = 0; }
        if (r.y < 0) { r.h += r.y; r.y = 0; }
        SDL_SetRenderClipRect(nksdl.renderer, &r);

        const void *vertices = nk_buffer_memory_const(&vbuf);
        SDL_RenderGeometryRaw(nksdl.renderer,
            (SDL_Texture *)cmd->texture.ptr,
            (const float*)((const nk_byte*)vertices + vp), vs,
            (const SDL_FColor*)((const nk_byte*)vertices + vc), vs,
            (const float*)((const nk_byte*)vertices + vt), vs,
            (int)(vbuf.needed / vs),
            (const void *)offset, (int)cmd->elem_count, 2);
        offset += cmd->elem_count;
    }

    SDL_SetRenderClipRect(nksdl.renderer, &saved_clip);
    if (!clipping) SDL_SetRenderClipRect(nksdl.renderer, NULL);

    nk_clear(ctx);
    nk_buffer_clear(&dev->cmds);
    nk_buffer_free(&vbuf);
    nk_buffer_free(&ebuf);
}

/* ── Theme ─────────────────────────────────────────────────────── */
void nk_ui_theme_dominion(struct nk_context *ctx) {
    struct nk_color table[NK_COLOR_COUNT];
    /* Dark Dominion palette */
    table[NK_COLOR_TEXT]         = nk_rgba(0xf0, 0xf8, 0xff, 255);
    table[NK_COLOR_WINDOW]       = nk_rgba(0x07, 0x0b, 0x1a, 245);
    table[NK_COLOR_HEADER]       = nk_rgba(0x0d, 0x15, 0x25, 240);
    table[NK_COLOR_BORDER]       = nk_rgba(0x00, 0xd4, 0xff, 80);
    table[NK_COLOR_BUTTON]       = nk_rgba(0x0d, 0x15, 0x25, 220);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(0x16, 0x20, 0x33, 240);
    table[NK_COLOR_BUTTON_ACTIVE]= nk_rgba(0x00, 0x4a, 0x7a, 240);
    table[NK_COLOR_TOGGLE]       = nk_rgba(0x0d, 0x15, 0x25, 220);
    table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(0x16, 0x20, 0x33, 240);
    table[NK_COLOR_TOGGLE_CURSOR]= nk_rgba(0x00, 0xd4, 0xff, 255);
    table[NK_COLOR_SELECT]       = nk_rgba(0x00, 0x4a, 0x7a, 200);
    table[NK_COLOR_SELECT_ACTIVE]= nk_rgba(0x00, 0xd4, 0xff, 200);
    table[NK_COLOR_SLIDER]       = nk_rgba(0x0d, 0x15, 0x25, 220);
    table[NK_COLOR_SLIDER_CURSOR]      = nk_rgba(0x00, 0xd4, 0xff, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER]= nk_rgba(0x16, 0x20, 0x33, 240);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE]=nk_rgba(0x00, 0x4a, 0x7a, 240);
    table[NK_COLOR_SCROLLBAR]          = nk_rgba(0x0d, 0x15, 0x25, 200);
    table[NK_COLOR_SCROLLBAR_CURSOR]   = nk_rgba(0x00, 0xd4, 0xff, 180);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER]=nk_rgba(0x16, 0x20, 0x33, 240);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE]=nk_rgba(0x00, 0x4a, 0x7a, 240);
    table[NK_COLOR_PROPERTY]     = nk_rgba(0x07, 0x0b, 0x1a, 200);
    table[NK_COLOR_EDIT]         = nk_rgba(0x0d, 0x15, 0x25, 240);
    table[NK_COLOR_EDIT_CURSOR]  = nk_rgba(0xf0, 0xf8, 0xff, 255);
    table[NK_COLOR_COMBO]        = nk_rgba(0x0d, 0x15, 0x25, 220);
    table[NK_COLOR_CHART]        = nk_rgba(0x00, 0xd4, 0xff, 200);
    table[NK_COLOR_CHART_COLOR]  = nk_rgba(0x00, 0xd4, 0xff, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(0xff, 0xd4, 0x00, 255);
    nk_style_from_table(ctx, table);

    /* Rounding */
    ctx->style.button.rounding = 4.0f;
    ctx->style.window.rounding = 6.0f;
    ctx->style.window.border  = 1.5f;
    ctx->style.window.header.padding = nk_vec2(12, 8);
    ctx->style.window.spacing = nk_vec2(8, 8);
    ctx->style.window.padding = nk_vec2(12, 12);
    ctx->style.button.padding = nk_vec2(16, 8);
}

void nk_ui_shutdown(void) {
    if (nksdl.dev.font_tex) SDL_DestroyTexture(nksdl.dev.font_tex);
    nk_font_atlas_clear(&nksdl.atlas);
    nk_buffer_free(&nksdl.dev.cmds);
    nk_free(&nksdl.ctx);
}

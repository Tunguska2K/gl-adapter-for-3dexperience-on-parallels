// Frame capture, call counters and runtime toggles (SHIM_DEBUG=1 only).
// Included by shim.c after the buffer-storage section.
//
//   %LOCALAPPDATA%\GLAdapter\capture.now   -> capture the next N frames
//                                                   (file content = N, default 1)
//   %LOCALAPPDATA%\GLAdapter\ctl.txt       -> toggles, one per line:
//        skipprog=12,13   onlyprog=12   nodepth  noblend  nocull  noscissor
//        wire  noblit  thumbs=400
//   output: %LOCALAPPDATA%\GLAdapter\cap\frame_<n>.txt, f<n>_<k>.ppm,
//           prog<p>_sh<s>.glsl
#define GL_CURRENT_PROGRAM            0x8B8D
#define GL_PROGRAM_PIPELINE_BINDING   0x825A
#define GL_VIEWPORT                   0x0BA2
#define GL_DEPTH_TEST                 0x0B71
#define GL_DEPTH_FUNC                 0x0B74
#define GL_DEPTH_WRITEMASK            0x0B72
#define GL_BLEND                      0x0BE2
#define GL_BLEND_SRC_RGB              0x80C9
#define GL_BLEND_DST_RGB              0x80C8
#define GL_COLOR_WRITEMASK            0x0C23
#define GL_CULL_FACE                  0x0B44
#define GL_CULL_FACE_MODE             0x0B45
#define GL_FRONT_FACE                 0x0B46
#define GL_STENCIL_TEST               0x0B90
#define GL_SCISSOR_TEST               0x0C11
#define GL_SCISSOR_BOX                0x0C10
#define GL_POLYGON_MODE               0x0B40
#define GL_RASTERIZER_DISCARD         0x8C89
#define GL_FRAMEBUFFER_SRGB           0x8DB9
#define GL_SAMPLES                    0x80A9
#define GL_DRAW_BUFFER0               0x8825
#define GL_READ_BUFFER                0x0C02
#define GL_READ_FRAMEBUFFER           0x8CA8
#define GL_READ_FRAMEBUFFER_BINDING   0x8CAA
#define GL_FRAMEBUFFER                0x8D40
#define GL_COLOR_ATTACHMENT0          0x8CE0
#define GL_DEPTH_ATTACHMENT           0x8D00
#define GL_STENCIL_ATTACHMENT         0x8D20
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE   0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME   0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE      0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE    0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL 0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER 0x8CD4
#define GL_FRAMEBUFFER_COMPLETE       0x8CD5
#define GL_TEXTURE                    0x1702
#define GL_RENDERBUFFER               0x8D41
#define GL_TEXTURE_2D                 0x0DE1
#define GL_TEXTURE_BINDING_2D         0x8069
#define GL_RGBA8                      0x8058
#define GL_RGBA                       0x1908
#define GL_UNSIGNED_BYTE              0x1401
#define GL_INT                        0x1404
#define GL_UNSIGNED_INT               0x1405
#define GL_COLOR_BUFFER_BIT           0x4000
#define GL_NEAREST                    0x2600
#define GL_LINEAR                     0x2601
#define GL_PIXEL_PACK_BUFFER_BINDING  0x88ED
#define GL_PIXEL_PACK_BUFFER          0x88EB
#define GL_PACK_ALIGNMENT             0x0D05
#define GL_PACK_ROW_LENGTH            0x0D02
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_DRAW_INDIRECT_BUFFER_BINDING 0x8F43
#define GL_ACTIVE_TEXTURE             0x84E0
#define GL_TEXTURE0                   0x84C0
#define GL_PRIMITIVE_RESTART          0x8F9D
#define GL_PRIMITIVE_RESTART_FIXED_INDEX 0x8D69
#define GL_CLIP_DISTANCE0             0x3000
#define GL_POLYGON_OFFSET_FILL        0x8037
#define GL_ATTACHED_SHADERS           0x8B85
#define GL_SHADER_TYPE_               0x8B4F
#define GL_FRONT_AND_BACK             0x0408
#define GL_FILL                       0x1B02
#define GL_LINE                       0x1B01
#define GL_PROVOKING_VERTEX           0x8E4F
#define GL_LINE_WIDTH                 0x0B21
#define GL_DEPTH_RANGE                0x0B70

typedef void (WINAPI *PFN_v_e)(GLenum);
typedef GLboolean (WINAPI *PFN_IsEnabled)(GLenum);
typedef void (WINAPI *PFN_GetFloatv)(GLenum, float *);
typedef void (WINAPI *PFN_GetBooleanv)(GLenum, GLboolean *);
typedef void (WINAPI *PFN_PolygonMode)(GLenum, GLenum);
typedef void (WINAPI *PFN_PixelStorei)(GLenum, GLint);
typedef void (WINAPI *PFN_ReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *);
typedef void (WINAPI *PFN_ReadBuffer)(GLenum);
typedef void (WINAPI *PFN_GenTextures)(GLsizei, GLuint *);
typedef void (WINAPI *PFN_DeleteTextures)(GLsizei, const GLuint *);
typedef void (WINAPI *PFN_TexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *);
typedef void (WINAPI *PFN_TexParameteri)(GLenum, GLenum, GLint);
typedef void (WINAPI *PFN_Viewport)(GLint, GLint, GLsizei, GLsizei);
typedef void (WINAPI *PFN_ClearGL)(GLbitfield);
typedef void (WINAPI *PFN_DrawArraysGL)(GLenum, GLint, GLsizei);
typedef void (WINAPI *PFN_DrawElementsGL)(GLenum, GLsizei, GLenum, const void *);

#define GL11(type, name) ((type)g_tbl[IDX_##name])

static CRITICAL_SECTION g_cap_cs;
static volatile LONG g_cap_frames;        // frames still to capture
static int g_cap_min, g_cap_small;        // minimum draws for a frame to count, and how many small frames were refunded
static volatile LONG g_cap_frame_no;      // running frame counter (captured frames)
static volatile LONG g_cap_draw_no;       // draws within the current captured frame
static volatile LONG g_cap_thumbs;        // thumbnails written this frame
static int g_cap_active;                  // capturing right now
static char g_cap_dir[MAX_PATH];
static char *g_cap_log;                   // frame log buffer
static size_t g_cap_log_len, g_cap_log_cap;
static DWORD g_ctl_checked;

// toggles
static struct {
    int nodepth, noblend, nocull, noscissor, wire, noblit, thumbs, nocond, calls;
    GLuint skip[32]; int nskip;
    GLuint only[32]; int nonly;
} g_tog = { .thumbs = 400 };

// call counters (debug mode)
enum { CNT_DrawArrays, CNT_DrawElements, CNT_DrawRangeElements, CNT_DrawElementsBaseVertex, CNT_DrawRangeElementsBaseVertex,
       CNT_DrawArraysInstanced, CNT_DrawElementsInstanced, CNT_DrawElementsInstancedBaseVertex, CNT_DrawArraysInstancedBaseInstance,
       CNT_DrawElementsInstancedBaseInstance, CNT_DrawElementsInstancedBaseVertexBaseInstance, CNT_MultiDrawArrays, CNT_MultiDrawElements,
       CNT_MultiDrawElementsBaseVertex, CNT_DrawArraysIndirect, CNT_DrawElementsIndirect, CNT_MultiDrawArraysIndirect,
       CNT_MultiDrawElementsIndirect, CNT_DispatchCompute, CNT_DispatchComputeIndirect, CNT_BlitFramebuffer, CNT_Clear, CNT_ClearBuffer,
       CNT_BindFramebuffer, CNT_UseProgram, CNT_BindProgramPipeline, CNT_ProvokingVertex, CNT_DrawTransformFeedback, CNT_COUNT };
static const char *const g_cnt_names[CNT_COUNT] = {
    "DrawArrays", "DrawElements", "DrawRangeElements", "DrawElementsBaseVertex", "DrawRangeElementsBaseVertex",
    "DrawArraysInstanced", "DrawElementsInstanced", "DrawElementsInstancedBaseVertex", "DrawArraysInstancedBaseInstance",
    "DrawElementsInstancedBaseInstance", "DrawElementsInstancedBaseVertexBaseInstance", "MultiDrawArrays", "MultiDrawElements",
    "MultiDrawElementsBaseVertex", "DrawArraysIndirect", "DrawElementsIndirect", "MultiDrawArraysIndirect",
    "MultiDrawElementsIndirect", "DispatchCompute", "DispatchComputeIndirect", "BlitFramebuffer", "Clear", "ClearBuffer",
    "BindFramebuffer", "UseProgram", "BindProgramPipeline", "ProvokingVertex", "DrawTransformFeedback" };
static volatile LONG g_cnt[CNT_COUNT];

static void log_counters(void)
{
    char buf[1024]; int n = 0;
    n += wsprintfA(buf + n, "CALLS: tboRanges=%d tboCopies=%d", (int)g_cnt_tbr_ranges, (int)g_cnt_tbr_copies);
    for (int i = 0; i < CNT_COUNT && n < 900; i++)
        if (g_cnt[i]) n += wsprintfA(buf + n, " %s=%d", g_cnt_names[i], (int)g_cnt[i]);
    log_line(buf, NULL);
}

static const char *cap_dir(void)
{
    if (!g_cap_dir[0]) {
        DWORD n = GetEnvironmentVariableA("LOCALAPPDATA", g_cap_dir, MAX_PATH - 40);
        if (n == 0 || n >= MAX_PATH - 40) { lstrcpyA(g_cap_dir, "C:\\Temp"); }
        lstrcatA(g_cap_dir, "\\GLAdapter");
        CreateDirectoryA(g_cap_dir, NULL);
    }
    return g_cap_dir;
}

static void cap_append(const char *s)
{
    size_t l = strlen(s);
    EnterCriticalSection(&g_cap_cs);
    if (g_cap_log_len + l + 2 > g_cap_log_cap) {
        size_t nc = g_cap_log_cap ? g_cap_log_cap * 2 : (1u << 20);
        while (nc < g_cap_log_len + l + 2) nc *= 2;
        char *nb = (char *)HeapAlloc(GetProcessHeap(), 0, nc);
        if (!nb) { LeaveCriticalSection(&g_cap_cs); return; }
        if (g_cap_log) { memcpy(nb, g_cap_log, g_cap_log_len); HeapFree(GetProcessHeap(), 0, g_cap_log); }
        g_cap_log = nb; g_cap_log_cap = nc;
    }
    memcpy(g_cap_log + g_cap_log_len, s, l); g_cap_log_len += l;
    g_cap_log[g_cap_log_len++] = '\r'; g_cap_log[g_cap_log_len++] = '\n';
    LeaveCriticalSection(&g_cap_cs);
}

static void cap_flush_log(LONG frame)
{
    char path[MAX_PATH];
    wsprintfA(path, "%s\\cap\\frame_%d.txt", cap_dir(), (int)frame);
    EnterCriticalSection(&g_cap_cs);
    HANDLE h = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD w;
        if (g_cap_log_len) WriteFile(h, g_cap_log, (DWORD)g_cap_log_len, &w, NULL);
        CloseHandle(h);
    }
    g_cap_log_len = 0;
    LeaveCriticalSection(&g_cap_cs);
}

static void write_file(const char *path, const void *data, DWORD len, const void *data2, DWORD len2)
{
    HANDLE h = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD w;
    if (len) WriteFile(h, data, len, &w, NULL);
    if (len2) WriteFile(h, data2, len2, &w, NULL);
    CloseHandle(h);
}

// parse ctl.txt / capture.now at most twice a second
static void cap_poll_controls(void)
{
    DWORD now = GetTickCount();
    if (now - g_ctl_checked < 500) return;
    g_ctl_checked = now;
    char path[MAX_PATH];
    wsprintfA(path, "%s\\capture.now", cap_dir());
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        char buf[16] = {0}; DWORD r = 0;
        ReadFile(h, buf, 15, &r, NULL);
        CloseHandle(h);
        DeleteFileA(path);
        int n = 0, m = 0; char *c = buf;
        for (; *c >= '0' && *c <= '9'; c++) n = n * 10 + (*c - '0');
        if (*c == ',') for (c++; *c >= '0' && *c <= '9'; c++) m = m * 10 + (*c - '0');   // "N,M": only frames with >= M draws count
        if (n <= 0) n = 1;
        if (n > 20) n = 20;
        wsprintfA(path, "%s\\cap", cap_dir());
        CreateDirectoryA(path, NULL);
        g_cap_min = m; g_cap_small = 0;
        InterlockedExchange(&g_cap_frames, n);
        log_fmt("CAPTURE armed for %d frame(s), min draws %d", n, m);
    }
    wsprintfA(path, "%s\\ctl.txt", cap_dir());
    h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
    static FILETIME last_ft;
    if (h == INVALID_HANDLE_VALUE) {
        if (g_tog.nodepth || g_tog.noblend || g_tog.nocull || g_tog.noscissor || g_tog.wire || g_tog.noblit || g_tog.nocond || g_tog.nskip || g_tog.nonly) {
            memset(&g_tog, 0, sizeof g_tog); g_tog.thumbs = 400;
            memset(&last_ft, 0, sizeof last_ft);
            log_line("TOGGLES cleared", NULL);
        }
        return;
    }
    FILETIME ft = {0};
    GetFileTime(h, NULL, NULL, &ft);
    if (ft.dwLowDateTime == last_ft.dwLowDateTime && ft.dwHighDateTime == last_ft.dwHighDateTime) { CloseHandle(h); return; }
    last_ft = ft;
    char buf[2048] = {0}; DWORD r = 0;
    ReadFile(h, buf, sizeof buf - 1, &r, NULL);
    CloseHandle(h);
    memset(&g_tog, 0, sizeof g_tog); g_tog.thumbs = 400;
    for (char *p = buf; *p; ) {
        while (*p == '\r' || *p == '\n' || *p == ' ') p++;
        char *e = p; while (*e && *e != '\r' && *e != '\n') e++;
        char line[128]; size_t l = (size_t)(e - p); if (l >= sizeof line) l = sizeof line - 1;
        memcpy(line, p, l); line[l] = 0;
        if (!strcmp(line, "nodepth")) g_tog.nodepth = 1;
        else if (!strcmp(line, "noblend")) g_tog.noblend = 1;
        else if (!strcmp(line, "nocull")) g_tog.nocull = 1;
        else if (!strcmp(line, "noscissor")) g_tog.noscissor = 1;
        else if (!strcmp(line, "wire")) g_tog.wire = 1;
        else if (!strcmp(line, "noblit")) g_tog.noblit = 1;
        else if (!strcmp(line, "nocond")) g_tog.nocond = 1;
        else if (!strcmp(line, "calls")) g_tog.calls = 1;
        else if (!strncmp(line, "thumbs=", 7)) g_tog.thumbs = atoi(line + 7);
        else if (!strncmp(line, "skipprog=", 9) || !strncmp(line, "onlyprog=", 9)) {
            int only = line[0] == 'o';
            for (char *q = line + 9; *q; ) {
                GLuint v = (GLuint)atoi(q);
                if (only) { if (g_tog.nonly < 32) g_tog.only[g_tog.nonly++] = v; }
                else      { if (g_tog.nskip < 32) g_tog.skip[g_tog.nskip++] = v; }
                while (*q && *q != ',') q++;
                if (*q == ',') q++;
            }
        }
        p = e;
    }
    log_fmt("TOGGLES: nodepth=%d noblend=%d nocull=%d noscissor=%d wire=%d noblit=%d nocond=%d skip=%d only=%d thumbs=%d calls=%d",
            g_tog.nodepth, g_tog.noblend, g_tog.nocull, g_tog.noscissor, g_tog.wire, g_tog.noblit, g_tog.nocond, g_tog.nskip, g_tog.nonly, g_tog.thumbs, g_tog.calls);
}

// ---------------------------------------------------------------------------
// thumbnails: resolve/downscale the current draw framebuffer into a small
// RGBA8 texture and write it as PPM
// ---------------------------------------------------------------------------
#define THUMB_W 256
typedef struct { HGLRC rc; GLuint fbo_full, tex_full, fbo_thumb, tex_thumb; int fw, fh, tw, th; } CapCtx;
static CapCtx g_capctx[8];
static int g_capctx_n;

static CapCtx *cap_ctx(void)
{
    static PFN_wglGetCurrentContext getcur;
    if (!getcur) getcur = (PFN_wglGetCurrentContext)GetProcAddress(LoadLibraryA("opengl32.dll"), "wglGetCurrentContext");
    HGLRC rc = getcur ? getcur() : NULL;
    for (int i = 0; i < g_capctx_n; i++) if (g_capctx[i].rc == rc) return &g_capctx[i];
    if (g_capctx_n >= 8) return NULL;
    CapCtx *c = &g_capctx[g_capctx_n++];
    memset(c, 0, sizeof *c); c->rc = rc;
    return c;
}

typedef void (WINAPI *PFN_GetShaderiv)(GLuint, GLenum, GLint *);
typedef void (WINAPI *PFN_GetInfoLog)(GLuint, GLsizei, GLsizei *, char *);
typedef void (WINAPI *PFN_GenFramebuffers)(GLsizei, GLuint *);
typedef void (WINAPI *PFN_FramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef GLenum (WINAPI *PFN_CheckFramebufferStatus)(GLenum);
typedef void (WINAPI *PFN_BlitFramebuffer)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
typedef void (WINAPI *PFN_GetFBAttach)(GLenum, GLenum, GLenum, GLint *);

static int cap_make_target(GLuint *fbo, GLuint *tex, int w, int h)
{
    REAL_RET(PFN_GenFramebuffers, genfb, "glGenFramebuffers", 0);
    REAL_RET(PFN_FramebufferTexture2D, fbtex, "glFramebufferTexture2D", 0);
    REAL_RET(PFN_BindFramebuffer, bindfb, "glBindFramebuffer", 0);
    REAL_RET(PFN_CheckFramebufferStatus, check, "glCheckFramebufferStatus", 0);
    GLint oldtex = get_int(GL_TEXTURE_BINDING_2D);
    if (*tex) GL11(PFN_DeleteTextures, glDeleteTextures)(1, tex);
    GL11(PFN_GenTextures, glGenTextures)(1, tex);
    bind_tex(GL_TEXTURE_2D, *tex);
    GL11(PFN_TexImage2D, glTexImage2D)(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    GL11(PFN_TexParameteri, glTexParameteri)(GL_TEXTURE_2D, 0x2801, GL_NEAREST);
    GL11(PFN_TexParameteri, glTexParameteri)(GL_TEXTURE_2D, 0x2800, GL_NEAREST);
    bind_tex(GL_TEXTURE_2D, (GLuint)oldtex);
    if (!*fbo) genfb(1, fbo);
    bindfb(GL_DRAW_FRAMEBUFFER, *fbo);
    fbtex(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *tex, 0);
    return check(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

// previous thumbnails per framebuffer, for change counting
static struct { GLuint fbo; int w, h; unsigned char *px; } g_prev[16];
static int g_prev_n;

static volatile LONG g_in_cond_render;
static int cap_thumbnail(const char *tag, char *info, size_t infosz)
{
    if (g_in_cond_render) { lstrcpynA(info, "cond-render(no thumb)", (int)infosz); return 0; }
    CapCtx *c = cap_ctx();
    ensure_buffer_procs();
    REAL_RET(PFN_BindFramebuffer, bindfb, "glBindFramebuffer", 0);
    REAL_RET(PFN_BlitFramebuffer, blit, "glBlitFramebuffer", 0);
    REAL_RET(PFN_GetFBAttach, getatt, "glGetFramebufferAttachmentParameteriv", 0);
    if (!c) return 0;
    info[0] = 0;
    GLint vp[4]; GL11(PFN_glGetIntegerv, glGetIntegerv)(GL_VIEWPORT, vp);
    GLint draw_fbo = get_int(GL_DRAW_FRAMEBUFFER_BINDING), read_fbo = get_int(GL_READ_FRAMEBUFFER_BINDING);
    if (vp[2] <= 0 || vp[3] <= 0 || vp[2] > 8192 || vp[3] > 8192) { lstrcpynA(info, "badvp", (int)infosz); return 0; }
    int samples = 0;
    GLenum readbuf = GL_COLOR_ATTACHMENT0;
    if (draw_fbo) {
        GLint type = 0, ctype = 0;
        getatt(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
        if (type == 0) { lstrcpynA(info, "nocolor0", (int)infosz); return 0; }
        getatt(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE, &ctype);
        if (ctype == GL_INT || ctype == GL_UNSIGNED_INT) { lstrcpynA(info, "intfmt", (int)infosz); return 0; }
        samples = get_int(GL_SAMPLES);
    } else {
        readbuf = 0x0405; // GL_BACK
        samples = get_int(GL_SAMPLES);
    }
    int w = vp[2], h = vp[3];
    int tw = w > THUMB_W ? THUMB_W : w, th = (int)((long long)h * tw / w); if (th < 1) th = 1;
    GLboolean scissor = GL11(PFN_IsEnabled, glIsEnabled)(GL_SCISSOR_TEST);
    GLboolean srgb = GL11(PFN_IsEnabled, glIsEnabled)(GL_FRAMEBUFFER_SRGB);
    if (scissor) GL11(PFN_v_e, glDisable)(GL_SCISSOR_TEST);
    if (srgb) GL11(PFN_v_e, glDisable)(GL_FRAMEBUFFER_SRGB);
    // source read buffer
    bindfb(GL_READ_FRAMEBUFFER, (GLuint)draw_fbo);
    GLint old_readbuf = get_int(GL_READ_BUFFER);
    GL11(PFN_ReadBuffer, glReadBuffer)(readbuf);
    int ok = 1;
    if (samples > 0) {
        if (c->fw != w || c->fh != h) { ok = cap_make_target(&c->fbo_full, &c->tex_full, w, h); c->fw = w; c->fh = h; bindfb(GL_READ_FRAMEBUFFER, (GLuint)draw_fbo); }
        if (ok) {
            bindfb(GL_DRAW_FRAMEBUFFER, c->fbo_full);
            blit(vp[0], vp[1], vp[0] + w, vp[1] + h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            bindfb(GL_READ_FRAMEBUFFER, c->fbo_full);
            GL11(PFN_ReadBuffer, glReadBuffer)(GL_COLOR_ATTACHMENT0);
            vp[0] = 0; vp[1] = 0;
        }
    }
    if (ok) {
        if (c->tw != tw || c->th != th) { ok = cap_make_target(&c->fbo_thumb, &c->tex_thumb, tw, th); c->tw = tw; c->th = th;
            bindfb(GL_READ_FRAMEBUFFER, samples > 0 ? c->fbo_full : (GLuint)draw_fbo); }
    }
    static unsigned char *buf; static size_t bufcap;
    if (ok) {
        bindfb(GL_DRAW_FRAMEBUFFER, c->fbo_thumb);
        blit(vp[0], vp[1], vp[0] + w, vp[1] + h, 0, 0, tw, th, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        bindfb(GL_READ_FRAMEBUFFER, c->fbo_thumb);
        GL11(PFN_ReadBuffer, glReadBuffer)(GL_COLOR_ATTACHMENT0);
        size_t need = (size_t)tw * th * 4;
        if (bufcap < need) { if (buf) HeapFree(GetProcessHeap(), 0, buf); buf = (unsigned char *)HeapAlloc(GetProcessHeap(), 0, need); bufcap = buf ? need : 0; }
        if (buf) {
            GLint oldpbo = get_int(GL_PIXEL_PACK_BUFFER_BINDING), oldalign = get_int(GL_PACK_ALIGNMENT), oldrow = get_int(GL_PACK_ROW_LENGTH);
            if (oldpbo && p_glBindBuffer) p_glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            GL11(PFN_PixelStorei, glPixelStorei)(GL_PACK_ALIGNMENT, 1);
            GL11(PFN_PixelStorei, glPixelStorei)(GL_PACK_ROW_LENGTH, 0);
            GL11(PFN_ReadPixels, glReadPixels)(0, 0, tw, th, GL_RGBA, GL_UNSIGNED_BYTE, buf);
            GL11(PFN_PixelStorei, glPixelStorei)(GL_PACK_ALIGNMENT, oldalign);
            GL11(PFN_PixelStorei, glPixelStorei)(GL_PACK_ROW_LENGTH, oldrow);
            if (oldpbo && p_glBindBuffer) p_glBindBuffer(GL_PIXEL_PACK_BUFFER, (GLuint)oldpbo);
            // change count against the previous thumbnail of this framebuffer
            int slot = -1;
            for (int i = 0; i < g_prev_n; i++) if (g_prev[i].fbo == (GLuint)draw_fbo) slot = i;
            if (slot < 0 && g_prev_n < 16) { slot = g_prev_n++; g_prev[slot].fbo = (GLuint)draw_fbo; g_prev[slot].px = NULL; g_prev[slot].w = 0; }
            long changed = -1, nonblack = 0;
            size_t npx = (size_t)tw * th;
            for (size_t i = 0; i < npx; i++) if (buf[i * 4] | buf[i * 4 + 1] | buf[i * 4 + 2]) nonblack++;
            if (slot >= 0) {
                if (g_prev[slot].px && g_prev[slot].w == tw && g_prev[slot].h == th) {
                    changed = 0;
                    for (size_t i = 0; i < npx * 4; i += 4)
                        if (abs((int)buf[i] - g_prev[slot].px[i]) > 4 || abs((int)buf[i + 1] - g_prev[slot].px[i + 1]) > 4 || abs((int)buf[i + 2] - g_prev[slot].px[i + 2]) > 4) changed++;
                } else {
                    if (g_prev[slot].px) HeapFree(GetProcessHeap(), 0, g_prev[slot].px);
                    g_prev[slot].px = (unsigned char *)HeapAlloc(GetProcessHeap(), 0, npx * 4);
                    g_prev[slot].w = tw; g_prev[slot].h = th;
                }
                if (g_prev[slot].px) memcpy(g_prev[slot].px, buf, npx * 4);
            }
            // PPM (flip vertically so the top row comes first)
            char head[48]; int hl = wsprintfA(head, "P6\n%d %d\n255\n", tw, th);
            unsigned char *rgb = (unsigned char *)HeapAlloc(GetProcessHeap(), 0, npx * 3);
            if (rgb) {
                for (int y = 0; y < th; y++) {
                    const unsigned char *src = buf + (size_t)(th - 1 - y) * tw * 4;
                    unsigned char *dst = rgb + (size_t)y * tw * 3;
                    for (int x = 0; x < tw; x++) { dst[x * 3] = src[x * 4]; dst[x * 3 + 1] = src[x * 4 + 1]; dst[x * 3 + 2] = src[x * 4 + 2]; }
                }
                char path[MAX_PATH];
                wsprintfA(path, "%s\\cap\\f%d_%s.ppm", cap_dir(), (int)g_cap_frame_no, tag);
                write_file(path, head, (DWORD)hl, rgb, (DWORD)(npx * 3));
                HeapFree(GetProcessHeap(), 0, rgb);
            }
            wsprintfA(info, "thumb=f%d_%s.ppm %dx%d chg=%d lit=%d%%", (int)g_cap_frame_no, tag, tw, th, (int)changed, (int)(nonblack * 100 / (npx ? npx : 1)));
        }
    } else lstrcpynA(info, "target-fail", (int)infosz);
    // restore
    bindfb(GL_READ_FRAMEBUFFER, (GLuint)draw_fbo);
    GL11(PFN_ReadBuffer, glReadBuffer)((GLenum)old_readbuf);
    bindfb(GL_READ_FRAMEBUFFER, (GLuint)read_fbo);
    bindfb(GL_DRAW_FRAMEBUFFER, (GLuint)draw_fbo);
    if (scissor) GL11(PFN_v_e, glEnable)(GL_SCISSOR_TEST);
    if (srgb) GL11(PFN_v_e, glEnable)(GL_FRAMEBUFFER_SRGB);
    return ok;
}

// ---------------------------------------------------------------------------
// shader sources of a program (dumped once per program per capture session)
// ---------------------------------------------------------------------------
static GLuint g_dumped_progs[4096];
static int g_dumped_n;
typedef void (WINAPI *PFN_GetAttachedShaders)(GLuint, GLsizei, GLsizei *, GLuint *);
typedef void (WINAPI *PFN_GetProgramPipelineiv)(GLuint, GLenum, GLint *);

// sources recorded from glShaderSource / glCreateShaderProgramv (fallback when shaders were detached)
#define SRC_SLOTS 65536
static char **g_src_by_shader;      // [shader id] -> source copy
static char **g_src_by_program;     // [program id] -> source copy (CreateShaderProgramv)
static GLuint (*g_attached)[8];     // [program id] -> attached shader ids

static void cap_dump_program(GLuint prog)
{
    if (!prog) return;
    for (int i = 0; i < g_dumped_n; i++) if (g_dumped_progs[i] == prog) return;
    if (g_dumped_n < 4096) g_dumped_progs[g_dumped_n++] = prog;
    REAL(PFN_GetAttachedShaders, getatt, "glGetAttachedShaders");
    REAL(PFN_GetShaderiv, getiv, "glGetShaderiv");
    REAL(PFN_GetInfoLog, getsrc, "glGetShaderSource");
    GLuint sh[8]; GLsizei n = 0;
    getatt(prog, 8, &n, sh);
    char path[MAX_PATH];
    char *buf = (char *)HeapAlloc(GetProcessHeap(), 0, 1 << 20);
    if (!buf) return;
    int wrote = 0;
    for (GLsizei i = 0; i < n; i++) {
        GLint type = 0, len = 0; getiv(sh[i], GL_SHADER_TYPE_, &type);
        GLsizei got = 0; getsrc(sh[i], (1 << 20) - 1, &got, buf); buf[got > 0 ? got : 0] = 0;
        if (got <= 0 && g_src_by_shader && sh[i] < SRC_SLOTS && g_src_by_shader[sh[i]]) { lstrcpynA(buf, g_src_by_shader[sh[i]], (1 << 20) - 1); got = lstrlenA(buf); }
        wsprintfA(path, "%s\\cap\\prog%u_sh%u_%x.glsl", cap_dir(), prog, sh[i], (unsigned)type);
        write_file(path, buf, (DWORD)(got > 0 ? got : 0), NULL, 0);
        (void)len; wrote++;
    }
    if (!wrote && g_attached && prog < SRC_SLOTS) {
        for (int i = 0; i < 8 && g_attached[prog][i]; i++) {
            GLuint s = g_attached[prog][i];
            const char *src = (g_src_by_shader && s < SRC_SLOTS) ? g_src_by_shader[s] : NULL;
            wsprintfA(path, "%s\\cap\\prog%u_sh%u_rec.glsl", cap_dir(), prog, s);
            write_file(path, src ? src : "", src ? (DWORD)lstrlenA(src) : 0, NULL, 0);
            wrote++;
        }
    }
    if (!wrote && g_src_by_program && prog < SRC_SLOTS && g_src_by_program[prog]) {
        wsprintfA(path, "%s\\cap\\prog%u_sep.glsl", cap_dir(), prog);
        write_file(path, g_src_by_program[prog], (DWORD)lstrlenA(g_src_by_program[prog]), NULL, 0);
        wrote++;
    }
    if (!wrote) { wsprintfA(path, "%s\\cap\\prog%u_nosource.txt", cap_dir(), prog); write_file(path, "no source available", 19, NULL, 0); }
    HeapFree(GetProcessHeap(), 0, buf);
}

static void record_source(char ***table, GLuint id, GLsizei count, const char *const *strings, const GLint *lengths)
{
    if (id >= SRC_SLOTS) return;
    if (!*table) *table = (char **)VirtualAlloc(NULL, SRC_SLOTS * sizeof(char *), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!*table) return;
    size_t total = 0;
    for (GLsizei i = 0; i < count; i++) {
        if (!strings || !strings[i]) continue;
        total += (lengths && lengths[i] >= 0) ? (size_t)lengths[i] : strlen(strings[i]);
    }
    char *buf = (char *)HeapAlloc(GetProcessHeap(), 0, total + 1);
    if (!buf) return;
    size_t o = 0;
    for (GLsizei i = 0; i < count; i++) {
        if (!strings || !strings[i]) continue;
        size_t l = (lengths && lengths[i] >= 0) ? (size_t)lengths[i] : strlen(strings[i]);
        memcpy(buf + o, strings[i], l); o += l;
    }
    buf[o] = 0;
    if ((*table)[id]) HeapFree(GetProcessHeap(), 0, (*table)[id]);
    (*table)[id] = buf;
}

// ---------------------------------------------------------------------------
// per-draw bookkeeping
// ---------------------------------------------------------------------------
typedef struct { int skip; GLboolean depth, blend, cull, scissor; GLint pm; GLuint prog; GLenum err_before; } DrawSave;

static int prog_listed(const GLuint *list, int n, GLuint p)
{
    for (int i = 0; i < n; i++) if (list[i] == p) return 1;
    return 0;
}

// Second detail line per draw: GL error, uniform-block ranges (with node matrix / material colours),
// enabled vertex attributes with their binding state, and the first vertex position of the draw.
typedef GLenum (WINAPI *CAPFN_GetError)(void);
typedef void (WINAPI *CAPFN_GetIntegeri_v)(GLenum, GLuint, GLint *);
typedef void (WINAPI *CAPFN_GetInteger64i_v)(GLenum, GLuint, LONGLONG *);
typedef void (WINAPI *CAPFN_BindBuffer)(GLenum, GLuint);
typedef void (WINAPI *CAPFN_GetBufferSubData)(GLenum, GLintptr, GLsizeiptr, void *);
typedef void (WINAPI *CAPFN_GetBufferParameteriv)(GLenum, GLenum, GLint *);
typedef void (WINAPI *CAPFN_GetVertexAttribiv)(GLuint, GLenum, GLint *);

static int f1000(const float *p, int i)
{
    float v = p[i];
    if (v != v) return -999999;
    if (v > 999999.f) v = 999999.f;
    if (v < -999999.f) v = -999999.f;
    return (int)(v * 1000.f);
}

typedef void (WINAPI *CAPFN_ValidateProgram)(GLuint);
typedef void (WINAPI *CAPFN_GetProgramiv)(GLuint, GLenum, GLint *);
typedef void (WINAPI *CAPFN_GetProgramInfoLog)(GLuint, GLsizei, GLsizei *, char *);

static void cap_draw_detail(LONG k, const char *desc, GLuint prog, GLenum err_before)
{
    REAL(CAPFN_GetError, geterr, "glGetError");
    REAL(CAPFN_GetIntegeri_v, geti, "glGetIntegeri_v");
    REAL(CAPFN_GetInteger64i_v, geti64, "glGetInteger64i_v");
    REAL(CAPFN_BindBuffer, bindbuf, "glBindBuffer");
    REAL(CAPFN_GetBufferSubData, getsub, "glGetBufferSubData");
    REAL(CAPFN_GetBufferParameteriv, getbp, "glGetBufferParameteriv");
    REAL(CAPFN_GetVertexAttribiv, getva, "glGetVertexAttribiv");
    char l[1600]; int n = 0;
    GLenum err = geterr();
    n += wsprintfA(l + n, "  V%d err=%x(before=%x)", (int)k, err, err_before);
    if (err && prog && !(prog & 0x80000000u)) {
        REAL(CAPFN_ValidateProgram, validate, "glValidateProgram");
        REAL(CAPFN_GetProgramiv, getpiv, "glGetProgramiv");
        REAL(CAPFN_GetProgramInfoLog, getlog, "glGetProgramInfoLog");
        GLint vs = -1, ls = -1; char msg[200] = ""; GLsizei ml = 0;
        validate(prog); getpiv(prog, 0x8B83 /*VALIDATE_STATUS*/, &vs); getpiv(prog, 0x8B82 /*LINK_STATUS*/, &ls);
        getlog(prog, sizeof msg - 1, &ml, msg);
        for (char *c = msg; *c; c++) if (*c == '\r' || *c == '\n') *c = ' ';
        n += wsprintfA(l + n, " link=%d validate=%d log=[%s]", ls, vs, msg);
        geterr();
    }
    for (GLuint b = 0; b < 4; b++) {
        GLint buf = 0; LONGLONG start = 0, size = 0;
        geti(0x90D3 /*SHADER_STORAGE_BUFFER_BINDING*/, b, &buf);
        if (!buf) continue;
        geti64(0x90D4, b, &start); geti64(0x90D5, b, &size);
        n += wsprintfA(l + n, " s%u=%d@%d+%d", b, buf, (int)start, (int)size);
    }
    GLint old = get_int(0x8F36 /*COPY_READ_BUFFER*/);
    for (GLuint b = 0; b < 5; b++) {
        GLint buf = 0; LONGLONG start = 0, size = 0;
        geti(0x8A28 /*UNIFORM_BUFFER_BINDING*/, b, &buf);
        if (!buf) { n += wsprintfA(l + n, " u%u=-", b); continue; }
        geti64(0x8A29 /*UNIFORM_BUFFER_START*/, b, &start); geti64(0x8A2A /*UNIFORM_BUFFER_SIZE*/, b, &size);
        n += wsprintfA(l + n, " u%u=%d@%d+%d", b, buf, (int)start, (int)size);
        if (b == 1 || b == 3) {
            float f[24]; memset(f, 0, sizeof f);
            GLint bs = 0;
            bindbuf(0x8F36, (GLuint)buf);
            getbp(0x8F36, 0x8764 /*BUFFER_SIZE*/, &bs);
            if ((LONGLONG)bs >= start + 96) {
                getsub(0x8F36, (GLintptr)start, 96, f);
                if (b == 1) n += wsprintfA(l + n, "[m00=%d m11=%d m22=%d m33=%d t=%d,%d,%d]", f1000(f, 0), f1000(f, 5), f1000(f, 10), f1000(f, 15), f1000(f, 12), f1000(f, 13), f1000(f, 14));
                else n += wsprintfA(l + n, "[amb=%d,%d,%d,%d dif=%d,%d,%d,%d]", f1000(f, 16), f1000(f, 17), f1000(f, 18), f1000(f, 19), f1000(f, 20), f1000(f, 21), f1000(f, 22), f1000(f, 23));
            } else n += wsprintfA(l + n, "[bs=%d]", bs);
        }
    }
    int first = -1; const char *fp = strstr(desc, "first="); if (fp) first = atoi(fp + 6);
    int dumped_v0 = 0;
    for (GLuint a = 0; a < 8 && n < 1100; a++) {
        GLint on = 0; getva(a, 0x8622, &on); if (!on) continue;
        GLint sz = 0, ty = 0, nm = 0, in = 0, bd = 0, ro = 0;
        getva(a, 0x8623, &sz); getva(a, 0x8625, &ty); getva(a, 0x886A, &nm); getva(a, 0x88FD, &in); getva(a, 0x82D4, &bd); getva(a, 0x82D5, &ro);
        GLint vbuf = 0, vst = 0, vdiv = 0; LONGLONG voff = 0;
        geti(0x8F4F, (GLuint)bd, &vbuf); geti64(0x82D7, (GLuint)bd, &voff); geti(0x82D8, (GLuint)bd, &vst); geti(0x82D6, (GLuint)bd, &vdiv);
        n += wsprintfA(l + n, " a%u=%dx%x%s%s+%d b%d(buf=%d off=%d st=%d div=%d)", a, sz, ty, nm ? "n" : "", in ? "i" : "", ro, bd, vbuf, (int)voff, vst, vdiv);
        if (!dumped_v0 && first >= 0 && vbuf && vst > 0) {
            dumped_v0 = 1;
            float f[4]; memset(f, 0, sizeof f);
            LONGLONG off = voff + ro + (LONGLONG)first * vst;
            GLint bs = 0;
            bindbuf(0x8F36, (GLuint)vbuf);
            getbp(0x8F36, 0x8764, &bs);
            if (off + 12 <= (LONGLONG)bs) {
                getsub(0x8F36, (GLintptr)off, 12, f);
                if (ty == 0x1406) n += wsprintfA(l + n, " v0=%d,%d,%d", f1000(f, 0), f1000(f, 1), f1000(f, 2));
                else { const unsigned *u = (const unsigned *)f; n += wsprintfA(l + n, " v0raw=%08x,%08x,%08x", u[0], u[1], u[2]); }
            } else n += wsprintfA(l + n, " v0=beyond(bs=%d off=%d)", bs, (int)off);
        }
    }
    bindbuf(0x8F36, (GLuint)old);
    // buffer textures on units 0..3 (and the adapter's range emulation records for them)
    {
        typedef void (WINAPI *CAPFN_ActiveTexture)(GLenum);
        REAL(CAPFN_ActiveTexture, actt, "glActiveTexture");
        typedef void (WINAPI *CAPFN_GetTexLevelParameteriv)(GLenum, GLint, GLenum, GLint *);
        REAL(CAPFN_GetTexLevelParameteriv, gettl, "glGetTexLevelParameteriv");
        GLint oldu = get_int(0x84E0 /*ACTIVE_TEXTURE*/);
        for (GLuint u = 0; u < 4 && n < 1200; u++) {
            actt(0x84C0 /*GL_TEXTURE0*/ + u);
            GLint t = get_int(0x8C2C /*TEXTURE_BINDING_BUFFER*/);
            if (!t) continue;
            GLint store = 0, bs = 0; gettl(0x8C2A /*TEXTURE_BUFFER*/, 0, 0x8C2D /*TEXTURE_BUFFER_DATA_STORE_BINDING*/, &store);
            n += wsprintfA(l + n, " tb%u=%d(buf=%d", u, t, store);
            if (store) {
                float f[8]; memset(f, 0, sizeof f);
                bindbuf(0x8F36, (GLuint)store); getbp(0x8F36, 0x8764, &bs);
                if (bs >= 32) getsub(0x8F36, 0, 32, f);
                n += wsprintfA(l + n, " size=%d v=%d,%d,%d,%d|%d,%d,%d,%d", bs, f1000(f, 0), f1000(f, 1), f1000(f, 2), f1000(f, 3), f1000(f, 4), f1000(f, 5), f1000(f, 6), f1000(f, 7));
            }
            for (int i = 0; i < g_tbr_n; i++) if (g_tbr[i].tex == (GLuint)t) { n += wsprintfA(l + n, " rng src=%u off=%d sz=%d dirty=%d", g_tbr[i].src, (int)g_tbr[i].off, (int)g_tbr[i].size, g_tbr[i].dirty); break; }
            n += wsprintfA(l + n, ")");
        }
        actt((GLenum)oldu);
    }
    // after a compute dispatch: the first output matrix (binding 1) and the first input double matrix + uniform block doubles
    if (strstr(desc, "DispatchCompute") && n < 1200) {
        GLint b1 = 0, b0 = 0, u0 = 0; LONGLONG st = 0;
        geti(0x90D3, 1, &b1); geti(0x90D3, 0, &b0); geti(0x8A28, 0, &u0); geti64(0x8A29, 0, &st);
        if (b1) { float f[16]; memset(f, 0, sizeof f); bindbuf(0x8F36, (GLuint)b1); getsub(0x8F36, 0, 64, f); n += wsprintfA(l + n, " out0=[%d,%d,%d,%d|%d,%d,%d,%d|%d,%d,%d,%d|%d,%d,%d,%d]", f1000(f, 0), f1000(f, 1), f1000(f, 2), f1000(f, 3), f1000(f, 4), f1000(f, 5), f1000(f, 6), f1000(f, 7), f1000(f, 8), f1000(f, 9), f1000(f, 10), f1000(f, 11), f1000(f, 12), f1000(f, 13), f1000(f, 14), f1000(f, 15)); }
        if (b0) { double dd[16]; memset(dd, 0, sizeof dd); bindbuf(0x8F36, (GLuint)b0); getsub(0x8F36, 0, 128, dd); n += wsprintfA(l + n, " in0=[%d,%d,%d,%d|..|%d,%d,%d,%d]", (int)(dd[0] * 1000), (int)(dd[1] * 1000), (int)(dd[2] * 1000), (int)(dd[3] * 1000), (int)(dd[12] * 1000), (int)(dd[13] * 1000), (int)(dd[14] * 1000), (int)(dd[15] * 1000)); }
        if (u0) { double dd[16]; int ii[4]; memset(dd, 0, sizeof dd); memset(ii, 0, sizeof ii); bindbuf(0x8F36, (GLuint)u0); getsub(0x8F36, (GLintptr)st, 128, dd); getsub(0x8F36, (GLintptr)st + 128, 16, ii); n += wsprintfA(l + n, " ubo0=[view %d,%d,%d,%d|..|%d,%d,%d,%d batch=%d nb=%d]", (int)(dd[0] * 1000), (int)(dd[1] * 1000), (int)(dd[2] * 1000), (int)(dd[3] * 1000), (int)(dd[12] * 1000), (int)(dd[13] * 1000), (int)(dd[14] * 1000), (int)(dd[15] * 1000), ii[0], ii[1]); }
    }
    // depth-only framebuffer (occlusion test): sample the depth at five points
    if (get_int(GL_DRAW_FRAMEBUFFER_BINDING) && n < 1300) {
        REAL(PFN_GetFBAttach, getatt2, "glGetFramebufferAttachmentParameteriv");
        GLint ct = 0, dt = 0; getatt2(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &ct); getatt2(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &dt);
        if (!ct && dt) {
            typedef void (WINAPI *CAPFN_ReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *);
            typedef void (WINAPI *CAPFN_BindFramebuffer)(GLenum, GLuint);
            REAL(CAPFN_ReadPixels, rp, "glReadPixels");
            REAL(CAPFN_BindFramebuffer, bfb, "glBindFramebuffer");
            GLint vp[4]; GL11(PFN_glGetIntegerv, glGetIntegerv)(GL_VIEWPORT, vp);
            GLint oldread = get_int(GL_READ_FRAMEBUFFER_BINDING);
            bfb(GL_READ_FRAMEBUFFER, (GLuint)get_int(GL_DRAW_FRAMEBUFFER_BINDING));
            float z[5] = { 0 }; int xs[5] = { vp[2] / 2, vp[2] / 4, 3 * vp[2] / 4, vp[2] / 2, vp[2] / 2 }, ys[5] = { vp[3] / 2, vp[3] / 2, vp[3] / 2, vp[3] / 4, 3 * vp[3] / 4 };
            for (int i = 0; i < 5; i++) rp(xs[i], ys[i], 1, 1, 0x1902 /*DEPTH_COMPONENT*/, 0x1406 /*FLOAT*/, &z[i]);
            bfb(GL_READ_FRAMEBUFFER, (GLuint)oldread);
            n += wsprintfA(l + n, " depth5=%d,%d,%d,%d,%d(x1000)", (int)(z[0] * 1000), (int)(z[1] * 1000), (int)(z[2] * 1000), (int)(z[3] * 1000), (int)(z[4] * 1000));
        }
    }
    // storage buffer 0 contents after the draw (visibility flags of the occlusion pass are small)
    {
        GLint buf = 0; LONGLONG start = 0, size = 0;
        geti(0x90D3, 0, &buf);
        if (buf) {
            geti64(0x90D4, 0, &start); geti64(0x90D5, 0, &size);
            GLint bs = 0; bindbuf(0x8F36, (GLuint)buf); getbp(0x8F36, 0x8764, &bs);
            if (size <= 0) size = bs - start;
            if (size > 0 && size <= 4096 && n < 1300) {
                unsigned v[8]; memset(v, 0, sizeof v);
                getsub(0x8F36, (GLintptr)start, size < 32 ? (GLsizeiptr)size : 32, v);
                n += wsprintfA(l + n, " s0data=%x,%x,%x,%x,%x,%x,%x,%x", v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
            }
        }
    }
    bindbuf(0x8F36, (GLuint)old);
    GLenum err2 = geterr();
    if (err2) n += wsprintfA(l + n, " qerr=%x", err2);
    cap_append(l);
}

static void draw_begin(DrawSave *s, int cnt)
{
    InterlockedIncrement(&g_cnt[cnt]);
    memset(s, 0, sizeof *s);
    cap_poll_controls();
    int tog = g_tog.nodepth || g_tog.noblend || g_tog.nocull || g_tog.noscissor || g_tog.wire || g_tog.nskip || g_tog.nonly;
    if (!tog && !g_cap_active) return;
    if (g_in_cond_render) return;   // no GL queries inside conditional-render blocks (Parallels drops the draw)
    s->prog = (GLuint)get_int(GL_CURRENT_PROGRAM);
    if (!s->prog) { GLint pipe = get_int(GL_PROGRAM_PIPELINE_BINDING); if (pipe) s->prog = 0x80000000u | (GLuint)pipe; }
    if (g_cap_active) { static CAPFN_GetError ge; if (!ge) ge = (CAPFN_GetError)real_proc("glGetError"); if (ge) s->err_before = ge(); }
    if (g_tog.nskip && prog_listed(g_tog.skip, g_tog.nskip, s->prog)) { s->skip = 1; return; }
    if (g_tog.nonly && !prog_listed(g_tog.only, g_tog.nonly, s->prog)) { s->skip = 1; return; }
    if (g_tog.nodepth) { s->depth = GL11(PFN_IsEnabled, glIsEnabled)(GL_DEPTH_TEST); if (s->depth) GL11(PFN_v_e, glDisable)(GL_DEPTH_TEST); }
    if (g_tog.noblend) { s->blend = GL11(PFN_IsEnabled, glIsEnabled)(GL_BLEND); if (s->blend) GL11(PFN_v_e, glDisable)(GL_BLEND); }
    if (g_tog.nocull) { s->cull = GL11(PFN_IsEnabled, glIsEnabled)(GL_CULL_FACE); if (s->cull) GL11(PFN_v_e, glDisable)(GL_CULL_FACE); }
    if (g_tog.noscissor) { s->scissor = GL11(PFN_IsEnabled, glIsEnabled)(GL_SCISSOR_TEST); if (s->scissor) GL11(PFN_v_e, glDisable)(GL_SCISSOR_TEST); }
    if (g_tog.wire) { GLint pm[2] = { GL_FILL, GL_FILL }; GL11(PFN_glGetIntegerv, glGetIntegerv)(GL_POLYGON_MODE, pm); s->pm = pm[0]; GL11(PFN_PolygonMode, glPolygonMode)(GL_FRONT_AND_BACK, GL_LINE); }
}

static void draw_end(DrawSave *s, const char *desc)
{
    if (s->skip) return;
    if (g_in_cond_render) {
        if (g_cap_active) { char l[300]; wsprintfA(l, "D%d t=%u (inside conditional render, no state query) | %s", (int)InterlockedIncrement(&g_cap_draw_no), (unsigned)GetCurrentThreadId(), desc); cap_append(l); }
        return;
    }
    if (g_tog.nodepth && s->depth) GL11(PFN_v_e, glEnable)(GL_DEPTH_TEST);
    if (g_tog.noblend && s->blend) GL11(PFN_v_e, glEnable)(GL_BLEND);
    if (g_tog.nocull && s->cull) GL11(PFN_v_e, glEnable)(GL_CULL_FACE);
    if (g_tog.noscissor && s->scissor) GL11(PFN_v_e, glEnable)(GL_SCISSOR_TEST);
    if (g_tog.wire) GL11(PFN_PolygonMode, glPolygonMode)(GL_FRONT_AND_BACK, (GLenum)s->pm);
    if (!g_cap_active) return;
    LONG k = InterlockedIncrement(&g_cap_draw_no);
    // state snapshot
    GLint vp[4], sc[4], cmask[4], pm[2]; GLboolean dmask = 0;
    GL11(PFN_glGetIntegerv, glGetIntegerv)(GL_VIEWPORT, vp);
    GL11(PFN_glGetIntegerv, glGetIntegerv)(GL_SCISSOR_BOX, sc);
    GL11(PFN_glGetIntegerv, glGetIntegerv)(GL_COLOR_WRITEMASK, cmask);
    GL11(PFN_glGetIntegerv, glGetIntegerv)(GL_POLYGON_MODE, pm);
    GL11(PFN_GetBooleanv, glGetBooleanv)(GL_DEPTH_WRITEMASK, &dmask);
    PFN_IsEnabled en = GL11(PFN_IsEnabled, glIsEnabled);
    int clip = 0; for (int i = 0; i < 8; i++) if (en(GL_CLIP_DISTANCE0 + i)) clip |= 1 << i;
    GLint fbo = get_int(GL_DRAW_FRAMEBUFFER_BINDING);
    GLint att_type = 0, att_name = 0, dep_type = 0, dep_name = 0, ctype = 0, rsize = 0, dsize = 0;
    {
        REAL(PFN_GetFBAttach, getatt, "glGetFramebufferAttachmentParameteriv");
        if (fbo) {
            getatt(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &att_type);
            if (att_type) {
                getatt(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &att_name);
                getatt(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE, &ctype);
                getatt(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &rsize);
            }
            getatt(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &dep_type);
            if (dep_type) {
                getatt(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &dep_name);
                getatt(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &dsize);
            }
        }
    }
    char line[900];
    wsprintfA(line, "D%d t=%u prog=%u%s vao=%d fbo=%d col0=%s%d(ct=%x r%d) depth=%s%d(d%d) smp=%d dbuf0=%x vp=%d,%d,%d,%d sc=%s%d,%d,%d,%d"
                    " dtest=%d func=%x dmask=%d blend=%d(%x,%x) cmask=%d%d%d%d cull=%d(%x,ff=%x) sten=%d pmode=%x discard=%d srgb=%d clip=%x poff=%d prestart=%d/%d ebo=%d ind=%d | %s",
              (int)k, (unsigned)GetCurrentThreadId(), s->prog & 0x7fffffffu, (s->prog & 0x80000000u) ? "(pipeline)" : "",
              get_int(GL_VERTEX_ARRAY_BINDING), fbo,
              att_type == GL_TEXTURE ? "tex" : att_type == GL_RENDERBUFFER ? "rb" : att_type ? "?" : "none", att_name, ctype, rsize,
              dep_type == GL_TEXTURE ? "tex" : dep_type == GL_RENDERBUFFER ? "rb" : dep_type ? "?" : "none", dep_name, dsize,
              get_int(GL_SAMPLES), get_int(GL_DRAW_BUFFER0), vp[0], vp[1], vp[2], vp[3],
              en(GL_SCISSOR_TEST) ? "on:" : "off:", sc[0], sc[1], sc[2], sc[3],
              en(GL_DEPTH_TEST), get_int(GL_DEPTH_FUNC), dmask, en(GL_BLEND), get_int(GL_BLEND_SRC_RGB), get_int(GL_BLEND_DST_RGB),
              cmask[0], cmask[1], cmask[2], cmask[3], en(GL_CULL_FACE), get_int(GL_CULL_FACE_MODE), get_int(GL_FRONT_FACE),
              en(GL_STENCIL_TEST), pm[0], en(GL_RASTERIZER_DISCARD), en(GL_FRAMEBUFFER_SRGB), clip, en(GL_POLYGON_OFFSET_FILL),
              en(GL_PRIMITIVE_RESTART), en(GL_PRIMITIVE_RESTART_FIXED_INDEX), get_int(GL_ELEMENT_ARRAY_BUFFER_BINDING), get_int(GL_DRAW_INDIRECT_BUFFER_BINDING), desc);
    char info[128] = "";
    LONG nth = InterlockedIncrement(&g_cap_thumbs);
    if (nth <= g_tog.thumbs || (nth % 25) == 0) {
        char tag[24]; wsprintfA(tag, "d%d", (int)k);
        cap_thumbnail(tag, info, sizeof info);
    }
    if (info[0]) { lstrcatA(line, " "); lstrcatA(line, info); }
    cap_append(line);
    cap_draw_detail(k, desc, s->prog, s->err_before);
    cap_dump_program(s->prog & 0x7fffffffu);
    if (s->prog & 0x80000000u) {
        REAL(PFN_GetProgramPipelineiv, getpipe, "glGetProgramPipelineiv");
        GLint vs = 0, fs = 0;
        getpipe(s->prog & 0x7fffffffu, 0x8B31, &vs); getpipe(s->prog & 0x7fffffffu, 0x8B30, &fs);
        cap_dump_program((GLuint)vs); cap_dump_program((GLuint)fs);
        char pl[64]; wsprintfA(pl, "  pipeline %u: vs=%d fs=%d", s->prog & 0x7fffffffu, vs, fs); cap_append(pl);
    }
}

static void cap_event(const char *desc, int thumb)
{
    if (!g_cap_active) return;
    char line[400], info[128] = "";
    LONG k = InterlockedIncrement(&g_cap_draw_no);
    if (thumb) { char tag[24]; wsprintfA(tag, "e%d", (int)k); cap_thumbnail(tag, info, sizeof info); }
    wsprintfA(line, "E%d t=%u fbo=%d %s %s", (int)k, (unsigned)GetCurrentThreadId(), get_int(GL_DRAW_FRAMEBUFFER_BINDING), desc, info);
    cap_append(line);
}

// frame boundary (called from wglSwapBuffers)
static void cap_frame_end(HDC dc)
{
    if (!g_debug) return;
    cap_poll_controls();
    if (g_cap_active) {
        char l[160]; wsprintfA(l, "SWAP dc=%p draws=%d", (void *)dc, (int)g_cap_draw_no); cap_append(l);
        char info[128] = "";
        cap_thumbnail("swap", info, sizeof info);
        if (info[0]) cap_append(info);
        cap_flush_log(g_cap_frame_no);
        log_fmt("CAPTURE frame %d written (%d draws/events)", (int)g_cap_frame_no, (int)g_cap_draw_no);
        g_cap_active = 0;
        if (g_cap_min > 0 && g_cap_draw_no < g_cap_min && g_cap_small < 300) { g_cap_small++; InterlockedIncrement(&g_cap_frames); }   // small frame: does not use up the budget
    }
    if (g_cap_frames > 0) {
        InterlockedDecrement(&g_cap_frames);
        InterlockedIncrement(&g_cap_frame_no);
        InterlockedExchange(&g_cap_draw_no, 0);
        InterlockedExchange(&g_cap_thumbs, 0);
        g_dumped_n = 0;
        g_cap_active = 1;
        InterlockedExchange(&g_tr_lines, 0);
        char l[64]; wsprintfA(l, "FRAME %d begin%s", (int)g_cap_frame_no, g_tog.calls ? " (call trace on)" : ""); cap_append(l);
    }
}

// ---------------------------------------------------------------------------
// wrappers (installed in debug mode)
// ---------------------------------------------------------------------------
#define DESC(...) char desc[200]; if (g_cap_active) wsprintfA(desc, __VA_ARGS__); else desc[0] = 0

void WINAPI glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    shim_init(); shim_sync_before_gpu();
    if (!g_debug) { GL11(PFN_DrawArraysGL, glDrawArrays)(mode, first, count); return; }
    DrawSave s; draw_begin(&s, CNT_DrawArrays); if (s.skip) return;
    GL11(PFN_DrawArraysGL, glDrawArrays)(mode, first, count);
    DESC("DrawArrays mode=%x first=%d count=%d", mode, first, count); draw_end(&s, desc);
}
void WINAPI glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *idx)
{
    shim_init(); shim_sync_before_gpu();
    if (!g_debug) { GL11(PFN_DrawElementsGL, glDrawElements)(mode, count, type, idx); return; }
    DrawSave s; draw_begin(&s, CNT_DrawElements); if (s.skip) return;
    GL11(PFN_DrawElementsGL, glDrawElements)(mode, count, type, idx);
    DESC("DrawElements mode=%x count=%d type=%x off=%u", mode, count, type, (unsigned)(ULONG_PTR)idx); draw_end(&s, desc);
}
void WINAPI glClear(GLbitfield mask)
{
    shim_init();
    GL11(PFN_ClearGL, glClear)(mask);
    if (g_debug) { InterlockedIncrement(&g_cnt[CNT_Clear]); if (g_cap_active) { char d[48]; wsprintfA(d, "Clear mask=%x", mask); cap_event(d, 1); } }
}

#define WRAP(name, cnt, sig, call, ...) \
    typedef void (WINAPI *PFN_w_##name) sig; static PFN_w_##name p_w_##name; \
    static void WINAPI w_##name sig { shim_sync_before_gpu(); DrawSave s; draw_begin(&s, cnt); if (s.skip) return; p_w_##name call; if (cnt == CNT_DispatchCompute || cnt == CNT_DispatchComputeIndirect) tbr_mark_all(); DESC(__VA_ARGS__); draw_end(&s, desc); }

WRAP(glDrawRangeElements, CNT_DrawRangeElements, (GLenum m, GLuint a, GLuint b, GLsizei c, GLenum t, const void *i), (m, a, b, c, t, i), "DrawRangeElements mode=%x count=%d type=%x off=%u", m, c, t, (unsigned)(ULONG_PTR)i)
WRAP(glDrawElementsBaseVertex, CNT_DrawElementsBaseVertex, (GLenum m, GLsizei c, GLenum t, const void *i, GLint bv), (m, c, t, i, bv), "DrawElementsBaseVertex mode=%x count=%d type=%x off=%u bv=%d", m, c, t, (unsigned)(ULONG_PTR)i, bv)
WRAP(glDrawRangeElementsBaseVertex, CNT_DrawRangeElementsBaseVertex, (GLenum m, GLuint a, GLuint b, GLsizei c, GLenum t, const void *i, GLint bv), (m, a, b, c, t, i, bv), "DrawRangeElementsBaseVertex mode=%x count=%d bv=%d", m, c, bv)
WRAP(glDrawArraysInstanced, CNT_DrawArraysInstanced, (GLenum m, GLint f, GLsizei c, GLsizei n), (m, f, c, n), "DrawArraysInstanced mode=%x first=%d count=%d inst=%d", m, f, c, n)
WRAP(glDrawElementsInstanced, CNT_DrawElementsInstanced, (GLenum m, GLsizei c, GLenum t, const void *i, GLsizei n), (m, c, t, i, n), "DrawElementsInstanced mode=%x count=%d type=%x inst=%d", m, c, t, n)
WRAP(glDrawElementsInstancedBaseVertex, CNT_DrawElementsInstancedBaseVertex, (GLenum m, GLsizei c, GLenum t, const void *i, GLsizei n, GLint bv), (m, c, t, i, n, bv), "DrawElementsInstancedBaseVertex mode=%x count=%d inst=%d bv=%d", m, c, n, bv)
WRAP(glDrawArraysInstancedBaseInstance, CNT_DrawArraysInstancedBaseInstance, (GLenum m, GLint f, GLsizei c, GLsizei n, GLuint bi), (m, f, c, n, bi), "DrawArraysInstancedBaseInstance mode=%x first=%d count=%d inst=%d bi=%u", m, f, c, n, bi)
WRAP(glDrawElementsInstancedBaseInstance, CNT_DrawElementsInstancedBaseInstance, (GLenum m, GLsizei c, GLenum t, const void *i, GLsizei n, GLuint bi), (m, c, t, i, n, bi), "DrawElementsInstancedBaseInstance mode=%x count=%d inst=%d bi=%u", m, c, n, bi)
WRAP(glDrawElementsInstancedBaseVertexBaseInstance, CNT_DrawElementsInstancedBaseVertexBaseInstance, (GLenum m, GLsizei c, GLenum t, const void *i, GLsizei n, GLint bv, GLuint bi), (m, c, t, i, n, bv, bi), "DrawElementsInstancedBaseVertexBaseInstance mode=%x count=%d inst=%d bv=%d bi=%u", m, c, n, bv, bi)
WRAP(glMultiDrawArrays, CNT_MultiDrawArrays, (GLenum m, const GLint *f, const GLsizei *c, GLsizei n), (m, f, c, n), "MultiDrawArrays mode=%x drawcount=%d c0=%d", m, n, (n > 0 && c) ? c[0] : -1)
WRAP(glMultiDrawElements, CNT_MultiDrawElements, (GLenum m, const GLsizei *c, GLenum t, const void *const *i, GLsizei n), (m, c, t, i, n), "MultiDrawElements mode=%x drawcount=%d c0=%d", m, n, (n > 0 && c) ? c[0] : -1)
WRAP(glMultiDrawElementsBaseVertex, CNT_MultiDrawElementsBaseVertex, (GLenum m, const GLsizei *c, GLenum t, const void *const *i, GLsizei n, const GLint *bv), (m, c, t, i, n, bv), "MultiDrawElementsBaseVertex mode=%x drawcount=%d c0=%d", m, n, (n > 0 && c) ? c[0] : -1)
WRAP(glDrawArraysIndirect, CNT_DrawArraysIndirect, (GLenum m, const void *i), (m, i), "DrawArraysIndirect mode=%x off=%u", m, (unsigned)(ULONG_PTR)i)
WRAP(glDrawElementsIndirect, CNT_DrawElementsIndirect, (GLenum m, GLenum t, const void *i), (m, t, i), "DrawElementsIndirect mode=%x type=%x off=%u", m, t, (unsigned)(ULONG_PTR)i)
WRAP(glDispatchCompute, CNT_DispatchCompute, (GLuint x, GLuint y, GLuint z), (x, y, z), "DispatchCompute %u,%u,%u", x, y, z)
WRAP(glDispatchComputeIndirect, CNT_DispatchComputeIndirect, (GLintptr o), (o), "DispatchComputeIndirect off=%u", (unsigned)o)
WRAP(glDrawTransformFeedback, CNT_DrawTransformFeedback, (GLenum m, GLuint id), (m, id), "DrawTransformFeedback mode=%x id=%u", m, id)

// the multi-draw-indirect splitters count and describe themselves in debug mode
static void WINAPI w_glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    DrawSave s; draw_begin(&s, CNT_MultiDrawElementsIndirect); if (s.skip) return;
    h_glMultiDrawElementsIndirect(mode, type, indirect, drawcount, stride);
    DESC("MultiDrawElementsIndirect mode=%x type=%x off=%u drawcount=%d stride=%d", mode, type, (unsigned)(ULONG_PTR)indirect, drawcount, stride); draw_end(&s, desc);
}
static void WINAPI w_glMultiDrawArraysIndirect(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    DrawSave s; draw_begin(&s, CNT_MultiDrawArraysIndirect); if (s.skip) return;
    h_glMultiDrawArraysIndirect(mode, indirect, drawcount, stride);
    DESC("MultiDrawArraysIndirect mode=%x off=%u drawcount=%d stride=%d", mode, (unsigned)(ULONG_PTR)indirect, drawcount, stride); draw_end(&s, desc);
}

typedef void (WINAPI *PFN_w_Blit)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
static PFN_w_Blit p_w_glBlitFramebuffer;
static void WINAPI w_glBlitFramebuffer(GLint a, GLint b, GLint c, GLint d, GLint e, GLint f, GLint g, GLint h, GLbitfield mask, GLenum filter)
{
    shim_sync_before_gpu();
    InterlockedIncrement(&g_cnt[CNT_BlitFramebuffer]);
    cap_poll_controls();
    if (g_tog.noblit) return;
    h_glBlitFramebuffer(a, b, c, d, e, f, g, h, mask, filter);   // includes the MSAA depth-texture workaround
    if (g_cap_active) {
        char dd[200];
        wsprintfA(dd, "Blit src=%d,%d,%d,%d dst=%d,%d,%d,%d mask=%x filt=%x readfbo=%d", a, b, c, d, e, f, g, h, mask, filter, get_int(GL_READ_FRAMEBUFFER_BINDING));
        cap_event(dd, 1);
    }
}

typedef void (WINAPI *PFN_w_ClearBufferfv)(GLenum, GLint, const float *);
typedef void (WINAPI *PFN_w_ClearBufferiv)(GLenum, GLint, const GLint *);
typedef void (WINAPI *PFN_w_ClearBufferfi)(GLenum, GLint, float, GLint);
static PFN_w_ClearBufferfv p_w_glClearBufferfv;
static PFN_w_ClearBufferiv p_w_glClearBufferiv, p_w_glClearBufferuiv;
static PFN_w_ClearBufferfi p_w_glClearBufferfi;
static void WINAPI w_glClearBufferfv(GLenum b, GLint i, const float *v)
{
    p_w_glClearBufferfv(b, i, v);
    InterlockedIncrement(&g_cnt[CNT_ClearBuffer]);
    if (g_cap_active) { char d[120]; wsprintfA(d, "ClearBufferfv buf=%x idx=%d v=%d,%d,%d,%d(x1000)", b, i, (int)(v[0] * 1000), b == 0x1800 ? (int)(v[1] * 1000) : 0, b == 0x1800 ? (int)(v[2] * 1000) : 0, b == 0x1800 ? (int)(v[3] * 1000) : 0); cap_event(d, 1); }
}
static void WINAPI w_glClearBufferiv(GLenum b, GLint i, const GLint *v) { p_w_glClearBufferiv(b, i, v); InterlockedIncrement(&g_cnt[CNT_ClearBuffer]); if (g_cap_active) { char d[64]; wsprintfA(d, "ClearBufferiv buf=%x idx=%d", b, i); cap_event(d, 0); } }
static void WINAPI w_glClearBufferuiv(GLenum b, GLint i, const GLint *v) { p_w_glClearBufferuiv(b, i, v); InterlockedIncrement(&g_cnt[CNT_ClearBuffer]); if (g_cap_active) { char d[64]; wsprintfA(d, "ClearBufferuiv buf=%x idx=%d", b, i); cap_event(d, 0); } }
static void WINAPI w_glClearBufferfi(GLenum b, GLint i, float dpt, GLint st) { p_w_glClearBufferfi(b, i, dpt, st); InterlockedIncrement(&g_cnt[CNT_ClearBuffer]); if (g_cap_active) { char d[80]; wsprintfA(d, "ClearBufferfi depth=%d(x1000) stencil=%d", (int)(dpt * 1000), st); cap_event(d, 0); } }

typedef void (WINAPI *PFN_w_BindFramebuffer)(GLenum, GLuint);
static PFN_w_BindFramebuffer p_w_glBindFramebuffer;
static void WINAPI w_glBindFramebuffer(GLenum target, GLuint fb)
{
    p_w_glBindFramebuffer(target, fb);
    InterlockedIncrement(&g_cnt[CNT_BindFramebuffer]);
    if (g_cap_active) { char d[64]; wsprintfA(d, "BindFramebuffer target=%x fbo=%u", target, fb); cap_event(d, 0); }
}
typedef void (WINAPI *PFN_w_UseProgram)(GLuint);
static PFN_w_UseProgram p_w_glUseProgram, p_w_glBindProgramPipeline;
static void WINAPI w_glUseProgram(GLuint p) { p_w_glUseProgram(p); InterlockedIncrement(&g_cnt[CNT_UseProgram]); }
static void WINAPI w_glBindProgramPipeline(GLuint p) { p_w_glBindProgramPipeline(p); InterlockedIncrement(&g_cnt[CNT_BindProgramPipeline]); }
typedef void (WINAPI *PFN_w_ProvokingVertex)(GLenum);
static PFN_w_ProvokingVertex p_w_glProvokingVertex;
static void WINAPI w_glProvokingVertex(GLenum m)
{
    p_w_glProvokingVertex(m);
    if (InterlockedIncrement(&g_cnt[CNT_ProvokingVertex]) <= 5) log_fmt("glProvokingVertex(0x%x) called", m);
}

typedef void (WINAPI *PFN_w_BeginCond)(GLuint, GLenum);
typedef void (WINAPI *PFN_w_EndCond)(void);
static PFN_w_BeginCond p_w_glBeginConditionalRender, p_w_glBeginConditionalRenderNV;
static PFN_w_EndCond p_w_glEndConditionalRender, p_w_glEndConditionalRenderNV;
static volatile LONG g_cnt_cond;
// Conditional rendering is bypassed while a frame is captured (the capture's
// readbacks make Parallels drop the conditional draws) and with the toggle
// "nocond" (diagnostic: everything is drawn regardless of occlusion queries).
static volatile LONG g_cond_bypassed;
static void WINAPI w_glBeginConditionalRender(GLuint id, GLenum mode)
{
    if (InterlockedIncrement(&g_cnt_cond) <= 3) log_fmt("glBeginConditionalRender(%u, 0x%x) used", id, mode);
    if (g_cap_active) { char d[80]; wsprintfA(d, "BeginConditionalRender q=%u mode=%x (bypassed)", id, mode); cap_event(d, 0); }
    if (g_cap_active || g_tog.nocond) { InterlockedIncrement(&g_cond_bypassed); return; }
    InterlockedIncrement(&g_in_cond_render); p_w_glBeginConditionalRender(id, mode);
}
static void WINAPI w_glBeginConditionalRenderNV(GLuint id, GLenum mode)
{
    if (InterlockedIncrement(&g_cnt_cond) <= 3) log_fmt("glBeginConditionalRenderNV(%u, 0x%x) used", id, mode);
    if (g_cap_active) { char d[80]; wsprintfA(d, "BeginConditionalRenderNV q=%u mode=%x (bypassed)", id, mode); cap_event(d, 0); }
    if (g_cap_active || g_tog.nocond) { InterlockedIncrement(&g_cond_bypassed); return; }
    InterlockedIncrement(&g_in_cond_render); p_w_glBeginConditionalRenderNV(id, mode);
}
static void WINAPI w_glEndConditionalRender(void)
{
    if (g_cond_bypassed > 0) { InterlockedDecrement(&g_cond_bypassed); if (g_cap_active) cap_event("EndConditionalRender (bypassed)", 1); return; }
    p_w_glEndConditionalRender(); InterlockedDecrement(&g_in_cond_render); if (g_cap_active) cap_event("EndConditionalRender", 1);
}
static void WINAPI w_glEndConditionalRenderNV(void)
{
    if (g_cond_bypassed > 0) { InterlockedDecrement(&g_cond_bypassed); if (g_cap_active) cap_event("EndConditionalRenderNV (bypassed)", 1); return; }
    p_w_glEndConditionalRenderNV(); InterlockedDecrement(&g_in_cond_render); if (g_cap_active) cap_event("EndConditionalRenderNV", 1);
}

typedef void (WINAPI *PFN_w_ShaderSource)(GLuint, GLsizei, const char *const *, const GLint *);
static PFN_w_ShaderSource p_w_glShaderSource;
static void WINAPI w_glShaderSource(GLuint sh, GLsizei count, const char *const *strings, const GLint *lengths)
{
    p_w_glShaderSource(sh, count, strings, lengths);
    record_source(&g_src_by_shader, sh, count, strings, lengths);
}
typedef void (WINAPI *PFN_w_AttachShader)(GLuint, GLuint);
static PFN_w_AttachShader p_w_glAttachShader;
static void WINAPI w_glAttachShader(GLuint prog, GLuint sh)
{
    p_w_glAttachShader(prog, sh);
    if (prog < SRC_SLOTS) {
        if (!g_attached) g_attached = (GLuint (*)[8])VirtualAlloc(NULL, SRC_SLOTS * sizeof(GLuint[8]), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (g_attached) for (int i = 0; i < 8; i++) if (!g_attached[prog][i] || g_attached[prog][i] == sh) { g_attached[prog][i] = sh; break; }
    }
}
typedef GLuint (WINAPI *PFN_w_CreateShaderProgramv)(GLenum, GLsizei, const char *const *);
static PFN_w_CreateShaderProgramv p_w_glCreateShaderProgramv;
static GLuint WINAPI w_glCreateShaderProgramv(GLenum type, GLsizei count, const char *const *strings)
{
    GLuint p = p_w_glCreateShaderProgramv(type, count, strings);
    record_source(&g_src_by_program, p, count, strings, NULL);
    return p;
}

#define CAPW(n) { #n, (void **)&p_w_##n, (PROC)w_##n, 1 }
#define CAP_WRAPS \
    CAPW(glDrawRangeElements), CAPW(glDrawElementsBaseVertex), CAPW(glDrawRangeElementsBaseVertex), CAPW(glDrawArraysInstanced), \
    CAPW(glDrawElementsInstanced), CAPW(glDrawElementsInstancedBaseVertex), CAPW(glDrawArraysInstancedBaseInstance), \
    CAPW(glDrawElementsInstancedBaseInstance), CAPW(glDrawElementsInstancedBaseVertexBaseInstance), CAPW(glMultiDrawArrays), \
    CAPW(glMultiDrawElements), CAPW(glMultiDrawElementsBaseVertex), CAPW(glDrawArraysIndirect), CAPW(glDrawElementsIndirect), \
    CAPW(glDispatchCompute), CAPW(glDispatchComputeIndirect), CAPW(glDrawTransformFeedback), CAPW(glBlitFramebuffer), \
    CAPW(glClearBufferfv), CAPW(glClearBufferiv), CAPW(glClearBufferuiv), CAPW(glClearBufferfi), CAPW(glBindFramebuffer), \
    CAPW(glUseProgram), CAPW(glBindProgramPipeline), CAPW(glProvokingVertex), CAPW(glShaderSource), CAPW(glAttachShader), \
    CAPW(glCreateShaderProgramv), CAPW(glBeginConditionalRender), CAPW(glBeginConditionalRenderNV), \
    CAPW(glEndConditionalRender), CAPW(glEndConditionalRenderNV), \
    { "glMultiDrawElementsIndirect", (void **)&p_glMultiDrawElementsIndirect, (PROC)w_glMultiDrawElementsIndirect, 1 }, \
    { "glMultiDrawArraysIndirect",   (void **)&p_glMultiDrawArraysIndirect,   (PROC)w_glMultiDrawArraysIndirect, 1 }

// GL adapter: an x64 opengl32.dll proxy that lets Dassault Systemes 3DEXPERIENCE (R2024x)
// render on the OpenGL 4.3 driver of Parallels Desktop (Windows 11 on ARM, Apple silicon).
// Not affiliated with Dassault Systemes or Parallels.
//
// Every call is forwarded to the real System32 opengl32.dll / the Parallels driver, except:
//   1. GL_VENDOR is reported as "INTEL" (3DEXPERIENCE crashes on unknown vendors;
//      SHIM_GL_VENDOR overrides it).
//   2. EXT_direct_state_access entry points the driver lacks are emulated.
//   3. ARB_buffer_storage incl. persistent/coherent mapping is emulated: persistent buffers
//      get a CPU shadow copy in a write-watched arena; dirty pages are uploaded before GPU
//      work, read-back buffers are refreshed at sync points.
//   4. Never-bound buffer names are bound once before use (the driver rejects them).
//   5. glMultiDraw{Elements,Arrays}Indirect are split into single indirect draws
//      (the driver only executes the first command).
//   6. glTexBufferRange is emulated (the driver ignores offset/size).
//   7. Depth blits out of multisample depth TEXTURES are done with a shader (the driver
//      raises GL_INVALID_OPERATION and copies nothing).
//   8. 3DEXPERIENCE's GPU occlusion culling reads its result from a small persistent
//      read-back buffer that is never written on this driver; such reads report
//      "all visible" (SHIM_OCC_READBACK=real disables this).
//   9. The "No certified OpenGL library has been found" message box is confirmed with OK.
// Diagnostics with SHIM_DEBUG=1: %LOCALAPPDATA%\GLAdapter\shim.log and frame captures
// (see capture.c). SHIM_GL_VERSION=4.1 and SHIM_HIDE_EXT are experiments; hiding
// GL_ARB_buffer_storage makes 3DEXPERIENCE crash.
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include "names.h"

// Marker the installer uses to recognise this DLL (kept by the linker).
#define GLADAPTER_VERSION "0.2.0"
__attribute__((used)) static const char g_gladapter_marker[] = "GLADAPTER-3DX-MARKER version " GLADAPTER_VERSION;

typedef unsigned int GLenum, GLuint, GLbitfield;
typedef int GLint, GLsizei;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
typedef ptrdiff_t GLintptr, GLsizeiptr;
typedef unsigned long long GLuint64;
typedef void *GLsync;

void *g_tbl[SHIM_COUNT];
void *g_ph_real[PH_COUNT];
// generic call trace (debug captures with the ctl.txt toggle "calls")
void *g_tr_real[TR_COUNT];
unsigned char g_tr_on;
static const char *g_tr_names[TR_COUNT];
static volatile LONG g_tr_n;
static volatile LONG g_tr_lines;
void shim_trace_call(unsigned k, const ULONG_PTR *a);
static void trace_dump_ring(const char *why, int follow);
void shim_trace_export(unsigned k, const ULONG_PTR *a);
static INIT_ONCE g_once = INIT_ONCE_STATIC_INIT;
static int g_debug;   // SHIM_DEBUG=1: shader checks, request log, call counters, capture
static int g_gldebug; // SHIM_GLDEBUG=1: additionally enable the driver's KHR_debug output
static int g_spoof_major, g_spoof_minor;   // SHIM_GL_VERSION=4.1: report an older GL (selects 3DEXPERIENCE's OGL_40 path)

// ---------------------------------------------------------------------------
// init + logging
// ---------------------------------------------------------------------------
static BOOL CALLBACK init_once(PINIT_ONCE once, PVOID param, PVOID *ctx)
{
    (void)once; (void)param; (void)ctx;
    WCHAR path[MAX_PATH];
    UINT len = GetSystemDirectoryW(path, MAX_PATH - 16);
    if (len == 0)
        return FALSE;
    lstrcatW(path, L"\\opengl32.dll");
    HMODULE real = LoadLibraryW(path);
    if (!real)
        return FALSE;
    for (int i = 0; i < SHIM_COUNT; i++)
        g_tbl[i] = (void *)GetProcAddress(real, g_names[i]);
    return TRUE;
}

void shim_init(void)
{
    InitOnceExecuteOnce(&g_once, init_once, NULL, NULL);
}

static CRITICAL_SECTION g_log_cs;
static void log_line(const char *a, const char *b)
{
    char path[MAX_PATH];
    DWORD n = GetEnvironmentVariableA("LOCALAPPDATA", path, MAX_PATH - 40);
    if (n == 0 || n >= MAX_PATH - 40)
        return;
    lstrcatA(path, "\\GLAdapter");
    if (g_debug)
        CreateDirectoryA(path, NULL);   // the log folder only appears when SHIM_DEBUG=1
    lstrcatA(path, "\\shim.log");
    EnterCriticalSection(&g_log_cs);
    HANDLE h = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD w;
        WriteFile(h, a, lstrlenA(a), &w, NULL);
        if (b)
            WriteFile(h, b, lstrlenA(b), &w, NULL);
        WriteFile(h, "\r\n", 2, &w, NULL);
        CloseHandle(h);
    }
    LeaveCriticalSection(&g_log_cs);
}

static void log_fmt(const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    wvsprintfA(buf, fmt, ap);
    va_end(ap);
    log_line(buf, NULL);
}

typedef PROC (WINAPI *PFN_wglGetProcAddress)(LPCSTR);
static PROC real_proc(const char *name)
{
    return ((PFN_wglGetProcAddress)g_tbl[IDX_wglGetProcAddress])(name);
}

// Resolves a real driver entry point once; returns from the caller if missing.
#define REAL(type, var, name)                                   \
    static type var;                                            \
    if (!var) var = (type)real_proc(name);                      \
    if (!var) { log_line("CORE MISSING: ", name); return; }
#define REAL_RET(type, var, name, ret)                          \
    static type var;                                            \
    if (!var) var = (type)real_proc(name);                      \
    if (!var) { log_line("CORE MISSING: ", name); return ret; }

typedef void (WINAPI *PFN_glGetIntegerv)(GLenum, GLint *);
typedef const GLubyte *(WINAPI *PFN_glGetStringi)(GLenum, GLuint);
static GLint get_int(GLenum pname)
{
    GLint v = 0;
    ((PFN_glGetIntegerv)g_tbl[IDX_glGetIntegerv])(pname, &v);
    return v;
}

typedef void (WINAPI *PFN_glBindTexture)(GLenum, GLuint);
static void bind_tex(GLenum target, GLuint tex)
{
    ((PFN_glBindTexture)g_tbl[IDX_glBindTexture])(target, tex);
}

// ---------------------------------------------------------------------------
// 1. vendor + extension list
// ---------------------------------------------------------------------------
#define GL_VENDOR          0x1F00
#define GL_VERSION         0x1F02
#define GL_EXTENSIONS      0x1F03
#define GL_NUM_EXTENSIONS  0x821D

#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_MAJOR_VERSION   0x821B
#define GL_MINOR_VERSION   0x821C

static const char *const g_extra_exts[] = { "GL_ARB_buffer_storage" };
#define EXTRA_EXT_COUNT ((GLint)(sizeof g_extra_exts / sizeof g_extra_exts[0]))

// Extensions that became core after GL 4.1; hidden when an older version is
// reported so the application sees a consistent 4.0/4.1 driver.
static const char *const g_modern_exts[] = {
    "GL_ARB_texture_compression_bptc", "GL_ARB_compressed_texture_pixel_storage", "GL_ARB_shader_atomic_counters",
    "GL_ARB_texture_storage", "GL_ARB_transform_feedback_instanced", "GL_ARB_base_instance",
    "GL_ARB_shader_image_load_store", "GL_ARB_conservative_depth", "GL_ARB_shading_language_420pack",
    "GL_ARB_internalformat_query", "GL_ARB_map_buffer_alignment", "GL_ARB_shading_language_packing",
    "GL_ARB_arrays_of_arrays", "GL_ARB_ES3_compatibility", "GL_ARB_clear_buffer_object", "GL_ARB_compute_shader",
    "GL_ARB_copy_image", "GL_ARB_explicit_uniform_location", "GL_ARB_fragment_layer_viewport",
    "GL_ARB_framebuffer_no_attachments", "GL_ARB_internalformat_query2", "GL_ARB_invalidate_subdata",
    "GL_ARB_multi_draw_indirect", "GL_ARB_program_interface_query", "GL_ARB_robust_buffer_access_behavior",
    "GL_ARB_shader_image_size", "GL_ARB_shader_storage_buffer_object", "GL_ARB_stencil_texturing",
    "GL_ARB_texture_buffer_range", "GL_ARB_texture_query_levels", "GL_ARB_texture_storage_multisample",
    "GL_ARB_texture_view", "GL_ARB_vertex_attrib_binding", "GL_ARB_buffer_storage", "GL_ARB_indirect_parameters",
    "GL_ARB_shader_draw_parameters", "GL_ARB_compute_variable_group_size", "GL_ARB_bindless_texture",
    "GL_ARB_enhanced_layouts", "GL_ARB_multi_bind", "GL_ARB_query_buffer_object", "GL_ARB_clear_texture",
    "GL_ARB_direct_state_access", "GL_ARB_clip_control", "GL_ARB_cull_distance", "GL_ARB_derivative_control",
    "GL_ARB_shader_texture_image_samples", "GL_ARB_texture_barrier", "GL_ARB_get_texture_sub_image",
    "GL_ARB_conditional_render_inverted", "GL_ARB_ES3_1_compatibility", "GL_ARB_ES3_2_compatibility",
    "GL_ARB_gl_spirv", "GL_ARB_spirv_extensions", "GL_ARB_pipeline_statistics_query",
    "GL_ARB_transform_feedback_overflow_query", "GL_ARB_shader_atomic_counter_ops", "GL_ARB_shader_group_vote",
    "GL_ARB_shader_ballot", "GL_ARB_sparse_buffer", "GL_ARB_sparse_texture", "GL_ARB_sparse_texture2",
    "GL_ARB_shader_clock", "GL_ARB_fragment_shader_interlock", "GL_ARB_post_depth_coverage",
    "GL_ARB_sample_locations", "GL_ARB_texture_filter_minmax", "GL_ARB_shader_viewport_layer_array",
    "GL_ARB_parallel_shader_compile", "GL_ARB_polygon_offset_clamp", "GL_ARB_texture_filter_anisotropic",
    "GL_EXT_shader_image_load_store", "GL_EXT_shader_image_load_formatted", "GL_NV_shader_buffer_load",
};

// the effective extension list: real ones minus hidden ones, plus extras
static char **g_ext_list;
static GLint g_ext_count = -1;
static char *g_ext_legacy;

// SHIM_HIDE_EXT: space/comma separated extension names to hide; the word
// "modern" stands for the whole post-4.1 table above.
static int ext_hidden(const char *name)
{
    static char extra[1024];
    static int loaded, hide_modern;
    if (!loaded) {
        loaded = 1;
        DWORD n = GetEnvironmentVariableA("SHIM_HIDE_EXT", extra, sizeof extra);
        if (n >= sizeof extra) extra[0] = 0;
        hide_modern = strstr(extra, "modern") != NULL;
    }
    if (hide_modern) {
        for (size_t i = 0; i < sizeof g_modern_exts / sizeof g_modern_exts[0]; i++)
            if (strcmp(name, g_modern_exts[i]) == 0)
                return 1;
    }
    size_t len = strlen(name);
    for (const char *p = extra; *p; ) {
        while (*p == ' ' || *p == ',') p++;
        const char *e = p;
        while (*e && *e != ' ' && *e != ',') e++;
        if ((size_t)(e - p) == len && memcmp(p, name, len) == 0)
            return 1;
        p = e;
    }
    return 0;
}

// needs a current context; builds the list once
static void ext_build(void)
{
    if (g_ext_count >= 0)
        return;
    EnterCriticalSection(&g_log_cs);
    if (g_ext_count < 0) {
        GLint n = 0;
        ((PFN_glGetIntegerv)g_tbl[IDX_glGetIntegerv])(GL_NUM_EXTENSIONS, &n);
        PFN_glGetStringi gsi = (PFN_glGetStringi)real_proc("glGetStringi");
        char **list = (char **)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (size_t)(n + EXTRA_EXT_COUNT + 1) * sizeof(char *));
        GLint k = 0, hidden = 0;
        size_t total = 0;
        if (list && gsi) {
            for (GLint i = 0; i < n; i++) {
                const char *e = (const char *)gsi(GL_EXTENSIONS, (GLuint)i);
                if (!e) continue;
                if (ext_hidden(e)) { hidden++; continue; }
                size_t l = strlen(e);
                list[k] = (char *)HeapAlloc(GetProcessHeap(), 0, l + 1);
                if (!list[k]) continue;
                memcpy(list[k], e, l + 1);
                total += l + 1;
                k++;
            }
        }
        if (list)
            for (GLint i = 0; i < EXTRA_EXT_COUNT; i++) {
                if (ext_hidden(g_extra_exts[i])) continue;
                int dup = 0;
                for (GLint j = 0; j < k; j++) if (strcmp(list[j], g_extra_exts[i]) == 0) dup = 1;
                if (!dup) { list[k++] = (char *)g_extra_exts[i]; total += strlen(g_extra_exts[i]) + 1; }
            }
        char *legacy = (char *)HeapAlloc(GetProcessHeap(), 0, total + 2);
        if (legacy) {
            legacy[0] = 0;
            for (GLint j = 0; j < k; j++) { if (j) lstrcatA(legacy, " "); lstrcatA(legacy, list[j]); }
        }
        g_ext_legacy = legacy;
        g_ext_list = list;
        log_fmt("EXTENSIONS: real=%d hidden=%d reported=%d spoof=%d.%d", (int)n, (int)hidden, (int)k, g_spoof_major, g_spoof_minor);
        g_ext_count = k;
    }
    LeaveCriticalSection(&g_log_cs);
}

static const char *vendor_override(void)
{
    static char buf[64];
    DWORD n = GetEnvironmentVariableA("SHIM_GL_VENDOR", buf, sizeof buf);
    return (n > 0 && n < sizeof buf) ? buf : "INTEL";
}

// "4.3 - Build 27.1.0" -> "4.1 - Build 27.1.0"; GLSL "4.30 ..." -> "4.10 ..."
static const char *spoof_version_string(const char *real, int glsl)
{
    static char buf[2][128];
    char *out = buf[glsl ? 1 : 0];
    if (!g_spoof_major || !real)
        return real;
    if (!out[0]) {
        const char *rest = real;
        while (*rest && *rest != ' ') rest++;
        if (glsl)
            wsprintfA(out, "%d.%d0%s", g_spoof_major, g_spoof_minor, rest);
        else
            wsprintfA(out, "%d.%d%s", g_spoof_major, g_spoof_minor, rest);
    }
    return out;
}

typedef const GLubyte *(WINAPI *PFN_glGetString)(GLenum);
const GLubyte *WINAPI glGetString(GLenum name)
{
    shim_init();
    if (name == GL_VENDOR)
        return (const GLubyte *)vendor_override();
    const GLubyte *s = ((PFN_glGetString)g_tbl[IDX_glGetString])(name);
    if (name == GL_EXTENSIONS && s) {
        ext_build();
        return g_ext_legacy ? (const GLubyte *)g_ext_legacy : s;
    }
    if (name == GL_VERSION) {
        static LONG once;
        if (!InterlockedExchange(&once, 1))
            log_line("GL_VERSION: ", s ? (const char *)s : "<null>");
        return (const GLubyte *)spoof_version_string((const char *)s, 0);
    }
    if (name == GL_SHADING_LANGUAGE_VERSION)
        return (const GLubyte *)spoof_version_string((const char *)s, 1);
    return s;
}

void WINAPI glGetIntegerv(GLenum pname, GLint *data)
{
    shim_init();
    if (pname == GL_NUM_EXTENSIONS && data) {
        ext_build();
        *data = g_ext_count >= 0 ? g_ext_count : 0;
        return;
    }
    if (g_spoof_major && data && (pname == GL_MAJOR_VERSION || pname == GL_MINOR_VERSION)) {
        *data = pname == GL_MAJOR_VERSION ? g_spoof_major : g_spoof_minor;
        return;
    }
    ((PFN_glGetIntegerv)g_tbl[IDX_glGetIntegerv])(pname, data);
}

static PFN_glGetStringi p_glGetStringi;
static const GLubyte *WINAPI h_glGetStringi(GLenum name, GLuint index)
{
    if (name == GL_EXTENSIONS) {
        ext_build();
        if (g_ext_count >= 0)
            return (GLint)index < g_ext_count ? (const GLubyte *)g_ext_list[index] : NULL;
    }
    return p_glGetStringi(name, index);
}

// ---------------------------------------------------------------------------
// 2. EXT_direct_state_access entry points Parallels lacks
// ---------------------------------------------------------------------------
#define GL_COPY_READ_BUFFER           0x8F36
#define GL_COPY_WRITE_BUFFER          0x8F37
#define GL_DRAW_FRAMEBUFFER           0x8CA9
#define GL_DRAW_FRAMEBUFFER_BINDING   0x8CA6
#define GL_VERTEX_ARRAY_BINDING       0x85B5
#define GL_TEXTURE_BUFFER             0x8C2A
#define GL_TEXTURE_BINDING_BUFFER     0x8C2C
#define GL_TEXTURE_2D_MULTISAMPLE           0x9100
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY     0x9102
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE        0x9104
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY  0x9105

typedef void (WINAPI *PFN_BindBuffer)(GLenum, GLuint);
typedef void (WINAPI *PFN_BindFramebuffer)(GLenum, GLuint);
typedef void (WINAPI *PFN_BindVertexArray)(GLuint);

static GLenum tex_binding_for(GLenum target)
{
    switch (target) {
    case GL_TEXTURE_BUFFER: return GL_TEXTURE_BINDING_BUFFER;
    case GL_TEXTURE_2D_MULTISAMPLE: return GL_TEXTURE_BINDING_2D_MULTISAMPLE;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: return GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY;
    default: return 0;
    }
}

static void WINAPI e_ClearNamedBufferDataEXT(GLuint buf, GLenum ifmt, GLenum fmt, GLenum type, const void *data)
{
    typedef void (WINAPI *F)(GLenum, GLenum, GLenum, GLenum, const void *);
    REAL(F, f, "glClearBufferData");
    REAL(PFN_BindBuffer, bind, "glBindBuffer");
    GLint old = get_int(GL_COPY_WRITE_BUFFER);
    bind(GL_COPY_WRITE_BUFFER, buf);
    f(GL_COPY_WRITE_BUFFER, ifmt, fmt, type, data);
    bind(GL_COPY_WRITE_BUFFER, (GLuint)old);
}

static void WINAPI e_ClearNamedBufferSubDataEXT(GLuint buf, GLenum ifmt, GLsizeiptr off, GLsizeiptr size,
                                                GLenum fmt, GLenum type, const void *data)
{
    typedef void (WINAPI *F)(GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void *);
    REAL(F, f, "glClearBufferSubData");
    REAL(PFN_BindBuffer, bind, "glBindBuffer");
    GLint old = get_int(GL_COPY_WRITE_BUFFER);
    bind(GL_COPY_WRITE_BUFFER, buf);
    f(GL_COPY_WRITE_BUFFER, ifmt, off, size, fmt, type, data);
    bind(GL_COPY_WRITE_BUFFER, (GLuint)old);
}

static void WINAPI e_GetNamedFramebufferParameterivEXT(GLuint fb, GLenum pname, GLint *params)
{
    typedef void (WINAPI *F)(GLenum, GLenum, GLint *);
    REAL(F, f, "glGetFramebufferParameteriv");
    REAL(PFN_BindFramebuffer, bind, "glBindFramebuffer");
    GLint old = get_int(GL_DRAW_FRAMEBUFFER_BINDING);
    bind(GL_DRAW_FRAMEBUFFER, fb);
    f(GL_DRAW_FRAMEBUFFER, pname, params);
    bind(GL_DRAW_FRAMEBUFFER, (GLuint)old);
}

static void WINAPI e_NamedFramebufferParameteriEXT(GLuint fb, GLenum pname, GLint param)
{
    typedef void (WINAPI *F)(GLenum, GLenum, GLint);
    REAL(F, f, "glFramebufferParameteri");
    REAL(PFN_BindFramebuffer, bind, "glBindFramebuffer");
    GLint old = get_int(GL_DRAW_FRAMEBUFFER_BINDING);
    bind(GL_DRAW_FRAMEBUFFER, fb);
    f(GL_DRAW_FRAMEBUFFER, pname, param);
    bind(GL_DRAW_FRAMEBUFFER, (GLuint)old);
}

static void ensure_buffer_object(GLuint buf);
static void ms_tex_mark(GLuint t);
static int tbr_set(GLenum target, GLenum fmt, GLuint buf, GLintptr off, GLsizeiptr size);
static void tbr_forget_cur(GLenum target);

static void WINAPI e_TextureBufferRangeEXT(GLuint tex, GLenum target, GLenum ifmt, GLuint buf,
                                           GLintptr off, GLsizeiptr size)
{
    typedef void (WINAPI *F)(GLenum, GLenum, GLuint, GLintptr, GLsizeiptr);
    REAL(F, f, "glTexBufferRange");   // Parallels accepts never-bound names here
    GLenum q = tex_binding_for(target);
    GLint old = q ? get_int(q) : 0;
    bind_tex(target, tex);
    ensure_buffer_object(buf);
    if (!tbr_set(target, ifmt, buf, off, size))
        f(target, ifmt, buf, off, size);
    bind_tex(target, (GLuint)old);
}

static void WINAPI e_TextureStorage2DMultisampleEXT(GLuint tex, GLenum target, GLsizei samples, GLenum ifmt,
                                                    GLsizei w, GLsizei h, GLboolean fixed)
{
    typedef void (WINAPI *F)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
    REAL(F, f, "glTexStorage2DMultisample");
    GLenum q = tex_binding_for(target);
    GLint old = q ? get_int(q) : 0;
    bind_tex(target, tex);
    f(target, samples, ifmt, w, h, fixed);
    ms_tex_mark(tex);
    bind_tex(target, (GLuint)old);
}

static void WINAPI e_TextureStorage3DMultisampleEXT(GLuint tex, GLenum target, GLsizei samples, GLenum ifmt,
                                                    GLsizei w, GLsizei h, GLsizei d, GLboolean fixed)
{
    typedef void (WINAPI *F)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
    REAL(F, f, "glTexStorage3DMultisample");
    GLenum q = tex_binding_for(target);
    GLint old = q ? get_int(q) : 0;
    bind_tex(target, tex);
    f(target, samples, ifmt, w, h, d, fixed);
    bind_tex(target, (GLuint)old);
}

#define WITH_VAO(vao, call)                                     \
    REAL(PFN_BindVertexArray, bindva, "glBindVertexArray");     \
    GLint oldva = get_int(GL_VERTEX_ARRAY_BINDING);             \
    bindva(vao);                                                \
    call;                                                       \
    bindva((GLuint)oldva);

typedef GLenum (WINAPI *PFN_glGetError)(void);
typedef GLboolean (WINAPI *PFN_IsObj)(GLuint);

// Parallels rejects buffer names that were generated but never bound
// (EXT_direct_state_access creates them on first use). Binding once creates
// the object; the previous binding is restored.
// Per-context bitmaps of buffer names already known to exist, so the extra
// bind happens once per buffer and context, not on every call. Buffer names
// are only unique within a share group, hence one bitmap per context.
#define KNOWN_BITS (1u << 22)
#define KNOWN_CTX  32
typedef HGLRC (WINAPI *PFN_wglGetCurrentContext)(void);
static struct { HGLRC rc; volatile LONG *bits; } g_known[KNOWN_CTX];
static volatile LONG g_known_n;

static volatile LONG *known_bits(void)
{
    static PFN_wglGetCurrentContext getcur;
    if (!getcur)
        getcur = (PFN_wglGetCurrentContext)GetProcAddress(LoadLibraryA("opengl32.dll"), "wglGetCurrentContext");
    HGLRC rc = getcur ? getcur() : NULL;
    if (!rc)
        return NULL;
    LONG n = g_known_n;
    for (LONG i = 0; i < n && i < KNOWN_CTX; i++)
        if (g_known[i].rc == rc)
            return g_known[i].bits;
    volatile LONG *bits = (volatile LONG *)VirtualAlloc(NULL, KNOWN_BITS / 8, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!bits)
        return NULL;
    LONG k = InterlockedIncrement(&g_known_n) - 1;
    if (k >= KNOWN_CTX) {
        VirtualFree((void *)bits, 0, MEM_RELEASE);
        return NULL;
    }
    g_known[k].bits = bits;
    g_known[k].rc = rc;
    return bits;
}

static void forget_buffer(GLuint buf)
{
    if (!buf || buf >= KNOWN_BITS)
        return;
    LONG n = g_known_n;
    for (LONG i = 0; i < n && i < KNOWN_CTX; i++)
        if (g_known[i].bits)
            InterlockedAnd(&g_known[i].bits[buf >> 5], ~(LONG)(1u << (buf & 31)));
}

static void ensure_buffer_object(GLuint buf)
{
    if (!buf)
        return;
    volatile LONG *bits = buf < KNOWN_BITS ? known_bits() : NULL;
    if (bits && (bits[buf >> 5] & (LONG)(1u << (buf & 31))))
        return;
    REAL(PFN_BindBuffer, bind, "glBindBuffer");
    GLint old = get_int(GL_COPY_WRITE_BUFFER);
    if ((GLuint)old != buf) {
        bind(GL_COPY_WRITE_BUFFER, buf);
        bind(GL_COPY_WRITE_BUFFER, (GLuint)old);
    }
    if (bits)
        InterlockedOr(&bits[buf >> 5], (LONG)(1u << (buf & 31)));
}

// Core entry points that take a buffer name directly and that Parallels
// rejects for never-bound names.
typedef void (WINAPI *PFN_BindVertexBuffer)(GLuint, GLuint, GLintptr, GLsizei);
typedef void (WINAPI *PFN_TexBuffer)(GLenum, GLenum, GLuint);
typedef void (WINAPI *PFN_TexBufferRange)(GLenum, GLenum, GLuint, GLintptr, GLsizeiptr);
typedef void (WINAPI *PFN_InvalidateBufferData)(GLuint);
typedef void (WINAPI *PFN_InvalidateBufferSubData)(GLuint, GLintptr, GLsizeiptr);
static PFN_BindVertexBuffer p_glBindVertexBuffer;
static PFN_TexBuffer p_glTexBuffer, p_glTexBufferARB, p_glTexBufferEXT;
static PFN_TexBufferRange p_glTexBufferRange;
static PFN_InvalidateBufferData p_glInvalidateBufferData;
static PFN_InvalidateBufferSubData p_glInvalidateBufferSubData;

static void WINAPI h_glBindVertexBuffer(GLuint idx, GLuint buf, GLintptr off, GLsizei stride)
{
    ensure_buffer_object(buf);
    if (g_debug && get_int(GL_VERTEX_ARRAY_BINDING) == 0) {
        // Parallels rejects vertex-buffer bindings on the default VAO 0
        static volatile LONG n;
        if (InterlockedIncrement(&n) <= 10)
            log_fmt("VAO0 glBindVertexBuffer idx=%u buf=%u off=%d stride=%d profile=%d thread=%u",
                    idx, buf, (int)off, (int)stride, (int)get_int(0x9126), (unsigned)GetCurrentThreadId());
    }
    p_glBindVertexBuffer(idx, buf, off, stride);
}
static int tbr_set(GLenum target, GLenum fmt, GLuint buf, GLintptr off, GLsizeiptr size);
static void tbr_forget_cur(GLenum target);
static void WINAPI h_glTexBuffer(GLenum t, GLenum f, GLuint buf)    { ensure_buffer_object(buf); tbr_forget_cur(t); p_glTexBuffer(t, f, buf); }
static void WINAPI h_glTexBufferARB(GLenum t, GLenum f, GLuint buf) { ensure_buffer_object(buf); tbr_forget_cur(t); p_glTexBufferARB(t, f, buf); }
static void WINAPI h_glTexBufferEXT(GLenum t, GLenum f, GLuint buf) { ensure_buffer_object(buf); tbr_forget_cur(t); p_glTexBufferEXT(t, f, buf); }
static void WINAPI h_glTexBufferRange(GLenum t, GLenum f, GLuint buf, GLintptr o, GLsizeiptr s)
{
    ensure_buffer_object(buf);
    if (!tbr_set(t, f, buf, o, s))
        p_glTexBufferRange(t, f, buf, o, s);
}
static void WINAPI h_glInvalidateBufferData(GLuint buf) { ensure_buffer_object(buf); p_glInvalidateBufferData(buf); }
static void WINAPI h_glInvalidateBufferSubData(GLuint buf, GLintptr o, GLsizeiptr l)
{
    ensure_buffer_object(buf);
    p_glInvalidateBufferSubData(buf, o, l);
}

static void WINAPI e_VertexArrayBindVertexBufferEXT(GLuint vao, GLuint idx, GLuint buf, GLintptr off, GLsizei stride)
{
    typedef void (WINAPI *F)(GLuint, GLuint, GLintptr, GLsizei);
    REAL(F, f, "glBindVertexBuffer");
    ensure_buffer_object(buf);
    WITH_VAO(vao, f(idx, buf, off, stride));
}

static void WINAPI e_VertexArrayVertexAttribBindingEXT(GLuint vao, GLuint attrib, GLuint binding)
{
    typedef void (WINAPI *F)(GLuint, GLuint);
    REAL(F, f, "glVertexAttribBinding");
    WITH_VAO(vao, f(attrib, binding));
}

static void WINAPI e_VertexArrayVertexAttribFormatEXT(GLuint vao, GLuint attrib, GLint size, GLenum type,
                                                      GLboolean norm, GLuint reloff)
{
    typedef void (WINAPI *F)(GLuint, GLint, GLenum, GLboolean, GLuint);
    REAL(F, f, "glVertexAttribFormat");
    WITH_VAO(vao, f(attrib, size, type, norm, reloff));
}

static void WINAPI e_VertexArrayVertexAttribIFormatEXT(GLuint vao, GLuint attrib, GLint size, GLenum type, GLuint reloff)
{
    typedef void (WINAPI *F)(GLuint, GLint, GLenum, GLuint);
    REAL(F, f, "glVertexAttribIFormat");
    WITH_VAO(vao, f(attrib, size, type, reloff));
}

static void WINAPI e_VertexArrayVertexAttribLFormatEXT(GLuint vao, GLuint attrib, GLint size, GLenum type, GLuint reloff)
{
    typedef void (WINAPI *F)(GLuint, GLint, GLenum, GLuint);
    REAL(F, f, "glVertexAttribLFormat");
    WITH_VAO(vao, f(attrib, size, type, reloff));
}

static void WINAPI e_VertexArrayVertexBindingDivisorEXT(GLuint vao, GLuint binding, GLuint divisor)
{
    typedef void (WINAPI *F)(GLuint, GLuint);
    REAL(F, f, "glVertexBindingDivisor");
    WITH_VAO(vao, f(binding, divisor));
}

// ---------------------------------------------------------------------------
// 3. ARB_buffer_storage emulation
// ---------------------------------------------------------------------------
#define GL_MAP_READ_BIT          0x0001
#define GL_MAP_WRITE_BIT         0x0002
#define GL_MAP_PERSISTENT_BIT    0x0040
#define GL_MAP_COHERENT_BIT      0x0080
#define GL_DYNAMIC_STORAGE_BIT   0x0100
#define GL_STATIC_DRAW           0x88E4
#define GL_DYNAMIC_DRAW          0x88E8
#define GL_DYNAMIC_READ          0x88E9
#define GL_ALREADY_SIGNALED      0x911A
#define GL_CONDITION_SATISFIED   0x911C
#define GL_SYNC_STATUS           0x9114
#define GL_SIGNALED              0x9119
#define GL_TRUE                  1

#define PAGE          4096u
#define ARENA_SIZE    (4ull << 30)          // 4 GB reserved, committed on demand
#define MAX_BUFS      8192

typedef struct {
    GLuint id;
    GLbitfield flags;
    size_t size;      // GL buffer size
    size_t pages;     // committed pages
    char *shadow;     // inside the arena
    int used;
    unsigned last[4]; // first 16 bytes at the previous download (debug logging)
    int gpu_written;  // ever bound as a GPU write target (SSBO/atomic/TF/copy dst/clear)
} pbuf;
static pbuf *pb_find(GLuint id);
static void pb_mark_written(GLuint id);
static void sync_after_gpu(void);
// ordered trace events into the capture frame file (debug captures only)
static void cap_event(const char *desc, int thumb);
static int g_cap_active;
#define TRACE_EV(...) do { if (g_cap_active) { char _b[200]; wsprintfA(_b, __VA_ARGS__); cap_event(_b, 0); } } while (0)

static CRITICAL_SECTION g_pb_cs;
static char *g_arena;                 // MEM_WRITE_WATCH region
static size_t g_arena_next;           // bump pointer (pages)
static pbuf g_pb[MAX_BUFS];           // sorted by shadow address (append order == address order)
static int g_pb_count;
static volatile LONG g_pb_live;       // number of live persistent buffers
static volatile LONG g_pb_readers;    // live persistent buffers with READ access
static volatile LONG g_pb_writers;    // live persistent buffers with WRITE access
static void **g_ww_addrs;             // GetWriteWatch output
static ULONG_PTR g_ww_cap;

static GLenum binding_for_buffer_target(GLenum target)
{
    switch (target) {
    case 0x8892: return 0x8894;  // ARRAY_BUFFER
    case 0x8893: return 0x8895;  // ELEMENT_ARRAY_BUFFER
    case 0x88EB: return 0x88ED;  // PIXEL_PACK_BUFFER
    case 0x88EC: return 0x88EF;  // PIXEL_UNPACK_BUFFER
    case 0x8A11: return 0x8A28;  // UNIFORM_BUFFER
    case 0x8C2A: return 0x8C2A;  // TEXTURE_BUFFER
    case 0x8C8E: return 0x8C8F;  // TRANSFORM_FEEDBACK_BUFFER
    case 0x8F36: return 0x8F36;  // COPY_READ_BUFFER
    case 0x8F37: return 0x8F37;  // COPY_WRITE_BUFFER
    case 0x8F3F: return 0x8F43;  // DRAW_INDIRECT_BUFFER
    case 0x90D2: return 0x90D3;  // SHADER_STORAGE_BUFFER
    case 0x90EE: return 0x90EF;  // DISPATCH_INDIRECT_BUFFER
    case 0x9192: return 0x9193;  // QUERY_BUFFER
    case 0x92C0: return 0x92C1;  // ATOMIC_COUNTER_BUFFER
    default: return 0;
    }
}

static GLuint bound_buffer(GLenum target)
{
    GLenum b = binding_for_buffer_target(target);
    return b ? (GLuint)get_int(b) : 0;
}

// ---------------------------------------------------------------------------
// glTexBufferRange emulation. Parallels ignores the offset and size of a ranged
// buffer texture and exposes the whole buffer from offset 0 (probe T143/T144).
// Every ranged buffer texture therefore gets a private scratch buffer holding a
// copy of its range; the copy is refreshed before the next draw whenever the
// source buffer may have changed (CPU updates, copies, clears, compute passes,
// memory barriers).
// ---------------------------------------------------------------------------
typedef void (WINAPI *PFN_GenBuffersT)(GLsizei, GLuint *);
typedef void (WINAPI *PFN_CopyBufferSubDataT)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr);
typedef void (WINAPI *PFN_BufferDataT)(GLenum, GLsizeiptr, const void *, GLenum);
typedef void (WINAPI *PFN_BindBufferT)(GLenum, GLuint);
typedef void (WINAPI *PFN_TexBufferT)(GLenum, GLenum, GLuint);
typedef struct { GLuint tex, src, scratch; GLintptr off; GLsizeiptr size, cap; GLenum target, fmt; int dirty; } TboRange;
#define TBR_MAX 512
static TboRange g_tbr[TBR_MAX];
static int g_tbr_n;
static volatile LONG g_tbr_dirty;       // number of records waiting for a refresh
static CRITICAL_SECTION g_tbr_cs;
static int g_tbr_cs_ready;
static volatile LONG g_cnt_tbr_copies, g_cnt_tbr_ranges;

static void tbr_lock(void)   { if (!g_tbr_cs_ready) { InitializeCriticalSection(&g_tbr_cs); g_tbr_cs_ready = 1; } EnterCriticalSection(&g_tbr_cs); }
static void tbr_unlock(void) { LeaveCriticalSection(&g_tbr_cs); }

static void tbr_mark_buffer(GLuint buf)      // the contents of buf may have changed
{
    if (!g_tbr_n || !buf) return;
    tbr_lock();
    for (int i = 0; i < g_tbr_n; i++)
        if (g_tbr[i].src == buf && !g_tbr[i].dirty) { g_tbr[i].dirty = 1; InterlockedIncrement(&g_tbr_dirty); }
    tbr_unlock();
}
static void tbr_mark_all(void)               // GPU-side writes (compute, barriers): anything may have changed
{
    if (!g_tbr_n) return;
    tbr_lock();
    for (int i = 0; i < g_tbr_n; i++)
        if (!g_tbr[i].dirty) { g_tbr[i].dirty = 1; InterlockedIncrement(&g_tbr_dirty); }
    tbr_unlock();
}
static void tbr_copy(TboRange *r, PFN_BindBufferT bb, PFN_CopyBufferSubDataT cp)
{
    bb(GL_COPY_READ_BUFFER, r->src);
    bb(GL_COPY_WRITE_BUFFER, r->scratch);
    cp(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, r->off, 0, r->size);
    InterlockedIncrement(&g_cnt_tbr_copies);
}
static void tbr_refresh(void)                // called before every draw / dispatch
{
    if (g_tbr_dirty <= 0) return;
    REAL(PFN_BindBufferT, bb, "glBindBuffer");
    REAL(PFN_CopyBufferSubDataT, cp, "glCopyBufferSubData");
    GLint oldr = get_int(GL_COPY_READ_BUFFER), oldw = get_int(GL_COPY_WRITE_BUFFER);
    tbr_lock();
    for (int i = 0; i < g_tbr_n; i++)
        if (g_tbr[i].dirty) { tbr_copy(&g_tbr[i], bb, cp); g_tbr[i].dirty = 0; }
    InterlockedExchange(&g_tbr_dirty, 0);
    tbr_unlock();
    bb(GL_COPY_READ_BUFFER, (GLuint)oldr);
    bb(GL_COPY_WRITE_BUFFER, (GLuint)oldw);
}
static void tbr_forget_tex(GLuint tex)
{
    if (!g_tbr_n || !tex) return;
    tbr_lock();
    for (int i = 0; i < g_tbr_n; i++)
        if (g_tbr[i].tex == tex) { if (g_tbr[i].dirty) InterlockedDecrement(&g_tbr_dirty); g_tbr[i] = g_tbr[--g_tbr_n]; break; }
    tbr_unlock();
}
static void tbr_forget_cur(GLenum target)    // the bound texture is being re-pointed with glTexBuffer
{
    GLenum q = tex_binding_for(target);
    if (q) tbr_forget_tex((GLuint)get_int(q));
}
static void tbr_forget_src(GLuint buf)       // source buffer deleted
{
    if (!g_tbr_n || !buf) return;
    tbr_lock();
    for (int i = 0; i < g_tbr_n; )
        if (g_tbr[i].src == buf) { if (g_tbr[i].dirty) InterlockedDecrement(&g_tbr_dirty); g_tbr[i] = g_tbr[--g_tbr_n]; } else i++;
    tbr_unlock();
}
// Returns 1 when the range was emulated (the texture now points at the scratch buffer).
static int tbr_set(GLenum target, GLenum fmt, GLuint buf, GLintptr off, GLsizeiptr size)
{
    GLenum q = tex_binding_for(target);
    GLuint tex = q ? (GLuint)get_int(q) : 0;
    if (!tex) return 0;
    if (!buf || size <= 0) { tbr_forget_tex(tex); return 0; }
    REAL_RET(PFN_BindBufferT, bb, "glBindBuffer", 0);
    REAL_RET(PFN_CopyBufferSubDataT, cp, "glCopyBufferSubData", 0);
    REAL_RET(PFN_GenBuffersT, gen, "glGenBuffers", 0);
    REAL_RET(PFN_BufferDataT, bd, "glBufferData", 0);
    REAL_RET(PFN_TexBufferT, tb, "glTexBuffer", 0);
    GLint oldr = get_int(GL_COPY_READ_BUFFER), oldw = get_int(GL_COPY_WRITE_BUFFER);
    tbr_lock();
    TboRange *r = NULL;
    for (int i = 0; i < g_tbr_n; i++) if (g_tbr[i].tex == tex) { r = &g_tbr[i]; break; }
    if (!r) {
        if (g_tbr_n >= TBR_MAX) { tbr_unlock(); return 0; }
        r = &g_tbr[g_tbr_n++]; memset(r, 0, sizeof *r); r->tex = tex;
        InterlockedIncrement(&g_cnt_tbr_ranges);
    }
    if (!r->scratch) gen(1, &r->scratch);
    if (r->cap < size) { bb(GL_COPY_WRITE_BUFFER, r->scratch); bd(GL_COPY_WRITE_BUFFER, size, NULL, 0x88E6 /*STATIC_COPY*/); r->cap = size; }
    if (r->dirty) { r->dirty = 0; InterlockedDecrement(&g_tbr_dirty); }
    r->src = buf; r->off = off; r->size = size; r->target = target; r->fmt = fmt;
    tbr_copy(r, bb, cp);
    GLuint scratch = r->scratch;
    tbr_unlock();
    bb(GL_COPY_READ_BUFFER, (GLuint)oldr);
    bb(GL_COPY_WRITE_BUFFER, (GLuint)oldw);
    tb(target, fmt, scratch);                 // the whole scratch buffer is exactly the range
    return 1;
}
// hooks that invalidate the copies
void shim_sync_before_gpu(void);
typedef void (WINAPI *PFN_MemoryBarrierT)(GLbitfield);
typedef void (WINAPI *PFN_DispatchComputeT)(GLuint, GLuint, GLuint);
typedef void (WINAPI *PFN_DispatchComputeIndirectT)(GLintptr);
typedef void (WINAPI *PFN_ClearBufferSubDataT)(GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void *);
typedef void (WINAPI *PFN_ClearBufferDataT)(GLenum, GLenum, GLenum, GLenum, const void *);
static PFN_CopyBufferSubDataT p_glCopyBufferSubData_app;
static PFN_MemoryBarrierT p_glMemoryBarrier_app;
static PFN_DispatchComputeT p_glDispatchCompute_app;
static PFN_DispatchComputeIndirectT p_glDispatchComputeIndirect_app;
static PFN_ClearBufferSubDataT p_glClearBufferSubData_app;
static PFN_ClearBufferDataT p_glClearBufferData_app;
static void WINAPI h_glCopyBufferSubData(GLenum rt, GLenum wt, GLintptr ro, GLintptr wo, GLsizeiptr sz)
{
    p_glCopyBufferSubData_app(rt, wt, ro, wo, sz);
    pb_mark_written(bound_buffer(wt));
    GLuint dst = bound_buffer(wt);
    TRACE_EV("CopyBufferSubData %u@%d -> %u@%d size %d", bound_buffer(rt), (int)ro, dst, (int)wo, (int)sz);
    tbr_mark_buffer(dst);
    if (g_pb_readers > 0) {
        EnterCriticalSection(&g_pb_cs);
        pbuf *b = pb_find(dst);
        LeaveCriticalSection(&g_pb_cs);
        if (b) {            // GPU copy into a persistently mapped read buffer: refresh its CPU copy now
            sync_after_gpu();
            if (g_debug) { static LONG nlog; if (InterlockedIncrement(&nlog) <= 40) { const unsigned *u = (const unsigned *)b->shadow; log_fmt("COPY -> pb buf %u (%u bytes from buf %u @%d): now %08x %08x %08x %08x", dst, (unsigned)sz, bound_buffer(rt), (int)ro, u[0], u[1], u[2], u[3]); } }
        }
    }
}
static void WINAPI h_glMemoryBarrier(GLbitfield bits)
{
    p_glMemoryBarrier_app(bits);
    TRACE_EV("MemoryBarrier bits=%x", bits);
    tbr_mark_all();
    if ((bits & 0x4000 /*CLIENT_MAPPED_BUFFER_BARRIER_BIT*/) && g_pb_readers > 0) sync_after_gpu();
}
// binding trace (capture only): which buffers the app binds for storage/atomic/copy targets
typedef void (WINAPI *PFN_BindBufferApp)(GLenum, GLuint);
typedef void (WINAPI *PFN_BindBufferBaseApp)(GLenum, GLuint, GLuint);
typedef void (WINAPI *PFN_BindBufferRangeApp)(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr);
static PFN_BindBufferApp p_glBindBuffer_app, p_glBindBufferARB_app;
static PFN_BindBufferBaseApp p_glBindBufferBase_app, p_glBindBufferBaseEXT_app;
static PFN_BindBufferRangeApp p_glBindBufferRange_app, p_glBindBufferRangeEXT_app;
static int trace_target(GLenum t) { return t == 0x90D2 || t == 0x92C0 || t == 0x8F36 || t == 0x8F37 || t == 0x8C8E; }
static void WINAPI h_glBindBuffer(GLenum t, GLuint b) { p_glBindBuffer_app(t, b); if (g_cap_active && trace_target(t)) TRACE_EV("BindBuffer t=%x buf=%u", t, b); }
static void WINAPI h_glBindBufferARB(GLenum t, GLuint b) { p_glBindBufferARB_app(t, b); if (g_cap_active && trace_target(t)) TRACE_EV("BindBufferARB t=%x buf=%u", t, b); }
static void WINAPI h_glBindBufferBase(GLenum t, GLuint i, GLuint b) { p_glBindBufferBase_app(t, i, b); if (t == 0x90D2 || t == 0x92C0 || t == 0x8C8E) pb_mark_written(b); if (g_cap_active && t != 0x8A11) TRACE_EV("BindBufferBase t=%x idx=%u buf=%u", t, i, b); }
static void WINAPI h_glBindBufferBaseEXT(GLenum t, GLuint i, GLuint b) { p_glBindBufferBaseEXT_app(t, i, b); if (t == 0x90D2 || t == 0x92C0 || t == 0x8C8E) pb_mark_written(b); if (g_cap_active && t != 0x8A11) TRACE_EV("BindBufferBaseEXT t=%x idx=%u buf=%u", t, i, b); }
static void WINAPI h_glBindBufferRange(GLenum t, GLuint i, GLuint b, GLintptr o, GLsizeiptr s) { p_glBindBufferRange_app(t, i, b, o, s); if (t == 0x90D2 || t == 0x92C0 || t == 0x8C8E) pb_mark_written(b); if (g_cap_active && t != 0x8A11) TRACE_EV("BindBufferRange t=%x idx=%u buf=%u off=%d size=%d", t, i, b, (int)o, (int)s); }
static void WINAPI h_glBindBufferRangeEXT(GLenum t, GLuint i, GLuint b, GLintptr o, GLsizeiptr s) { p_glBindBufferRangeEXT_app(t, i, b, o, s); if (t == 0x90D2 || t == 0x92C0 || t == 0x8C8E) pb_mark_written(b); if (g_cap_active && t != 0x8A11) TRACE_EV("BindBufferRangeEXT t=%x idx=%u buf=%u off=%d size=%d", t, i, b, (int)o, (int)s); }
typedef void (WINAPI *PFN_GetBufferSubDataApp)(GLenum, GLintptr, GLsizeiptr, void *);
static PFN_GetBufferSubDataApp p_glGetBufferSubData_app;
// 3DEXPERIENCE's GPU occlusion culling packs visibility bits into an SSBO and then reads a
// small persistent read-back buffer with glGetBufferSubData. On this driver path nothing ever
// writes that read-back buffer, so it reads 0 = "nothing visible" and skips the shaded faces.
// Reads from such never-written read-back buffers report "all visible" (bit set = visible);
// this only disables the culling speed-up. SHIM_OCC_READBACK=real turns it off.
static int g_occ_real = -1;
static void pb_mark_written(GLuint id)
{
    if (!g_pb_live || !id) return;
    EnterCriticalSection(&g_pb_cs);
    pbuf *b = pb_find(id);
    if (b) b->gpu_written = 1;
    LeaveCriticalSection(&g_pb_cs);
}
static int occ_spoof(GLuint id, GLintptr o, GLsizeiptr sz, void *d)
{
    if (g_occ_real < 0) {
        char v[16]; DWORD n = GetEnvironmentVariableA("SHIM_OCC_READBACK", v, sizeof v);
        g_occ_real = n > 0 && n < sizeof v && (v[0] == 'r' || v[0] == 'R');
        log_fmt("OCCLUSION read-back: %s", g_occ_real ? "real (SHIM_OCC_READBACK=real)" : "never-written buffers report all visible");
    }
    if (g_occ_real || !g_pb_live || !d || sz <= 0) return 0;
    int hit = 0;
    EnterCriticalSection(&g_pb_cs);
    pbuf *b = pb_find(id);
    if (b && (b->flags & GL_MAP_READ_BIT) && !(b->flags & GL_MAP_WRITE_BIT) && !b->gpu_written && b->size <= 64 && (size_t)o + (size_t)sz <= b->size)
        hit = 1;
    LeaveCriticalSection(&g_pb_cs);
    if (!hit) return 0;
    memset(d, 0xFF, (size_t)sz);
    static LONG n; if (InterlockedIncrement(&n) <= 5) log_fmt("OCCLUSION read-back buf %u @%d size %d -> all visible", id, (int)o, (int)sz);
    return 1;
}
static void WINAPI h_glGetBufferSubData(GLenum t, GLintptr o, GLsizeiptr s, void *d)
{
    if (occ_spoof(bound_buffer(t), o, s, d)) { TRACE_EV("GetBufferSubData buf %u @%d size %d -> spoofed all visible", bound_buffer(t), (int)o, (int)s); return; }
    p_glGetBufferSubData_app(t, o, s, d);
    if (d && s >= 4) TRACE_EV("GetBufferSubData buf %u @%d size %d -> %08x", bound_buffer(t), (int)o, (int)s, *(const unsigned *)d);
    if (g_debug && d && s <= 64) { static LONG nlog; if (InterlockedIncrement(&nlog) <= 60) { const unsigned *u = (const unsigned *)d; log_fmt("APP GetBufferSubData buf %u @%d size %d: %08x %08x %08x %08x", bound_buffer(t), (int)o, (int)s, u[0], s > 4 ? u[1] : 0, s > 8 ? u[2] : 0, s > 12 ? u[3] : 0); } }
}
#define GL_BUFFER_IMMUTABLE_STORAGE 0x8220
#define GL_BUFFER_STORAGE_FLAGS     0x8221
typedef void (WINAPI *PFN_GetBufferParameterivT)(GLenum, GLenum, GLint *);
typedef void (WINAPI *PFN_GetBufferParameteri64vT)(GLenum, GLenum, GLuint64 *);
static PFN_GetBufferParameterivT p_glGetBufferParameteriv_app, p_glGetBufferParameterivARB_app;
static PFN_GetBufferParameteri64vT p_glGetBufferParameteri64v_app;
// ARB_buffer_storage parameters of emulated buffers (the driver would raise INVALID_ENUM)
static int pb_param(GLenum target, GLenum pname, GLint *out)
{
    if (pname != GL_BUFFER_IMMUTABLE_STORAGE && pname != GL_BUFFER_STORAGE_FLAGS)
        return 0;
    GLuint id = bound_buffer(target);
    int got = 0;
    EnterCriticalSection(&g_pb_cs);
    pbuf *b = pb_find(id);
    if (b) { *out = pname == GL_BUFFER_IMMUTABLE_STORAGE ? 1 : (GLint)b->flags; got = 1; }
    LeaveCriticalSection(&g_pb_cs);
    if (!got) *out = 0;   // not an emulated immutable buffer: IMMUTABLE_STORAGE = false, flags = 0
    if (g_debug) { static LONG n; if (InterlockedIncrement(&n) <= 40) log_fmt("APP GetBufferParameter buf %u pname %x -> %d%s", id, pname, *out, got ? " (pb)" : ""); }
    return 1;
}
static void WINAPI h_glGetBufferParameteriv(GLenum t, GLenum pn, GLint *p)
{
    if (p && pb_param(t, pn, p)) return;
    p_glGetBufferParameteriv_app(t, pn, p);
    if (g_debug && p && (pn == 0x88BC || pn == 0x911F || pn == 0x9120 || pn == 0x9121)) { static LONG n; if (InterlockedIncrement(&n) <= 40) log_fmt("APP GetBufferParameter buf %u pname %x -> %d", bound_buffer(t), pn, *p); }
}
static void WINAPI h_glGetBufferParameterivARB(GLenum t, GLenum pn, GLint *p)
{
    if (p && pb_param(t, pn, p)) return;
    p_glGetBufferParameterivARB_app(t, pn, p);
}
static void WINAPI h_glGetBufferParameteri64v(GLenum t, GLenum pn, GLuint64 *p)
{
    GLint v;
    if (p && pb_param(t, pn, &v)) { *p = (GLuint64)(GLuint)v; return; }
    p_glGetBufferParameteri64v_app(t, pn, p);
}
static void WINAPI h_glDispatchCompute(GLuint x, GLuint y, GLuint z) { shim_sync_before_gpu(); p_glDispatchCompute_app(x, y, z); tbr_mark_all(); }
static void WINAPI h_glDispatchComputeIndirect(GLintptr o) { shim_sync_before_gpu(); p_glDispatchComputeIndirect_app(o); tbr_mark_all(); }
static void WINAPI h_glClearBufferSubData(GLenum t, GLenum f, GLintptr o, GLsizeiptr s, GLenum fm, GLenum ty, const void *d)
{
    p_glClearBufferSubData_app(t, f, o, s, fm, ty, d);
    pb_mark_written(bound_buffer(t));
    tbr_mark_buffer(bound_buffer(t));
}
static void WINAPI h_glClearBufferData(GLenum t, GLenum f, GLenum fm, GLenum ty, const void *d)
{
    p_glClearBufferData_app(t, f, fm, ty, d);
    pb_mark_written(bound_buffer(t));
    tbr_mark_buffer(bound_buffer(t));
}

static int arena_init(void)
{
    if (g_arena)
        return 1;
    g_arena = (char *)VirtualAlloc(NULL, ARENA_SIZE, MEM_RESERVE | MEM_WRITE_WATCH, PAGE_READWRITE);
    if (!g_arena) {
        log_line("ARENA: reserve failed", NULL);
        return 0;
    }
    g_ww_cap = (ULONG_PTR)(ARENA_SIZE / PAGE);
    g_ww_addrs = (void **)VirtualAlloc(NULL, g_ww_cap * sizeof(void *), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!g_ww_addrs) {
        log_line("ARENA: address list alloc failed", NULL);
        return 0;
    }
    log_line("ARENA: 4 GB write-watch region reserved", NULL);
    return 1;
}

// caller holds g_pb_cs
static pbuf *pb_find(GLuint id)
{
    if (!id)
        return NULL;
    for (int i = g_pb_count - 1; i >= 0; i--)
        if (g_pb[i].used && g_pb[i].id == id)
            return &g_pb[i];
    return NULL;
}

// caller holds g_pb_cs
static pbuf *pb_by_addr(const char *p)
{
    int lo = 0, hi = g_pb_count - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        pbuf *b = &g_pb[mid];
        if (p < b->shadow)
            hi = mid - 1;
        else if (p >= b->shadow + b->pages * PAGE)
            lo = mid + 1;
        else
            return b->used ? b : NULL;
    }
    return NULL;
}

// caller holds g_pb_cs
static void pb_release(pbuf *b)
{
    if (!b || !b->used)
        return;
    VirtualFree(b->shadow, b->pages * PAGE, MEM_DECOMMIT);
    InterlockedDecrement(&g_pb_live);
    if (b->flags & GL_MAP_READ_BIT)
        InterlockedDecrement(&g_pb_readers);
    if (b->flags & GL_MAP_WRITE_BIT)
        InterlockedDecrement(&g_pb_writers);
    b->used = 0;
}

typedef void (WINAPI *PFN_BufferData)(GLenum, GLsizeiptr, const void *, GLenum);
typedef void (WINAPI *PFN_BufferSubData)(GLenum, GLintptr, GLsizeiptr, const void *);
typedef void (WINAPI *PFN_GetBufferSubData)(GLenum, GLintptr, GLsizeiptr, void *);
static PFN_BindBuffer p_glBindBuffer;
static PFN_BufferSubData p_glBufferSubData;
static PFN_GetBufferSubData p_glGetBufferSubData;

static int ensure_buffer_procs(void)
{
    if (!p_glBindBuffer) p_glBindBuffer = (PFN_BindBuffer)real_proc("glBindBuffer");
    if (!p_glBufferSubData) p_glBufferSubData = (PFN_BufferSubData)real_proc("glBufferSubData");
    if (!p_glGetBufferSubData) p_glGetBufferSubData = (PFN_GetBufferSubData)real_proc("glGetBufferSubData");
    return p_glBindBuffer && p_glBufferSubData && p_glGetBufferSubData;
}

static volatile LONG64 g_stat_uploads, g_stat_upload_bytes, g_stat_downloads;
static volatile LONG g_cnt_fence, g_cnt_wait_ok, g_cnt_wait_other, g_cnt_getsync_sig,
                     g_cnt_map_persist, g_cnt_map_other, g_cnt_swaps;
static void maybe_log_stats(void);

// Upload every CPU-written page of writable persistent buffers to the GPU.
// Called before each GPU command; cheap when nothing was written.
void shim_sync_before_gpu(void)
{
    if (g_debug)
        maybe_log_stats();
    if (g_tbr_dirty > 0)
        tbr_refresh();
    if (g_pb_writers == 0 || !ensure_buffer_procs())   // read-back-only buffers need no upload
        return;
    EnterCriticalSection(&g_pb_cs);
    ULONG_PTR count = g_ww_cap;
    ULONG gran = 0;
    size_t span = g_arena_next * PAGE;
    if (span && GetWriteWatch(WRITE_WATCH_FLAG_RESET, g_arena, span, g_ww_addrs, &count, &gran) == 0 && count) {
        GLint old = get_int(GL_COPY_WRITE_BUFFER);
        GLuint bound = (GLuint)old;
        ULONG_PTR i = 0;
        while (i < count) {
            char *start = (char *)g_ww_addrs[i];
            pbuf *b = pb_by_addr(start);
            ULONG_PTR j = i + 1;
            while (j < count && (char *)g_ww_addrs[j] == (char *)g_ww_addrs[j - 1] + PAGE &&
                   pb_by_addr((char *)g_ww_addrs[j]) == b)
                j++;
            if (b && (b->flags & GL_MAP_WRITE_BIT)) {
                size_t off = (size_t)(start - b->shadow);
                size_t len = (j - i) * PAGE;
                if (off < b->size) {
                    if (off + len > b->size)
                        len = b->size - off;
                    if (bound != b->id)
                        p_glBindBuffer(GL_COPY_WRITE_BUFFER, b->id), bound = b->id;
                    p_glBufferSubData(GL_COPY_WRITE_BUFFER, (GLintptr)off, (GLsizeiptr)len, b->shadow + off);
                    InterlockedIncrement64(&g_stat_uploads);
                    InterlockedAdd64(&g_stat_upload_bytes, (LONG64)len);
                }
            }
            i = j;
        }
        if (bound != (GLuint)old)
            p_glBindBuffer(GL_COPY_WRITE_BUFFER, (GLuint)old);
    }
    LeaveCriticalSection(&g_pb_cs);
}

// Refresh the CPU copy of read-back persistent buffers from the GPU.
static void sync_after_gpu(void)
{
    if (g_pb_readers == 0 || !ensure_buffer_procs())
        return;
    shim_sync_before_gpu();
    EnterCriticalSection(&g_pb_cs);
    GLint old = get_int(GL_COPY_READ_BUFFER);
    for (int i = 0; i < g_pb_count; i++) {
        pbuf *b = &g_pb[i];
        if (!b->used || !(b->flags & GL_MAP_READ_BIT))
            continue;
        p_glBindBuffer(GL_COPY_READ_BUFFER, b->id);
        p_glGetBufferSubData(GL_COPY_READ_BUFFER, 0, (GLsizeiptr)b->size, b->shadow);
        ResetWriteWatch(b->shadow, b->pages * PAGE);   // our own write, not the app's
        LONG64 nd = InterlockedIncrement64(&g_stat_downloads);
        if (b->size >= 16) { const unsigned *u = (const unsigned *)b->shadow; TRACE_EV("Download pb %u: %08x %08x %08x %08x", b->id, u[0], u[1], u[2], u[3]); }
        if (g_debug && b->size >= 16) {
            const unsigned *u = (const unsigned *)b->shadow;
            int changed = memcmp(b->last, u, 16) != 0;
            if (nd <= 20 || nd % 500 == 0 || changed) {
                static LONG nchg; LONG k = changed ? InterlockedIncrement(&nchg) : 0;
                if (!changed || k <= 200)
                    log_fmt("READBACK buf %u size %u%s: %08x %08x %08x %08x", b->id, (unsigned)b->size, changed ? " CHANGED" : "", u[0], u[1], u[2], u[3]);
                memcpy(b->last, u, 16);
            }
        }
    }
    p_glBindBuffer(GL_COPY_READ_BUFFER, (GLuint)old);
    LeaveCriticalSection(&g_pb_cs);
}

static void WINAPI e_BufferStorage(GLenum target, GLsizeiptr size, const void *data, GLbitfield flags)
{
    REAL(PFN_BufferData, bufdata, "glBufferData");
    GLenum usage = (flags & GL_MAP_READ_BIT) ? GL_DYNAMIC_READ
                 : (flags & (GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT)) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    bufdata(target, size, data, usage);
    GLuint id = bound_buffer(target);

    EnterCriticalSection(&g_pb_cs);
    pb_release(pb_find(id));   // re-specification drops an old shadow
    if ((flags & GL_MAP_PERSISTENT_BIT) && id && size > 0 && arena_init()) {
        size_t pages = ((size_t)size + PAGE - 1) / PAGE;
        if ((g_arena_next + pages) * PAGE > ARENA_SIZE || g_pb_count >= MAX_BUFS) {
            log_fmt("ARENA FULL: buffer %u size %u", id, (unsigned)size);
        } else {
            char *mem = g_arena + g_arena_next * PAGE;
            if (VirtualAlloc(mem, pages * PAGE, MEM_COMMIT, PAGE_READWRITE)) {
                pbuf *b = &g_pb[g_pb_count++];
                b->id = id; b->flags = flags; b->size = (size_t)size;
                b->pages = pages; b->shadow = mem; b->used = 1;
                g_arena_next += pages;
                if (data)
                    memcpy(mem, data, (size_t)size);
                ResetWriteWatch(mem, pages * PAGE);
                InterlockedIncrement(&g_pb_live);
                if (flags & GL_MAP_READ_BIT)
                    InterlockedIncrement(&g_pb_readers);
                if (flags & GL_MAP_WRITE_BIT)
                    InterlockedIncrement(&g_pb_writers);
                static LONG logged;
                LONG nl = InterlockedIncrement(&logged);
                if (nl <= 20)
                    log_fmt("PERSISTENT buffer %u size %u flags 0x%x target %x", id, (unsigned)size, flags, target);
                if (g_debug && nl <= 6)
                    trace_dump_ring("BufferStorage(persistent)", 160);
            }
        }
    }
    LeaveCriticalSection(&g_pb_cs);
}

// ARB_buffer_storage's EXT_direct_state_access variant
static void WINAPI e_NamedBufferStorageEXT(GLuint buf, GLsizeiptr size, const void *data, GLbitfield flags)
{
    REAL(PFN_BindBuffer, bind, "glBindBuffer");
    GLint old = get_int(GL_COPY_WRITE_BUFFER);
    bind(GL_COPY_WRITE_BUFFER, buf);
    e_BufferStorage(GL_COPY_WRITE_BUFFER, size, data, flags);
    bind(GL_COPY_WRITE_BUFFER, (GLuint)old);
}

typedef void *(WINAPI *PFN_MapBufferRange)(GLenum, GLintptr, GLsizeiptr, GLbitfield);
static PFN_MapBufferRange p_glMapBufferRange;
static void *WINAPI h_glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    if (g_pb_live) {
        GLuint id = bound_buffer(target);
        EnterCriticalSection(&g_pb_cs);
        pbuf *b = pb_find(id);
        if (b) {
            char *p = b->shadow + offset;
            int want_read = (access & GL_MAP_READ_BIT) != 0;
            LeaveCriticalSection(&g_pb_cs);
            InterlockedIncrement(&g_cnt_map_persist);
            TRACE_EV("MapBufferRange pb %u off=%d len=%d access=%x", id, (int)offset, (int)length, access);
            if (want_read)
                sync_after_gpu();
            return p;
        }
        LeaveCriticalSection(&g_pb_cs);
    }
    InterlockedIncrement(&g_cnt_map_other);
    return p_glMapBufferRange(target, offset, length, access & ~(GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
}

typedef void *(WINAPI *PFN_MapBuffer)(GLenum, GLenum);
static PFN_MapBuffer p_glMapBuffer;
static void *WINAPI h_glMapBuffer(GLenum target, GLenum access)
{
    if (g_pb_live) {
        GLuint id = bound_buffer(target);
        EnterCriticalSection(&g_pb_cs);
        pbuf *b = pb_find(id);
        char *p = b ? b->shadow : NULL;
        LeaveCriticalSection(&g_pb_cs);
        if (p) {
            TRACE_EV("MapBuffer pb %u access=%x", id, access);
            if (access != 0x88B9 /* GL_WRITE_ONLY */)
                sync_after_gpu();
            return p;
        }
    }
    return p_glMapBuffer(target, access);
}

typedef GLboolean (WINAPI *PFN_UnmapBuffer)(GLenum);
static PFN_UnmapBuffer p_glUnmapBuffer;
static GLboolean WINAPI h_glUnmapBuffer(GLenum target)
{
    if (g_tbr_n) tbr_mark_buffer(bound_buffer(target));
    if (g_pb_live) {
        GLuint id = bound_buffer(target);
        EnterCriticalSection(&g_pb_cs);
        pbuf *b = pb_find(id);
        LeaveCriticalSection(&g_pb_cs);
        if (b) {
            shim_sync_before_gpu();
            return GL_TRUE;
        }
    }
    return p_glUnmapBuffer(target);
}

typedef void (WINAPI *PFN_FlushMappedBufferRange)(GLenum, GLintptr, GLsizeiptr);
static PFN_FlushMappedBufferRange p_glFlushMappedBufferRange;
static void WINAPI h_glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
    if (g_tbr_n) tbr_mark_buffer(bound_buffer(target));
    if (g_pb_live) {
        GLuint id = bound_buffer(target);
        EnterCriticalSection(&g_pb_cs);
        pbuf *b = pb_find(id);
        LeaveCriticalSection(&g_pb_cs);
        if (b) {
            shim_sync_before_gpu();
            return;
        }
    }
    p_glFlushMappedBufferRange(target, offset, length);
}

typedef void (WINAPI *PFN_DeleteBuffers)(GLsizei, const GLuint *);
static PFN_DeleteBuffers p_glDeleteBuffers;
static void WINAPI h_glDeleteBuffers(GLsizei n, const GLuint *ids)
{
    if (ids)
        for (GLsizei i = 0; i < n; i++) {
            forget_buffer(ids[i]);
            if (g_tbr_n) tbr_forget_src(ids[i]);
        }
    if (g_pb_live && ids) {
        EnterCriticalSection(&g_pb_cs);
        for (GLsizei i = 0; i < n; i++)
            pb_release(pb_find(ids[i]));
        LeaveCriticalSection(&g_pb_cs);
    }
    p_glDeleteBuffers(n, ids);
}

typedef void (WINAPI *PFN_BufferSubDataT)(GLenum, GLintptr, GLsizeiptr, const void *);
static PFN_BufferSubDataT p_glBufferSubData_app;
static void WINAPI h_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data)
{
    if (g_tbr_n) tbr_mark_buffer(bound_buffer(target));
    if (g_pb_live) {
        GLuint id = bound_buffer(target);
        EnterCriticalSection(&g_pb_cs);
        pbuf *b = pb_find(id);
        if (b && data && (size_t)offset + (size_t)size <= b->size) {
            memcpy(b->shadow + offset, data, (size_t)size);   // keep shadow in step
        }
        LeaveCriticalSection(&g_pb_cs);
    }
    p_glBufferSubData_app(target, offset, size, data);
}

typedef GLenum (WINAPI *PFN_ClientWaitSync)(GLsync, GLbitfield, GLuint64);
static PFN_ClientWaitSync p_glClientWaitSync;
static GLenum WINAPI h_glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    shim_sync_before_gpu();
    GLenum r = p_glClientWaitSync(sync, flags, timeout);
    TRACE_EV("ClientWaitSync flags=%x -> %x", flags, r);
    if (r == GL_ALREADY_SIGNALED || r == GL_CONDITION_SATISFIED) {
        InterlockedIncrement(&g_cnt_wait_ok);
        sync_after_gpu();
    } else {
        InterlockedIncrement(&g_cnt_wait_other);
    }
    return r;
}

typedef void (WINAPI *PFN_GetSynciv)(GLsync, GLenum, GLsizei, GLsizei *, GLint *);
static PFN_GetSynciv p_glGetSynciv;
static void WINAPI h_glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
    p_glGetSynciv(sync, pname, bufSize, length, values);
    TRACE_EV("GetSynciv pname=%x value=%d", pname, (values && bufSize > 0) ? values[0] : -1);
    if (pname == GL_SYNC_STATUS && values && bufSize > 0 && values[0] == GL_SIGNALED) {
        InterlockedIncrement(&g_cnt_getsync_sig);
        sync_after_gpu();
    }
}

typedef GLsync (WINAPI *PFN_FenceSync)(GLenum, GLbitfield);
static PFN_FenceSync p_glFenceSync;
static GLsync WINAPI h_glFenceSync(GLenum cond, GLbitfield flags)
{
    InterlockedIncrement(&g_cnt_fence);
    shim_sync_before_gpu();
    TRACE_EV("FenceSync");
    return p_glFenceSync(cond, flags);
}

// Re-specifying a buffer with glBufferData drops its persistent shadow.
typedef void (WINAPI *PFN_BufferDataApp)(GLenum, GLsizeiptr, const void *, GLenum);
static PFN_BufferDataApp p_glBufferData_app;
static void WINAPI h_glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
    if (g_tbr_n) tbr_mark_buffer(bound_buffer(target));
    if (g_pb_live) {
        GLuint id = bound_buffer(target);
        EnterCriticalSection(&g_pb_cs);
        pb_release(pb_find(id));
        LeaveCriticalSection(&g_pb_cs);
    }
    p_glBufferData_app(target, size, data, usage);
}

static void cap_frame_end(HDC dc);
// Frame end: push CPU writes, then refresh read-back buffers so that values the
// GPU produced this frame are visible through the persistent pointers.
typedef BOOL (WINAPI *PFN_wglSwapBuffers)(HDC);
BOOL WINAPI wglSwapBuffers(HDC dc)
{
    shim_init();
    InterlockedIncrement(&g_cnt_swaps);
    shim_sync_before_gpu();
    sync_after_gpu();
    if (g_debug)
        cap_frame_end(dc);
    return ((PFN_wglSwapBuffers)g_tbl[IDX_wglSwapBuffers])(dc);
}

typedef void (WINAPI *PFN_glFinish)(void);
void WINAPI glFinish(void)
{
    shim_init();
    shim_sync_before_gpu();
    ((PFN_glFinish)g_tbl[IDX_glFinish])();
    TRACE_EV("Finish");
    sync_after_gpu();
}

// ---------------------------------------------------------------------------
// 4. Multi-draw indirect: Parallels executes only the first command of
//    glMultiDrawElementsIndirect / glMultiDrawArraysIndirect when drawcount > 1
//    (no GL error; verified with gltest2 T20-T22, T53, T56). Single indirect
//    draws honour the offset, so each command is issued on its own.
// ---------------------------------------------------------------------------
typedef void (WINAPI *PFN_MDEI)(GLenum, GLenum, const void *, GLsizei, GLsizei);
typedef void (WINAPI *PFN_MDAI)(GLenum, const void *, GLsizei, GLsizei);
typedef void (WINAPI *PFN_DEI)(GLenum, GLenum, const void *);
typedef void (WINAPI *PFN_DAI)(GLenum, const void *);
static PFN_MDEI p_glMultiDrawElementsIndirect;
static PFN_MDAI p_glMultiDrawArraysIndirect;
static volatile LONG g_cnt_mdi_split, g_cnt_mdi_cmds;

static void WINAPI h_glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    shim_sync_before_gpu();
    if (drawcount <= 1) {
        p_glMultiDrawElementsIndirect(mode, type, indirect, drawcount, stride);
        return;
    }
    REAL(PFN_DEI, dei, "glDrawElementsIndirect");
    size_t step = stride > 0 ? (size_t)stride : 20;
    InterlockedIncrement(&g_cnt_mdi_split);
    InterlockedAdd(&g_cnt_mdi_cmds, drawcount);
    for (GLsizei i = 0; i < drawcount; i++)
        dei(mode, type, (const char *)indirect + (size_t)i * step);
}

static void WINAPI h_glMultiDrawArraysIndirect(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    shim_sync_before_gpu();
    if (drawcount <= 1) {
        p_glMultiDrawArraysIndirect(mode, indirect, drawcount, stride);
        return;
    }
    REAL(PFN_DAI, dai, "glDrawArraysIndirect");
    size_t step = stride > 0 ? (size_t)stride : 16;
    InterlockedIncrement(&g_cnt_mdi_split);
    InterlockedAdd(&g_cnt_mdi_cmds, drawcount);
    for (GLsizei i = 0; i < drawcount; i++)
        dai(mode, (const char *)indirect + (size_t)i * step);
}

// ---------------------------------------------------------------------------
// Depth blits out of multisample depth TEXTURES. Parallels raises
// GL_INVALID_OPERATION and copies nothing when the read framebuffer's depth
// attachment is a multisample texture (probe T171/T176); copies from
// multisample renderbuffers work. 3DEXPERIENCE copies its 4x MSAA scene depth
// (a texture) into the occlusion-test framebuffer this way, so every bounding
// box failed the depth test and all parts counted as hidden. The adapter
// resolves such blits with a small shader that writes gl_FragDepth from
// sample 0 of the multisample texture (stencil is not copied).
// ---------------------------------------------------------------------------
#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER 0x8CA8
#endif
#ifndef GL_READ_FRAMEBUFFER_BINDING
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#endif
#ifndef GL_DRAW_FRAMEBUFFER_BINDING
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#endif
#ifndef GL_DEPTH_ATTACHMENT
#define GL_DEPTH_ATTACHMENT 0x8D00
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#endif
#ifndef GL_TEXTURE
#define GL_TEXTURE 0x1702
#endif
#ifndef GL_CURRENT_PROGRAM
#define GL_CURRENT_PROGRAM 0x8B8D
#endif
#ifndef GL_PROGRAM_PIPELINE_BINDING
#define GL_PROGRAM_PIPELINE_BINDING 0x825A
#endif
#ifndef GL_VERTEX_ARRAY_BINDING
#define GL_VERTEX_ARRAY_BINDING 0x85B5
#endif
#ifndef GL_VIEWPORT
#define GL_VIEWPORT 0x0BA2
#endif
#ifndef GL_SCISSOR_BOX
#define GL_SCISSOR_BOX 0x0C10
#endif
#ifndef GL_DEPTH_FUNC
#define GL_DEPTH_FUNC 0x0B74
#endif
#ifndef GL_POLYGON_MODE
#define GL_POLYGON_MODE 0x0B40
#endif
#ifndef GL_COLOR_WRITEMASK
#define GL_COLOR_WRITEMASK 0x0C23
#endif
#ifndef GL_DEPTH_WRITEMASK
#define GL_DEPTH_WRITEMASK 0x0B72
#endif
#ifndef GL_SCISSOR_TEST
#define GL_SCISSOR_TEST 0x0C11
#endif
#ifndef GL_DEPTH_TEST
#define GL_DEPTH_TEST 0x0B71
#endif
#ifndef GL_STENCIL_TEST
#define GL_STENCIL_TEST 0x0B90
#endif
#ifndef GL_BLEND
#define GL_BLEND 0x0BE2
#endif
#ifndef GL_CULL_FACE
#define GL_CULL_FACE 0x0B44
#endif
#ifndef GL_RASTERIZER_DISCARD
#define GL_RASTERIZER_DISCARD 0x8C89
#endif
#ifndef GL_POLYGON_OFFSET_FILL
#define GL_POLYGON_OFFSET_FILL 0x8037
#endif
#ifndef GL_CLIP_DISTANCE0
#define GL_CLIP_DISTANCE0 0x3000
#endif
#ifndef GL_TEXTURE_2D_MULTISAMPLE
#define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#endif
#ifndef GL_ALWAYS
#define GL_ALWAYS 0x0207
#endif
#ifndef GL_FRONT_AND_BACK
#define GL_FRONT_AND_BACK 0x0408
#endif
#ifndef GL_FILL
#define GL_FILL 0x1B02
#endif
#ifndef GL_TRIANGLES
#define GL_TRIANGLES 0x0004
#endif
#ifndef GL_DEPTH_BUFFER_BIT
#define GL_DEPTH_BUFFER_BIT 0x00000100
#endif
#ifndef GL_STENCIL_BUFFER_BIT
#define GL_STENCIL_BUFFER_BIT 0x00000400
#endif
static volatile LONG g_ms_tex[65536 / 32];      // names of multisample textures
static void ms_tex_mark(GLuint t) { if (t && t < 65536) InterlockedOr(&g_ms_tex[t >> 5], (LONG)(1u << (t & 31))); }
static int  ms_tex_is(GLuint t)   { return t && t < 65536 && (g_ms_tex[t >> 5] & (LONG)(1u << (t & 31))); }

typedef void (WINAPI *PFN_TexImage2DMS)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
typedef void (WINAPI *PFN_TexImage3DMS)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
static PFN_TexImage2DMS p_glTexImage2DMultisample_app, p_glTexStorage2DMultisample_app;
static PFN_TexImage3DMS p_glTexImage3DMultisample_app, p_glTexStorage3DMultisample_app;
static void ms_mark_bound(GLenum target) { GLenum q = tex_binding_for(target); if (q) ms_tex_mark((GLuint)get_int(q)); }
static void WINAPI h_glTexImage2DMultisample(GLenum t, GLsizei s, GLenum f, GLsizei w, GLsizei h, GLboolean fx) { p_glTexImage2DMultisample_app(t, s, f, w, h, fx); ms_mark_bound(t); }
static void WINAPI h_glTexStorage2DMultisample(GLenum t, GLsizei s, GLenum f, GLsizei w, GLsizei h, GLboolean fx) { p_glTexStorage2DMultisample_app(t, s, f, w, h, fx); ms_mark_bound(t); }
static void WINAPI h_glTexImage3DMultisample(GLenum t, GLsizei s, GLenum f, GLsizei w, GLsizei h, GLsizei d, GLboolean fx) { p_glTexImage3DMultisample_app(t, s, f, w, h, d, fx); ms_mark_bound(t); }
static void WINAPI h_glTexStorage3DMultisample(GLenum t, GLsizei s, GLenum f, GLsizei w, GLsizei h, GLsizei d, GLboolean fx) { p_glTexStorage3DMultisample_app(t, s, f, w, h, d, fx); ms_mark_bound(t); }

typedef struct { HGLRC rc; GLuint prog, vao; GLint loc_m, loc_dz; int failed; } MsbCtx;
static MsbCtx g_msb[8];
static int g_msb_n;
static volatile LONG g_cnt_msdepth;

typedef GLuint (WINAPI *PFN_CreateShader)(GLenum);
typedef void (WINAPI *PFN_ShaderSource)(GLuint, GLsizei, const char *const *, const GLint *);
typedef void (WINAPI *PFN_CompileShader)(GLuint);
typedef GLuint (WINAPI *PFN_CreateProgram)(void);
typedef void (WINAPI *PFN_AttachShader)(GLuint, GLuint);
typedef void (WINAPI *PFN_LinkProgram)(GLuint);
typedef void (WINAPI *PFN_GetProgramiv)(GLuint, GLenum, GLint *);
typedef GLint (WINAPI *PFN_GetUniformLocation)(GLuint, const char *);
typedef void (WINAPI *PFN_UseProgram)(GLuint);
typedef void (WINAPI *PFN_Uniform1i)(GLint, GLint);
typedef void (WINAPI *PFN_Uniform4f)(GLint, float, float, float, float);
typedef void (WINAPI *PFN_GenVertexArrays)(GLsizei, GLuint *);
typedef void (WINAPI *PFN_ActiveTexture)(GLenum);
typedef void (WINAPI *PFN_BindSampler)(GLuint, GLuint);
typedef void (WINAPI *PFN_BindProgramPipeline)(GLuint);
typedef void (WINAPI *PFN_GetFBAttach2)(GLenum, GLenum, GLenum, GLint *);
typedef void (WINAPI *PFN_Enable2)(GLenum);
typedef GLboolean (WINAPI *PFN_IsEnabled2)(GLenum);
typedef void (WINAPI *PFN_Viewport2)(GLint, GLint, GLsizei, GLsizei);
typedef void (WINAPI *PFN_DepthFunc2)(GLenum);
typedef void (WINAPI *PFN_DepthMask2)(GLboolean);
typedef void (WINAPI *PFN_ColorMask2)(GLboolean, GLboolean, GLboolean, GLboolean);
typedef void (WINAPI *PFN_DepthRange2)(double, double);
typedef void (WINAPI *PFN_GetDoublev2)(GLenum, double *);
typedef void (WINAPI *PFN_GetBooleanv2)(GLenum, GLboolean *);
typedef void (WINAPI *PFN_PolygonMode2)(GLenum, GLenum);
typedef void (WINAPI *PFN_DrawArrays2)(GLenum, GLint, GLsizei);
typedef void (WINAPI *PFN_BlitFB)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);

static void *p_glBlitFramebuffer_unused, *p_glBlitFramebufferEXT_unused;
typedef HGLRC (WINAPI *PFN_wglGetCurrentContext_msb)(void);
static MsbCtx *msb_ctx(void)
{
    static PFN_wglGetCurrentContext_msb getcur;
    if (!getcur) getcur = (PFN_wglGetCurrentContext_msb)GetProcAddress(GetModuleHandleA("opengl32.dll"), "wglGetCurrentContext");
    HGLRC rc = getcur ? getcur() : NULL;
    for (int i = 0; i < g_msb_n; i++) if (g_msb[i].rc == rc) return &g_msb[i];
    if (g_msb_n >= 8) return NULL;
    MsbCtx *c = &g_msb[g_msb_n++];
    memset(c, 0, sizeof *c); c->rc = rc;
    return c;
}

static int msb_prepare(MsbCtx *c)
{
    if (c->prog) return 1;
    if (c->failed) return 0;
    REAL_RET(PFN_CreateShader, crs, "glCreateShader", 0);
    REAL_RET(PFN_ShaderSource, src, "glShaderSource", 0);
    REAL_RET(PFN_CompileShader, cmp, "glCompileShader", 0);
    REAL_RET(PFN_CreateProgram, crp, "glCreateProgram", 0);
    REAL_RET(PFN_AttachShader, att, "glAttachShader", 0);
    REAL_RET(PFN_LinkProgram, lnk, "glLinkProgram", 0);
    REAL_RET(PFN_GetProgramiv, gpi, "glGetProgramiv", 0);
    REAL_RET(PFN_GetUniformLocation, gul, "glGetUniformLocation", 0);
    REAL_RET(PFN_GenVertexArrays, gva, "glGenVertexArrays", 0);
    const char *vs = "#version 430\nvoid main(){ vec2 p = vec2(gl_VertexID == 1 ? 3.0 : -1.0, gl_VertexID == 2 ? 3.0 : -1.0); gl_Position = vec4(p, 0.0, 1.0); }\n";
    const char *fs = "#version 430\nuniform sampler2DMS dz; uniform vec4 m;\nvoid main(){ ivec2 c = ivec2(int(gl_FragCoord.x * m.x + m.y), int(gl_FragCoord.y * m.z + m.w)); gl_FragDepth = texelFetch(dz, c, 0).r; }\n";
    GLuint v = crs(0x8B31), f = crs(0x8B30);
    src(v, 1, &vs, NULL); cmp(v); src(f, 1, &fs, NULL); cmp(f);
    GLuint p = crp(); att(p, v); att(p, f); lnk(p);
    GLint ok = 0; gpi(p, 0x8B82, &ok);
    if (!ok) { c->failed = 1; log_line("MSDEPTH: resolve program failed to link", NULL); return 0; }
    c->prog = p; c->loc_dz = gul(p, "dz"); c->loc_m = gul(p, "m");
    gva(1, &c->vao);
    log_fmt("MSDEPTH: resolve program %u ready on context %p", p, (void *)c->rc);
    return 1;
}

// Returns 1 when the depth part of the blit was handled here.
static int msdepth_blit(GLint sx0, GLint sy0, GLint sx1, GLint sy1, GLint dx0, GLint dy0, GLint dx1, GLint dy1)
{
    GLint rfbo = get_int(GL_READ_FRAMEBUFFER_BINDING), dfbo = get_int(GL_DRAW_FRAMEBUFFER_BINDING);
    if (!rfbo) return 0;
    REAL_RET(PFN_GetFBAttach2, getatt, "glGetFramebufferAttachmentParameteriv", 0);
    GLint ty = 0, name = 0;
    getatt(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &ty);
    if (ty != GL_TEXTURE) return 0;
    getatt(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);
    if (!ms_tex_is((GLuint)name)) return 0;
    GLint dty = 0;
    if (dfbo) { getatt(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &dty); if (!dty) return 1; }
    MsbCtx *c = msb_ctx();
    if (!c || !msb_prepare(c)) return 0;
    REAL_RET(PFN_UseProgram, usep, "glUseProgram", 0);
    REAL_RET(PFN_BindVertexArray, bva, "glBindVertexArray", 0);
    REAL_RET(PFN_ActiveTexture, actt, "glActiveTexture", 0);
    REAL_RET(PFN_BindSampler, bsamp, "glBindSampler", 0);
    REAL_RET(PFN_BindProgramPipeline, bpipe, "glBindProgramPipeline", 0);
    REAL_RET(PFN_Uniform1i, u1i, "glUniform1i", 0);
    REAL_RET(PFN_Uniform4f, u4f, "glUniform4f", 0);
    PFN_Enable2 en = (PFN_Enable2)g_tbl[IDX_glEnable], dis = (PFN_Enable2)g_tbl[IDX_glDisable];
    PFN_IsEnabled2 ise = (PFN_IsEnabled2)g_tbl[IDX_glIsEnabled];
    PFN_Viewport2 vpf = (PFN_Viewport2)g_tbl[IDX_glViewport], scf = (PFN_Viewport2)g_tbl[IDX_glScissor];
    PFN_DepthFunc2 dfn = (PFN_DepthFunc2)g_tbl[IDX_glDepthFunc];
    PFN_DepthMask2 dmk = (PFN_DepthMask2)g_tbl[IDX_glDepthMask];
    PFN_ColorMask2 cmk = (PFN_ColorMask2)g_tbl[IDX_glColorMask];
    PFN_DepthRange2 drg = (PFN_DepthRange2)g_tbl[IDX_glDepthRange];
    PFN_GetDoublev2 gdv = (PFN_GetDoublev2)g_tbl[IDX_glGetDoublev];
    PFN_GetBooleanv2 gbv = (PFN_GetBooleanv2)g_tbl[IDX_glGetBooleanv];
    PFN_PolygonMode2 pmd = (PFN_PolygonMode2)g_tbl[IDX_glPolygonMode];
    PFN_DrawArrays2 dra = (PFN_DrawArrays2)g_tbl[IDX_glDrawArrays];
    PFN_glBindTexture btx = (PFN_glBindTexture)g_tbl[IDX_glBindTexture];
    // save state
    GLint prog = get_int(GL_CURRENT_PROGRAM), pipe = get_int(GL_PROGRAM_PIPELINE_BINDING), vao = get_int(GL_VERTEX_ARRAY_BINDING);
    GLint unit = get_int(0x84E0), vp[4], sc[4], dfunc = get_int(GL_DEPTH_FUNC), pm[2], samp0 = 0, tex0 = 0;
    ((PFN_glGetIntegerv)g_tbl[IDX_glGetIntegerv])(GL_VIEWPORT, vp);
    ((PFN_glGetIntegerv)g_tbl[IDX_glGetIntegerv])(GL_SCISSOR_BOX, sc);
    ((PFN_glGetIntegerv)g_tbl[IDX_glGetIntegerv])(GL_POLYGON_MODE, pm);
    GLboolean cm[4], dm = 0; gbv(GL_COLOR_WRITEMASK, cm); gbv(GL_DEPTH_WRITEMASK, &dm);
    double dr[2] = { 0, 1 }; gdv(0x0B70 /*DEPTH_RANGE*/, dr);
    static const GLenum caps[] = { GL_SCISSOR_TEST, GL_DEPTH_TEST, GL_STENCIL_TEST, GL_BLEND, GL_CULL_FACE, GL_RASTERIZER_DISCARD, GL_POLYGON_OFFSET_FILL, 0x864F /*DEPTH_CLAMP*/, 0x809D /*MULTISAMPLE*/, 0x8E51 /*SAMPLE_MASK*/, 0x809E /*SAMPLE_ALPHA_TO_COVERAGE*/,
                                   GL_CLIP_DISTANCE0, GL_CLIP_DISTANCE0 + 1, GL_CLIP_DISTANCE0 + 2, GL_CLIP_DISTANCE0 + 3, GL_CLIP_DISTANCE0 + 4, GL_CLIP_DISTANCE0 + 5, GL_CLIP_DISTANCE0 + 6, GL_CLIP_DISTANCE0 + 7 };
    GLboolean was[sizeof caps / sizeof caps[0]];
    for (size_t i = 0; i < sizeof caps / sizeof caps[0]; i++) was[i] = ise(caps[i]);
    actt(0x84C0); tex0 = get_int(0x9104 /*TEXTURE_BINDING_2D_MULTISAMPLE*/); samp0 = get_int(0x8919 /*SAMPLER_BINDING*/);
    // set up
    if (pipe) bpipe(0);
    usep(c->prog); bva(c->vao);
    btx(GL_TEXTURE_2D_MULTISAMPLE, (GLuint)name); bsamp(0, 0);
    GLint ddx0 = dx0 < dx1 ? dx0 : dx1, ddy0 = dy0 < dy1 ? dy0 : dy1, dw = dx1 > dx0 ? dx1 - dx0 : dx0 - dx1, dh = dy1 > dy0 ? dy1 - dy0 : dy0 - dy1;
    float ax = dw ? (float)(sx1 - sx0) / (float)(dx1 - dx0) : 1.0f, ay = dh ? (float)(sy1 - sy0) / (float)(dy1 - dy0) : 1.0f;
    float bx = (float)sx0 - (float)dx0 * ax, by = (float)sy0 - (float)dy0 * ay;
    u1i(c->loc_dz, 0); u4f(c->loc_m, ax, bx, ay, by);
    vpf(ddx0, ddy0, dw, dh); en(GL_SCISSOR_TEST); scf(ddx0, ddy0, dw, dh);
    en(GL_DEPTH_TEST); dfn(GL_ALWAYS); dmk(GL_TRUE); cmk(0, 0, 0, 0); drg(0.0, 1.0); pmd(GL_FRONT_AND_BACK, GL_FILL);
    for (size_t i = 2; i < sizeof caps / sizeof caps[0]; i++) if (caps[i] != 0x809D) dis(caps[i]);
    dra(GL_TRIANGLES, 0, 3);
    InterlockedIncrement(&g_cnt_msdepth);
    // restore
    for (size_t i = 0; i < sizeof caps / sizeof caps[0]; i++) { if (was[i]) en(caps[i]); else dis(caps[i]); }
    vpf(vp[0], vp[1], vp[2], vp[3]); scf(sc[0], sc[1], sc[2], sc[3]);
    dfn((GLenum)dfunc); dmk(dm); cmk(cm[0], cm[1], cm[2], cm[3]); drg(dr[0], dr[1]); pmd(GL_FRONT_AND_BACK, (GLenum)pm[0]);
    btx(GL_TEXTURE_2D_MULTISAMPLE, (GLuint)tex0); bsamp(0, (GLuint)samp0); actt((GLenum)unit);
    bva((GLuint)vao); usep((GLuint)prog); if (pipe) bpipe((GLuint)pipe);
    return 1;
}

static void WINAPI h_glBlitFramebuffer(GLint sx0, GLint sy0, GLint sx1, GLint sy1, GLint dx0, GLint dy0, GLint dx1, GLint dy1, GLbitfield mask, GLenum filter)
{
    REAL(PFN_BlitFB, real_blit, "glBlitFramebuffer");
    if ((mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) && msdepth_blit(sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1)) {
        mask &= ~(GLbitfield)(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        if (!mask) return;
    }
    real_blit(sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, mask, filter);
}

#include "capture.c"

// ring of the last calls (debug mode, always on) + "follow" logging of the next calls after an event
#define TR_RING 192
typedef struct { const char *fn; unsigned a[4]; DWORD tid; } TrRec;
static TrRec g_tr_ring[TR_RING];
static volatile LONG g_tr_ring_n;
static volatile LONG g_tr_follow;
static void trace_dump_ring(const char *why, int follow)
{
    LONG n = g_tr_ring_n;
    log_fmt("---- last %d GL calls before %s:", TR_RING, why);
    for (LONG i = n - TR_RING; i < n; i++) {
        if (i < 0) continue;
        const TrRec *r = &g_tr_ring[i % TR_RING];
        if (r->fn) log_fmt("  %s(%x,%x,%x,%x) t=%u", r->fn, r->a[0], r->a[1], r->a[2], r->a[3], (unsigned)r->tid);
    }
    log_fmt("---- end; following the next %d calls", follow);
    InterlockedExchange(&g_tr_follow, follow);
}
static void trace_line(const char *fn, const ULONG_PTR *a)
{
    LONG n = InterlockedIncrement(&g_tr_ring_n) - 1;
    TrRec *r = &g_tr_ring[n % TR_RING];
    r->fn = fn; r->a[0] = (unsigned)a[3]; r->a[1] = (unsigned)a[2]; r->a[2] = (unsigned)a[1]; r->a[3] = (unsigned)a[0]; r->tid = GetCurrentThreadId();
    if (g_tr_follow > 0 && InterlockedDecrement(&g_tr_follow) >= 0)
        log_fmt("  > %s(%x,%x,%x,%x) t=%u", fn, r->a[0], r->a[1], r->a[2], r->a[3], (unsigned)r->tid);
    if (!g_cap_active || !g_tog.calls) return;
    if (InterlockedIncrement(&g_tr_lines) > 80000) return;
    char l[240];
    // a[3]=rcx a[2]=rdx a[1]=r8 a[0]=r9 (low 32 bits each)
    wsprintfA(l, "C t=%u %s(%x,%x,%x,%x)", (unsigned)GetCurrentThreadId(), fn, (unsigned)a[3], (unsigned)a[2], (unsigned)a[1], (unsigned)a[0]);
    cap_append(l);
}
void shim_trace_call(unsigned k, const ULONG_PTR *a)
{
    if (k >= TR_COUNT || !g_tr_names[k]) return;
    trace_line(g_tr_names[k], a);
}
void shim_trace_export(unsigned k, const ULONG_PTR *a)
{
    if (k >= SHIM_COUNT) return;
    trace_line(g_names[k], a);
}

// ---------------------------------------------------------------------------
// shader diagnostics: log compile/link failures with the driver's message
// ---------------------------------------------------------------------------
#define GL_COMPILE_STATUS   0x8B81
#define GL_LINK_STATUS      0x8B82
#define GL_INFO_LOG_LENGTH  0x8B84
#define GL_SHADER_TYPE      0x8B4F
#define GL_SHADER_SOURCE_LENGTH 0x8B88

typedef void (WINAPI *PFN_CompileShader)(GLuint);
typedef void (WINAPI *PFN_LinkProgram)(GLuint);
typedef void (WINAPI *PFN_GetShaderiv)(GLuint, GLenum, GLint *);
typedef void (WINAPI *PFN_GetInfoLog)(GLuint, GLsizei, GLsizei *, char *);
static PFN_CompileShader p_glCompileShader;
static PFN_LinkProgram p_glLinkProgram;
static volatile LONG g_shader_fail_logs, g_shader_ok, g_shader_fail, g_link_fail;

static void dump_source_head(GLuint sh)
{
    static PFN_GetInfoLog getsrc;
    if (!getsrc) getsrc = (PFN_GetInfoLog)real_proc("glGetShaderSource");
    if (!getsrc) return;
    char *buf = (char *)HeapAlloc(GetProcessHeap(), 0, 200001);
    if (!buf) return;
    GLsizei n = 0;
    getsrc(sh, 200000, &n, buf);
    buf[n > 0 ? n : 0] = 0;
    // log the first lines (version/extension header) and save the full source
    char head[700];
    lstrcpynA(head, buf, sizeof head);
    log_line("  SOURCE HEAD: ", head);
    char path[MAX_PATH];
    DWORD len = GetEnvironmentVariableA("LOCALAPPDATA", path, MAX_PATH - 60);
    if (len && len < MAX_PATH - 60) {
        char name[48];
        wsprintfA(name, "\\GLAdapter\\failed_shader_%u.glsl", (unsigned)sh);
        lstrcatA(path, name);
        HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            DWORD w;
            WriteFile(h, buf, (DWORD)lstrlenA(buf), &w, NULL);
            CloseHandle(h);
        }
    }
    HeapFree(GetProcessHeap(), 0, buf);
}

static void WINAPI h_glCompileShader(GLuint sh)
{
    p_glCompileShader(sh);
    REAL(PFN_GetShaderiv, getiv, "glGetShaderiv");
    REAL(PFN_GetInfoLog, getlog, "glGetShaderInfoLog");
    GLint ok = 1, type = 0;
    getiv(sh, GL_COMPILE_STATUS, &ok);
    if (ok) {
        InterlockedIncrement(&g_shader_ok);
        return;
    }
    InterlockedIncrement(&g_shader_fail);
    if (InterlockedIncrement(&g_shader_fail_logs) > 25)
        return;
    getiv(sh, GL_SHADER_TYPE, &type);
    char msg[1600];
    GLsizei n = 0;
    getlog(sh, sizeof msg - 1, &n, msg);
    msg[n > 0 ? n : 0] = 0;
    log_fmt("SHADER COMPILE FAILED: shader %u type 0x%x", sh, type);
    log_line("  LOG: ", msg);
    dump_source_head(sh);
}

static void WINAPI h_glLinkProgram(GLuint prog)
{
    p_glLinkProgram(prog);
    REAL(PFN_GetShaderiv, getiv, "glGetProgramiv");
    REAL(PFN_GetInfoLog, getlog, "glGetProgramInfoLog");
    GLint ok = 1;
    getiv(prog, GL_LINK_STATUS, &ok);
    if (ok)
        return;
    InterlockedIncrement(&g_link_fail);
    if (InterlockedIncrement(&g_shader_fail_logs) > 25)
        return;
    char msg[1600];
    GLsizei n = 0;
    getlog(prog, sizeof msg - 1, &n, msg);
    msg[n > 0 ? n : 0] = 0;
    log_fmt("PROGRAM LINK FAILED: program %u", prog);
    log_line("  LOG: ", msg);
}

// ---------------------------------------------------------------------------
// KHR_debug: log the driver's error / undefined-behaviour messages
// ---------------------------------------------------------------------------
#define GL_DEBUG_OUTPUT              0x92E0
#define GL_DEBUG_TYPE_ERROR          0x824C
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B

typedef void (WINAPI *DEBUGPROC)(GLenum, GLenum, GLuint, GLenum, GLsizei, const char *, const void *);
typedef void (WINAPI *PFN_DebugMessageCallback)(DEBUGPROC, const void *);
static PFN_DebugMessageCallback p_glDebugMessageCallback;
static DEBUGPROC g_app_cb;
static const void *g_app_cb_param;
static volatile LONG g_dbg_logged;

static void WINAPI dbg_cb(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                          const char *message, const void *param)
{
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION && message) {
        // de-duplicate by message hash (Parallels uses id 0 for everything), count repeats
        static struct { DWORD hash; LONG count; } seen[1024];
        static volatile LONG nseen;
        DWORD h = 2166136261u;
        for (const char *c = message; *c; c++)
            h = (h ^ (unsigned char)*c) * 16777619u;
        LONG n = nseen, hit = -1;
        for (LONG i = 0; i < n && i < 1024; i++)
            if (seen[i].hash == h) { hit = i; break; }
        if (hit >= 0) {
            LONG c = InterlockedIncrement(&seen[hit].count);
            if (c == 10 || c == 100 || c == 1000 || c == 10000)
                log_fmt("GLDEBUG repeat x%d: %s", (int)c, message);
        } else if (InterlockedIncrement(&g_dbg_logged) <= 400) {
            LONG k = InterlockedIncrement(&nseen) - 1;
            if (k < 1024) { seen[k].hash = h; seen[k].count = 1; }
            char head[128];
            wsprintfA(head, "GLDEBUG type=0x%x sev=0x%x thread=%u: ", type, severity, (unsigned)GetCurrentThreadId());
            log_line(head, message);
        }
    }
    if (g_app_cb)
        g_app_cb(source, type, id, severity, length, message, g_app_cb_param);
    (void)param;
}

static void WINAPI h_glDebugMessageCallback(DEBUGPROC cb, const void *param)
{
    g_app_cb = cb;
    g_app_cb_param = param;
    p_glDebugMessageCallback(dbg_cb, NULL);
}

// ---------------------------------------------------------------------------
// 3DEXPERIENCE warns "No certified OpenGL library has been found" whenever the
// first opengl32.dll in the DLL search order isn't a Microsoft/Dassault build,
// which is always true for this adapter. Confirm exactly that box with OK.
// ---------------------------------------------------------------------------
static BOOL CALLBACK find_text_child(HWND child, LPARAM lp)
{
    char text[64];
    if (GetWindowTextA(child, text, sizeof text) > 0 && strncmp(text, "No certified OpenGL library", 27) == 0) {
        *(BOOL *)lp = TRUE;
        return FALSE;
    }
    return TRUE;
}

static BOOL CALLBACK find_warning(HWND w, LPARAM lp)
{
    DWORD pid = 0;
    char cls[32];
    GetWindowThreadProcessId(w, &pid);
    if (pid != GetCurrentProcessId() || !IsWindowVisible(w))
        return TRUE;
    if (GetClassNameA(w, cls, sizeof cls) == 0 || strcmp(cls, "Warning") != 0)
        return TRUE;
    BOOL match = FALSE;
    EnumChildWindows(w, find_text_child, (LPARAM)&match);
    if (match) {
        PostMessageA(w, WM_COMMAND, IDOK, 0);
        *(int *)lp = 1;
        return FALSE;
    }
    return TRUE;
}

static DWORD WINAPI warning_watcher(LPVOID unused)
{
    (void)unused;
    for (int i = 0; i < 2400; i++) {          // up to 10 minutes after the first context
        int done = 0;
        EnumWindows(find_warning, (LPARAM)&done);
        if (done) {
            log_line("Dismissed the 'No certified OpenGL library' warning", NULL);
            return 0;
        }
        Sleep(250);
    }
    return 0;
}

typedef BOOL (WINAPI *PFN_wglMakeCurrent)(HDC, HGLRC);
typedef void (WINAPI *PFN_glEnable)(GLenum);
BOOL WINAPI wglMakeCurrent(HDC dc, HGLRC rc)
{
    shim_init();
    BOOL ok = ((PFN_wglMakeCurrent)g_tbl[IDX_wglMakeCurrent])(dc, rc);
    if (ok && rc) {
        static LONG watcher;
        if (!InterlockedExchange(&watcher, 1)) {
            HANDLE t = CreateThread(NULL, 0, warning_watcher, NULL, 0, NULL);
            if (t)
                CloseHandle(t);
        }
        if (!g_debug || !g_gldebug)
            return ok;
        static HGLRC seen[64];
        static volatile LONG nseen;
        LONG n = nseen;
        for (LONG i = 0; i < n && i < 64; i++)
            if (seen[i] == rc)
                return ok;
        LONG k = InterlockedIncrement(&nseen) - 1;
        if (k < 64)
            seen[k] = rc;
        if (!p_glDebugMessageCallback)
            p_glDebugMessageCallback = (PFN_DebugMessageCallback)real_proc("glDebugMessageCallback");
        if (!p_glDebugMessageCallback)
            p_glDebugMessageCallback = (PFN_DebugMessageCallback)real_proc("glDebugMessageCallbackARB");
        if (p_glDebugMessageCallback) {
            ((PFN_glEnable)g_tbl[IDX_glEnable])(GL_DEBUG_OUTPUT);
            ((PFN_glEnable)g_tbl[IDX_glEnable])(0x8242 /*GL_DEBUG_OUTPUT_SYNCHRONOUS*/);
            p_glDebugMessageCallback(dbg_cb, NULL);
            log_fmt("DEBUG OUTPUT enabled on context %p", (void *)rc);
        }
    }
    return ok;
}

// periodic statistics in the log
static void maybe_log_stats(void)
{
    static DWORD last;
    DWORD now = GetTickCount();
    if (now - last < 10000)
        return;
    last = now;
    log_fmt("STATS: live=%d readers=%d arena=%u MB uploads=%u (%u MB) downloads=%u shaders ok=%d failed=%d linkfail=%d",
            (int)g_pb_live, (int)g_pb_readers, (unsigned)(g_arena_next * PAGE >> 20),
            (unsigned)g_stat_uploads, (unsigned)(g_stat_upload_bytes >> 20), (unsigned)g_stat_downloads,
            (int)g_shader_ok, (int)g_shader_fail, (int)g_link_fail);
    log_fmt("SYNC: fences=%d waitOK=%d waitOther=%d getsyncSignaled=%d mapPersist=%d mapOther=%d swaps=%d mdiSplit=%d (%d cmds)",
            (int)g_cnt_fence, (int)g_cnt_wait_ok, (int)g_cnt_wait_other, (int)g_cnt_getsync_sig,
            (int)g_cnt_map_persist, (int)g_cnt_map_other, (int)g_cnt_swaps, (int)g_cnt_mdi_split, (int)g_cnt_mdi_cmds);
    if (g_debug)
        log_counters();
}

// ---------------------------------------------------------------------------
// wglGetProcAddress: route everything above
// ---------------------------------------------------------------------------
static const struct { const char *name; PROC fn; } g_fallbacks[] = {
    { "glClearNamedBufferDataEXT",            (PROC)e_ClearNamedBufferDataEXT },
    { "glClearNamedBufferSubDataEXT",         (PROC)e_ClearNamedBufferSubDataEXT },
    { "glGetNamedFramebufferParameterivEXT",  (PROC)e_GetNamedFramebufferParameterivEXT },
    { "glNamedFramebufferParameteriEXT",      (PROC)e_NamedFramebufferParameteriEXT },
    { "glTextureBufferRangeEXT",              (PROC)e_TextureBufferRangeEXT },
    { "glTextureStorage2DMultisampleEXT",     (PROC)e_TextureStorage2DMultisampleEXT },
    { "glTextureStorage3DMultisampleEXT",     (PROC)e_TextureStorage3DMultisampleEXT },
    { "glVertexArrayBindVertexBufferEXT",     (PROC)e_VertexArrayBindVertexBufferEXT },
    { "glVertexArrayVertexAttribBindingEXT",  (PROC)e_VertexArrayVertexAttribBindingEXT },
    { "glVertexArrayVertexAttribFormatEXT",   (PROC)e_VertexArrayVertexAttribFormatEXT },
    { "glVertexArrayVertexAttribIFormatEXT",  (PROC)e_VertexArrayVertexAttribIFormatEXT },
    { "glVertexArrayVertexAttribLFormatEXT",  (PROC)e_VertexArrayVertexAttribLFormatEXT },
    { "glVertexArrayVertexBindingDivisorEXT", (PROC)e_VertexArrayVertexBindingDivisorEXT },
    { "glBufferStorage",                      (PROC)e_BufferStorage },
    { "glNamedBufferStorageEXT",              (PROC)e_NamedBufferStorageEXT },
};

// Functions the driver has but that we wrap: name, real-pointer slot, wrapper,
// and whether the wrapper is a diagnostic used only with SHIM_DEBUG=1.
static const struct { const char *name; void **real; PROC fn; int debug_only; } g_wraps[] = {
    { "glGetStringi",             (void **)&p_glGetStringi,             (PROC)h_glGetStringi, 0 },
    { "glTexBufferRange",         (void **)&p_glTexBufferRange,         (PROC)h_glTexBufferRange, 0 },
    { "glBlitFramebuffer",        (void **)&p_glBlitFramebuffer_unused, (PROC)h_glBlitFramebuffer, 0 },
    { "glBlitFramebufferEXT",     (void **)&p_glBlitFramebufferEXT_unused, (PROC)h_glBlitFramebuffer, 0 },
    { "glGetBufferParameteriv",   (void **)&p_glGetBufferParameteriv_app, (PROC)h_glGetBufferParameteriv, 0 },
    { "glGetBufferParameterivARB", (void **)&p_glGetBufferParameterivARB_app, (PROC)h_glGetBufferParameterivARB, 0 },
    { "glGetBufferParameteri64v", (void **)&p_glGetBufferParameteri64v_app, (PROC)h_glGetBufferParameteri64v, 0 },
    { "glGetBufferSubData",       (void **)&p_glGetBufferSubData_app,   (PROC)h_glGetBufferSubData, 0 },
    { "glBindBuffer",             (void **)&p_glBindBuffer_app,         (PROC)h_glBindBuffer, 0 },
    { "glBindBufferARB",          (void **)&p_glBindBufferARB_app,      (PROC)h_glBindBufferARB, 0 },
    { "glBindBufferBase",         (void **)&p_glBindBufferBase_app,     (PROC)h_glBindBufferBase, 0 },
    { "glBindBufferBaseEXT",      (void **)&p_glBindBufferBaseEXT_app,  (PROC)h_glBindBufferBaseEXT, 0 },
    { "glBindBufferRange",        (void **)&p_glBindBufferRange_app,    (PROC)h_glBindBufferRange, 0 },
    { "glBindBufferRangeEXT",     (void **)&p_glBindBufferRangeEXT_app, (PROC)h_glBindBufferRangeEXT, 0 },
    { "glTexImage2DMultisample",  (void **)&p_glTexImage2DMultisample_app,   (PROC)h_glTexImage2DMultisample, 0 },
    { "glTexStorage2DMultisample",(void **)&p_glTexStorage2DMultisample_app, (PROC)h_glTexStorage2DMultisample, 0 },
    { "glTexImage3DMultisample",  (void **)&p_glTexImage3DMultisample_app,   (PROC)h_glTexImage3DMultisample, 0 },
    { "glTexStorage3DMultisample",(void **)&p_glTexStorage3DMultisample_app, (PROC)h_glTexStorage3DMultisample, 0 },
    { "glCopyBufferSubData",      (void **)&p_glCopyBufferSubData_app,  (PROC)h_glCopyBufferSubData, 0 },
    { "glMemoryBarrier",          (void **)&p_glMemoryBarrier_app,      (PROC)h_glMemoryBarrier, 0 },
    { "glDispatchCompute",        (void **)&p_glDispatchCompute_app,    (PROC)h_glDispatchCompute, 0 },
    { "glDispatchComputeIndirect",(void **)&p_glDispatchComputeIndirect_app, (PROC)h_glDispatchComputeIndirect, 0 },
    { "glClearBufferSubData",     (void **)&p_glClearBufferSubData_app, (PROC)h_glClearBufferSubData, 0 },
    { "glClearBufferData",        (void **)&p_glClearBufferData_app,    (PROC)h_glClearBufferData, 0 },
    { "glBufferData",             (void **)&p_glBufferData_app,         (PROC)h_glBufferData, 0 },
    { "glBindVertexBuffer",       (void **)&p_glBindVertexBuffer,       (PROC)h_glBindVertexBuffer, 0 },
    { "glTexBuffer",              (void **)&p_glTexBuffer,              (PROC)h_glTexBuffer, 0 },
    { "glTexBufferARB",           (void **)&p_glTexBufferARB,           (PROC)h_glTexBufferARB, 0 },
    { "glTexBufferEXT",           (void **)&p_glTexBufferEXT,           (PROC)h_glTexBufferEXT, 0 },
    { "glInvalidateBufferData",   (void **)&p_glInvalidateBufferData,   (PROC)h_glInvalidateBufferData, 0 },
    { "glInvalidateBufferSubData",(void **)&p_glInvalidateBufferSubData,(PROC)h_glInvalidateBufferSubData, 0 },
    { "glMapBufferRange",         (void **)&p_glMapBufferRange,         (PROC)h_glMapBufferRange, 0 },
    { "glMapBuffer",              (void **)&p_glMapBuffer,              (PROC)h_glMapBuffer, 0 },
    { "glUnmapBuffer",            (void **)&p_glUnmapBuffer,            (PROC)h_glUnmapBuffer, 0 },
    { "glFlushMappedBufferRange", (void **)&p_glFlushMappedBufferRange, (PROC)h_glFlushMappedBufferRange, 0 },
    { "glDeleteBuffers",          (void **)&p_glDeleteBuffers,          (PROC)h_glDeleteBuffers, 0 },
    { "glBufferSubData",          (void **)&p_glBufferSubData_app,      (PROC)h_glBufferSubData, 0 },
    { "glClientWaitSync",         (void **)&p_glClientWaitSync,         (PROC)h_glClientWaitSync, 0 },
    { "glGetSynciv",              (void **)&p_glGetSynciv,              (PROC)h_glGetSynciv, 0 },
    { "glFenceSync",              (void **)&p_glFenceSync,              (PROC)h_glFenceSync, 0 },
    { "glMultiDrawElementsIndirect", (void **)&p_glMultiDrawElementsIndirect, (PROC)h_glMultiDrawElementsIndirect, 0 },
    { "glMultiDrawArraysIndirect",   (void **)&p_glMultiDrawArraysIndirect,   (PROC)h_glMultiDrawArraysIndirect, 0 },
    { "glCompileShader",          (void **)&p_glCompileShader,          (PROC)h_glCompileShader, 1 },
    { "glLinkProgram",            (void **)&p_glLinkProgram,            (PROC)h_glLinkProgram, 1 },
    { "glDebugMessageCallback",   (void **)&p_glDebugMessageCallback,   (PROC)h_glDebugMessageCallback, 1 },
    CAP_WRAPS,
};

static PROC resolve_proc(LPCSTR name);
PROC WINAPI wglGetProcAddress(LPCSTR name)
{
    shim_init();
    if (!name)
        return NULL;
    PROC r = resolve_proc(name);
    if (!r || !g_debug)
        return r;
    // hand the app a trace stub that records the call (when tracing is on) and jumps to r
    EnterCriticalSection(&g_log_cs);
    LONG n = g_tr_n;
    int k = -1;
    for (LONG i = 0; i < n; i++)
        if (strcmp(g_tr_names[i], name) == 0 && g_tr_real[i] == (void *)r) { k = (int)i; break; }
    if (k < 0 && n < TR_COUNT) {
        size_t len = strlen(name) + 1;
        char *copy = (char *)HeapAlloc(GetProcessHeap(), 0, len);
        if (copy) {
            memcpy(copy, name, len);
            g_tr_names[n] = copy;
            g_tr_real[n] = (void *)r;
            k = (int)n;
            g_tr_n = n + 1;
        }
    }
    LeaveCriticalSection(&g_log_cs);
    return k >= 0 ? (PROC)g_tr_stubs[k] : r;
}

static PROC resolve_proc(LPCSTR name)
{
    maybe_log_stats();
    if (strcmp(name, "glGetString") == 0)
        return (PROC)glGetString;
    if (strcmp(name, "glGetIntegerv") == 0)
        return (PROC)glGetIntegerv;
    PROC p = real_proc(name);
    if (g_debug) {
        // log each distinct requested name once
        static DWORD seen[4096];
        static volatile LONG nseen;
        DWORD h = 2166136261u;
        for (const char *c = name; *c; c++)
            h = (h ^ (unsigned char)*c) * 16777619u;
        int dup = 0;
        for (LONG i = 0; i < nseen && i < 4096; i++)
            if (seen[i] == h) { dup = 1; break; }
        if (!dup) {
            LONG k = InterlockedIncrement(&nseen) - 1;
            if (k < 4096)
                seen[k] = h;
            log_line(p ? "REQ have: " : "REQ none: ", name);
        }
    }
    // ARB aliases of buffer functions must hit the same wrappers as the core names
    const char *lookup = name;
    char core[64];
    size_t len = strlen(name);
    if (p && len > 3 && len < sizeof core && strcmp(name + len - 3, "ARB") == 0) {
        static const char *const aliased[] = { "glMapBuffer", "glUnmapBuffer", "glBufferSubData",
                                               "glDeleteBuffers", "glBufferData", "glGetBufferSubData" };
        memcpy(core, name, len - 3);
        core[len - 3] = 0;
        for (size_t i = 0; i < sizeof aliased / sizeof aliased[0]; i++)
            if (strcmp(core, aliased[i]) == 0)
                lookup = core;
    }
    if (p) {
        // two passes: with SHIM_DEBUG the debug wrappers win, otherwise they are skipped
        for (int pass = 0; pass < 2; pass++)
            for (size_t i = 0; i < sizeof g_wraps / sizeof g_wraps[0]; i++)
                if (strcmp(lookup, g_wraps[i].name) == 0) {
                    if (g_wraps[i].debug_only != (pass == 0 ? 1 : 0))
                        continue;
                    if (g_wraps[i].debug_only && !g_debug)
                        continue;
                    *g_wraps[i].real = (void *)p;
                    return g_wraps[i].fn;
                }
        for (int k = 0; k < PH_COUNT; k++)
            if (strcmp(name, g_ph_names[k]) == 0) {
                g_ph_real[k] = (void *)p;
                return (PROC)g_ph_stubs[k];
            }
        return p;
    }
    // with GL_ARB_buffer_storage hidden (SHIM_HIDE_EXT) its entry points must be missing too
    if (ext_hidden("GL_ARB_buffer_storage") && (strcmp(name, "glBufferStorage") == 0 || strcmp(name, "glNamedBufferStorageEXT") == 0)) {
        log_line("MISSING (hidden ext): ", name);
        return NULL;
    }
    for (size_t i = 0; i < sizeof g_fallbacks / sizeof g_fallbacks[0]; i++)
        if (strcmp(name, g_fallbacks[i].name) == 0) {
            log_line("EMULATED: ", name);
            return g_fallbacks[i].fn;
        }
    log_line("MISSING: ", name);
    return NULL;
}

// SHIM_DEBUG=1: log illegal-instruction / access-violation exceptions with the faulting module,
// offset, instruction bytes and the module addresses found on the stack (first-chance; the
// application's own handlers still run).
static void veh_module(ULONG_PTR addr, char *out, int n)
{
    HMODULE m = NULL;
    char path[MAX_PATH] = "?";
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)addr, &m) && m) {
        GetModuleFileNameA(m, path, MAX_PATH);
        const char *b = path; for (const char *c = path; *c; c++) if (*c == '\\') b = c + 1;
        wsprintfA(out, "%s+0x%x", b, (unsigned)(addr - (ULONG_PTR)m));
    } else {
        wsprintfA(out, "%p(no module)", (void *)addr);
    }
    (void)n;
}
static LONG WINAPI shim_veh(EXCEPTION_POINTERS *ep)
{
    DWORD code = ep->ExceptionRecord->ExceptionCode;
    if (code != 0xC000001D && code != 0xC0000005 && code != 0xC0000096)
        return EXCEPTION_CONTINUE_SEARCH;
    static volatile LONG n;
    LONG k = InterlockedIncrement(&n);
    if (k > 12) return EXCEPTION_CONTINUE_SEARCH;
    ULONG_PTR ip = (ULONG_PTR)ep->ExceptionRecord->ExceptionAddress;
    char where[MAX_PATH + 32];
    veh_module(ip, where, sizeof where);
    char bytes[64] = "";
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((void *)ip, &mbi, sizeof mbi) && mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))) {
        const unsigned char *b = (const unsigned char *)ip;
        for (int i = 0; i < 16; i++) wsprintfA(bytes + i * 3, "%02x ", b[i]);
    }
    log_fmt("EXCEPTION 0x%08x at %s thread=%u bytes=[%s] protect=0x%x", (unsigned)code, where, (unsigned)GetCurrentThreadId(), bytes, (unsigned)mbi.Protect);
#ifdef _M_X64
    ULONG_PTR *sp = (ULONG_PTR *)ep->ContextRecord->Rsp;
    int shown = 0;
    for (int i = 0; i < 256 && shown < 14; i++) {
        ULONG_PTR v;
        if (!VirtualQuery(sp + i, &mbi, sizeof mbi) || mbi.State != MEM_COMMIT) break;
        v = sp[i];
        HMODULE m = NULL;
        if (v > 0x10000 && GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)v, &m) && m) {
            char w[MAX_PATH + 32]; veh_module(v, w, sizeof w);
            log_fmt("  stack[%d] %s", i, w);
            shown++;
        }
    }
#endif
    if (k <= 3)
        trace_dump_ring(code == 0xC0000005 ? "access violation" : "exception", 0);
    return EXCEPTION_CONTINUE_SEARCH;
}

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved)
{
    (void)inst; (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        InitializeCriticalSection(&g_log_cs);
        InitializeCriticalSection(&g_pb_cs);
        InitializeCriticalSection(&g_cap_cs);
        char v[8];
        DWORD n = GetEnvironmentVariableA("SHIM_DEBUG", v, sizeof v);
        g_debug = n > 0 && n < sizeof v && v[0] == '1';
        g_tr_on = (unsigned char)g_debug;   // call ring always recorded in debug mode
        if (g_debug)
            AddVectoredExceptionHandler(1, shim_veh);
        n = GetEnvironmentVariableA("SHIM_GLDEBUG", v, sizeof v);
        g_gldebug = n > 0 && n < sizeof v && v[0] == '1';
        char gv[16];
        n = GetEnvironmentVariableA("SHIM_GL_VERSION", gv, sizeof gv);
        if (n >= 3 && n < sizeof gv && gv[0] >= '1' && gv[0] <= '9' && gv[1] == '.' && gv[2] >= '0' && gv[2] <= '9') {
            g_spoof_major = gv[0] - '0';
            g_spoof_minor = gv[2] - '0';
        }
    }
    return TRUE;
}

// Rendering probe of Parallels' OpenGL driver: draws with the techniques the
// 3DEXPERIENCE OGL_43 rasterizer uses and checks the pixels that come out.
// A technique that fails silently (no GL error, wrong picture) shows as FAIL.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
typedef ptrdiff_t GLintptr, GLsizeiptr;
typedef char GLchar;
typedef void *GLsync;
typedef unsigned long long GLuint64;
typedef long long GLint64;

#define GL_VERSION 0x1F02
#define GL_RENDERER 0x1F01
#define GL_CONTEXT_PROFILE_MASK 0x9126
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_UNIFORM_BUFFER 0x8A11
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#define GL_DRAW_INDIRECT_BUFFER 0x8F3F
#define GL_ATOMIC_COUNTER_BUFFER 0x92C0
#define GL_TEXTURE_BUFFER 0x8C2A
#define GL_TRANSFORM_FEEDBACK_BUFFER 0x8C8E
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_COPY 0x88EA
#define GL_FLOAT 0x1406
#define GL_UNSIGNED_SHORT 0x1403
#define GL_UNSIGNED_INT 0x1405
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPUTE_SHADER 0x91B9
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_FRAMEBUFFER 0x8D40
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_RENDERBUFFER 0x8D41
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_RGBA8 0x8058
#define GL_RGBA16F 0x881A
#define GL_RGBA32F 0x8814
#define GL_R32F 0x822E
#define GL_R32UI 0x8236
#define GL_RED_INTEGER 0x8D94
#define GL_RGBA_INTEGER 0x8D99
#define GL_RGBA8UI 0x8D7C
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_DEPTH_COMPONENT32F 0x8CAC
#define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#define GL_TEXTURE_2D_ARRAY 0x8C1A
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_COMPARE_REF_TO_TEXTURE 0x884E
#define GL_SAMPLES_PASSED 0x8914
#define GL_ANY_SAMPLES_PASSED 0x8C2F
#define GL_QUERY_RESULT 0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_QUERY_WAIT 0x8E13
#define GL_QUERY_NO_WAIT 0x8E14
#define GL_PRIMITIVE_RESTART 0x8F9D
#define GL_PRIMITIVE_RESTART_FIXED_INDEX 0x8D69
#define GL_RASTERIZER_DISCARD 0x8C89
#define GL_INTERLEAVED_ATTRIBS 0x8C8C
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_SHADER_STORAGE_BARRIER_BIT 0x2000
#define GL_COMMAND_BARRIER_BIT 0x40
#define GL_TEXTURE_FETCH_BARRIER_BIT 0x8
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x20
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x1
#define GL_ATOMIC_COUNTER_BARRIER_BIT 0x1000
#define GL_ALL_BARRIER_BITS 0xFFFFFFFF
#define GL_VERTEX_SHADER_BIT 0x1
#define GL_FRAGMENT_SHADER_BIT 0x2
#define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#define GL_SYNC_FLUSH_COMMANDS_BIT 0x1
#define GL_TIMEOUT_IGNORED 0xFFFFFFFFFFFFFFFFull
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_DEPTH_CLAMP 0x864F
#define GL_MULTISAMPLE 0x809D

typedef HGLRC (WINAPI *PFN_CCA)(HDC, HGLRC, const int *);
#define GLFN(ret, name, args) typedef ret (WINAPI *PFN_##name) args; static PFN_##name name;
GLFN(void, glGenVertexArrays, (GLsizei, GLuint *))
GLFN(void, glBindVertexArray, (GLuint))
GLFN(void, glGenBuffers, (GLsizei, GLuint *))
GLFN(void, glBindBuffer, (GLenum, GLuint))
GLFN(void, glBindBufferBase, (GLenum, GLuint, GLuint))
GLFN(void, glBindBufferRange, (GLenum, GLuint, GLuint, GLintptr, GLsizeiptr))
GLFN(void, glBufferData, (GLenum, GLsizeiptr, const void *, GLenum))
GLFN(void, glBufferSubData, (GLenum, GLintptr, GLsizeiptr, const void *))
GLFN(void, glGetBufferSubData, (GLenum, GLintptr, GLsizeiptr, void *))
GLFN(void, glInvalidateBufferData, (GLuint))
GLFN(void, glInvalidateBufferSubData, (GLuint, GLintptr, GLsizeiptr))
GLFN(void, glCopyBufferSubData, (GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr))
GLFN(void, glClearBufferSubData, (GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void *))
GLFN(void *, glMapBuffer, (GLenum, GLenum))
GLFN(void, glBindVertexBuffer, (GLuint, GLuint, GLintptr, GLsizei))
GLFN(void, glVertexAttribFormat, (GLuint, GLint, GLenum, GLboolean, GLuint))
GLFN(void, glVertexAttribBinding, (GLuint, GLuint))
GLFN(void, glVertexBindingDivisor, (GLuint, GLuint))
GLFN(void, glEnableVertexAttribArray, (GLuint))
GLFN(void, glDisableVertexAttribArray, (GLuint))
GLFN(GLuint, glCreateShader, (GLenum))
GLFN(void, glShaderSource, (GLuint, GLsizei, const GLchar *const *, const GLint *))
GLFN(void, glCompileShader, (GLuint))
GLFN(void, glGetShaderiv, (GLuint, GLenum, GLint *))
GLFN(void, glGetShaderInfoLog, (GLuint, GLsizei, GLsizei *, GLchar *))
GLFN(GLuint, glCreateProgram, (void))
GLFN(void, glAttachShader, (GLuint, GLuint))
GLFN(void, glLinkProgram, (GLuint))
GLFN(void, glGetProgramiv, (GLuint, GLenum, GLint *))
GLFN(void, glGetProgramInfoLog, (GLuint, GLsizei, GLsizei *, GLchar *))
GLFN(void, glUseProgram, (GLuint))
GLFN(void, glDeleteProgram, (GLuint))
GLFN(void, glDeleteShader, (GLuint))
GLFN(GLint, glGetUniformLocation, (GLuint, const GLchar *))
GLFN(void, glUniform4f, (GLint, float, float, float, float))
GLFN(void, glUniform1i, (GLint, GLint))
GLFN(void, glUniform1f, (GLint, float))
GLFN(void, glProgramUniform4f, (GLuint, GLint, float, float, float, float))
GLFN(void, glProgramParameteri, (GLuint, GLenum, GLint))
GLFN(GLuint, glCreateShaderProgramv, (GLenum, GLsizei, const GLchar *const *))
GLFN(void, glGenProgramPipelines, (GLsizei, GLuint *))
GLFN(void, glBindProgramPipeline, (GLuint))
GLFN(void, glUseProgramStages, (GLuint, GLbitfield, GLuint))
GLFN(void, glValidateProgramPipeline, (GLuint))
GLFN(void, glGetProgramPipelineiv, (GLuint, GLenum, GLint *))
GLFN(void, glGenFramebuffers, (GLsizei, GLuint *))
GLFN(void, glBindFramebuffer, (GLenum, GLuint))
GLFN(void, glFramebufferTexture2D, (GLenum, GLenum, GLenum, GLuint, GLint))
GLFN(void, glFramebufferTexture, (GLenum, GLenum, GLuint, GLint))
GLFN(void, glFramebufferRenderbuffer, (GLenum, GLenum, GLenum, GLuint))
GLFN(GLenum, glCheckFramebufferStatus, (GLenum))
GLFN(void, glGenRenderbuffers, (GLsizei, GLuint *))
GLFN(void, glBindRenderbuffer, (GLenum, GLuint))
GLFN(void, glRenderbufferStorage, (GLenum, GLenum, GLsizei, GLsizei))
GLFN(void, glRenderbufferStorageMultisample, (GLenum, GLsizei, GLenum, GLsizei, GLsizei))
GLFN(void, glBlitFramebuffer, (GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum))
GLFN(void, glDrawBuffers, (GLsizei, const GLenum *))
GLFN(void, glClearBufferfv, (GLenum, GLint, const float *))
GLFN(void, glColorMaski, (GLuint, GLboolean, GLboolean, GLboolean, GLboolean))
GLFN(void, glTexStorage2D, (GLenum, GLsizei, GLenum, GLsizei, GLsizei))
GLFN(void, glTexStorage3D, (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei))
GLFN(void, glTexImage2DMultisample, (GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean))
GLFN(void, glTexSubImage3D, (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *))
GLFN(void, glTexBuffer, (GLenum, GLenum, GLuint))
GLFN(void, glActiveTexture, (GLenum))
GLFN(void, glBindImageTexture, (GLuint, GLuint, GLint, GLboolean, GLint, GLenum, GLenum))
GLFN(void, glDispatchCompute, (GLuint, GLuint, GLuint))
GLFN(void, glMemoryBarrier, (GLbitfield))
GLFN(void, glDrawArraysInstanced, (GLenum, GLint, GLsizei, GLsizei))
GLFN(void, glDrawArraysInstancedBaseInstance, (GLenum, GLint, GLsizei, GLsizei, GLuint))
GLFN(void, glDrawElementsBaseVertex, (GLenum, GLsizei, GLenum, const void *, GLint))
GLFN(void, glDrawElementsInstancedBaseVertexBaseInstance, (GLenum, GLsizei, GLenum, const void *, GLsizei, GLint, GLuint))
GLFN(void, glDrawElementsIndirect, (GLenum, GLenum, const void *))
GLFN(void, glDrawArraysIndirect, (GLenum, const void *))
GLFN(void, glMultiDrawElementsIndirect, (GLenum, GLenum, const void *, GLsizei, GLsizei))
GLFN(void, glMultiDrawArraysIndirect, (GLenum, const void *, GLsizei, GLsizei))
GLFN(void, glMultiDrawArrays, (GLenum, const GLint *, const GLsizei *, GLsizei))
GLFN(void, glMultiDrawElements, (GLenum, const GLsizei *, GLenum, const void *const *, GLsizei))
GLFN(void, glMultiDrawElementsBaseVertex, (GLenum, const GLsizei *, GLenum, const void *const *, GLsizei, const GLint *))
GLFN(void, glGenQueries, (GLsizei, GLuint *))
GLFN(void, glBeginQuery, (GLenum, GLuint))
GLFN(void, glEndQuery, (GLenum))
GLFN(void, glGetQueryObjectuiv, (GLuint, GLenum, GLuint *))
GLFN(void, glGetQueryObjectiv, (GLuint, GLenum, GLint *))
GLFN(void, glGetQueryObjectui64v, (GLuint, GLenum, unsigned long long *))
GLFN(const GLubyte *, glGetStringi, (GLenum, GLuint))
GLFN(void, glTexBufferRange, (GLenum, GLenum, GLuint, GLintptr, GLsizeiptr))
GLFN(void, glBufferStorage, (GLenum, GLsizeiptr, const void *, GLbitfield))
GLFN(void, glDeleteSync, (GLsync))
GLFN(void, glBeginConditionalRender, (GLuint, GLenum))
GLFN(void, glEndConditionalRender, (void))
GLFN(void, glPrimitiveRestartIndex, (GLuint))
GLFN(void, glTransformFeedbackVaryings, (GLuint, GLsizei, const GLchar *const *, GLenum))
GLFN(void, glBeginTransformFeedback, (GLenum))
GLFN(void, glEndTransformFeedback, (void))
GLFN(GLsync, glFenceSync, (GLenum, GLbitfield))
GLFN(void, glWaitSync, (GLsync, GLbitfield, GLuint64))
GLFN(GLenum, glClientWaitSync, (GLsync, GLbitfield, GLuint64))
GLFN(void *, glMapBufferRange, (GLenum, GLintptr, GLsizeiptr, GLbitfield))
GLFN(GLboolean, glUnmapBuffer, (GLenum))
GLFN(void, glGenSamplers, (GLsizei, GLuint *))
GLFN(void, glBindSampler, (GLuint, GLuint))
GLFN(void, glSamplerParameteri, (GLuint, GLenum, GLint))
GLFN(void, glDeleteFramebuffers, (GLsizei, const GLuint *))
GLFN(void, glDeleteBuffers, (GLsizei, const GLuint *))
GLFN(void, glDeleteVertexArrays, (GLsizei, const GLuint *))
GLFN(void, glClearDepthf, (float))
GLFN(void, glGetInteger64v, (GLenum, GLint64 *))
GLFN(void, glEnablei, (GLenum, GLuint))
GLFN(void, glDisablei, (GLenum, GLuint))
GLFN(void, glBlendFunci, (GLuint, GLenum, GLenum))
GLFN(void, glViewportIndexedf, (GLuint, float, float, float, float))
GLFN(void, glDrawRangeElements, (GLenum, GLuint, GLuint, GLsizei, GLenum, const void *))
GLFN(void, glCopyImageSubData, (GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei))
GLFN(void, glProvokingVertex, (GLenum))
GLFN(void, glVertexAttribIFormat, (GLuint, GLint, GLenum, GLuint))
GLFN(void, glGenerateMipmap, (GLenum))

static int g_missing;
static void load(void)
{
#define L(n) n = (PFN_##n)wglGetProcAddress(#n); if (!n) { printf("  missing %s\n", #n); g_missing++; }
    L(glGenVertexArrays) L(glBindVertexArray) L(glGenBuffers) L(glBindBuffer) L(glBindBufferBase) L(glBindBufferRange)
    L(glBufferData) L(glBufferSubData) L(glGetBufferSubData) L(glBindVertexBuffer) L(glVertexAttribFormat)
    L(glInvalidateBufferData) L(glInvalidateBufferSubData) L(glCopyBufferSubData) L(glClearBufferSubData) L(glMapBuffer)
    L(glGetQueryObjectiv) L(glGetQueryObjectui64v) L(glGetStringi) L(glDeleteSync) L(glTexBufferRange)
    glBufferStorage = (PFN_glBufferStorage)wglGetProcAddress("glBufferStorage");   // optional (Parallels lacks it; the adapter emulates it)
    L(glVertexAttribBinding) L(glVertexBindingDivisor) L(glEnableVertexAttribArray) L(glDisableVertexAttribArray) L(glCreateShader) L(glShaderSource)
    L(glCompileShader) L(glGetShaderiv) L(glGetShaderInfoLog) L(glCreateProgram) L(glAttachShader) L(glLinkProgram)
    L(glGetProgramiv) L(glGetProgramInfoLog) L(glUseProgram) L(glDeleteProgram) L(glDeleteShader) L(glGetUniformLocation)
    L(glUniform4f) L(glUniform1i) L(glUniform1f) L(glProgramUniform4f) L(glProgramParameteri) L(glCreateShaderProgramv)
    L(glGenProgramPipelines) L(glBindProgramPipeline) L(glUseProgramStages) L(glValidateProgramPipeline) L(glGetProgramPipelineiv)
    L(glGenFramebuffers) L(glBindFramebuffer) L(glFramebufferTexture2D) L(glFramebufferTexture) L(glFramebufferRenderbuffer)
    L(glCheckFramebufferStatus) L(glGenRenderbuffers) L(glBindRenderbuffer) L(glRenderbufferStorage)
    L(glRenderbufferStorageMultisample) L(glBlitFramebuffer) L(glDrawBuffers) L(glClearBufferfv) L(glColorMaski)
    L(glTexStorage2D) L(glTexStorage3D) L(glTexImage2DMultisample) L(glTexSubImage3D) L(glTexBuffer) L(glActiveTexture)
    L(glBindImageTexture) L(glDispatchCompute) L(glMemoryBarrier) L(glDrawArraysInstanced) L(glDrawArraysInstancedBaseInstance)
    L(glDrawElementsBaseVertex) L(glDrawElementsInstancedBaseVertexBaseInstance) L(glDrawElementsIndirect) L(glDrawArraysIndirect)
    L(glMultiDrawElementsIndirect) L(glMultiDrawArraysIndirect) L(glMultiDrawArrays) L(glMultiDrawElements) L(glMultiDrawElementsBaseVertex) L(glGenQueries) L(glBeginQuery) L(glEndQuery)
    L(glGetQueryObjectuiv) L(glBeginConditionalRender) L(glEndConditionalRender) L(glPrimitiveRestartIndex)
    L(glTransformFeedbackVaryings) L(glBeginTransformFeedback) L(glEndTransformFeedback) L(glFenceSync) L(glWaitSync)
    L(glClientWaitSync) L(glMapBufferRange) L(glUnmapBuffer) L(glGenSamplers) L(glBindSampler) L(glSamplerParameteri)
    L(glDeleteFramebuffers) L(glDeleteBuffers) L(glDeleteVertexArrays) L(glClearDepthf) L(glGetInteger64v)
    L(glEnablei) L(glDisablei) L(glBlendFunci) L(glViewportIndexedf) L(glDrawRangeElements) L(glCopyImageSubData)
    L(glProvokingVertex) L(glVertexAttribIFormat) L(glGenerateMipmap)
}

// ---------------------------------------------------------------------------
static void drain(void) { while (glGetError()) {} }
static const char *errs(void)
{
    GLenum e = glGetError();
    drain();
    switch (e) {
    case 0: return "no error";
    case 0x500: return "INVALID_ENUM";
    case 0x501: return "INVALID_VALUE";
    case 0x502: return "INVALID_OPERATION";
    case 0x506: return "INVALID_FRAMEBUFFER_OPERATION";
    default: { static char b[16]; sprintf(b, "err 0x%x", e); return b; }
    }
}

static int g_pass, g_fail;
static void result(const char *name, int ok, const char *detail)
{
    printf("%-44s %s  %s | %s\n", name, ok ? "PASS" : "FAIL", detail ? detail : "", errs());
    fflush(stdout);
    if (ok) g_pass++; else g_fail++;
}

static GLuint compile(GLenum type, const char *src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048]; GLsizei n = 0;
        glGetShaderInfoLog(s, sizeof log - 1, &n, log); log[n] = 0;
        printf("  shader compile failed (type 0x%x): %s\n", type, log);
    }
    return s;
}

static GLuint link_prog(GLuint a, GLuint b, const char *const *xfb, int nxfb)
{
    GLuint p = glCreateProgram();
    glAttachShader(p, a);
    if (b) glAttachShader(p, b);
    if (xfb) glTransformFeedbackVaryings(p, nxfb, xfb, GL_INTERLEAVED_ATTRIBS);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048]; GLsizei n = 0;
        glGetProgramInfoLog(p, sizeof log - 1, &n, log); log[n] = 0;
        printf("  program link failed: %s\n", log);
    }
    return p;
}

static GLuint prog(const char *vs, const char *fs) { return link_prog(compile(GL_VERTEX_SHADER, vs), compile(GL_FRAGMENT_SHADER, fs), NULL, 0); }
static GLuint cprog(const char *cs) { return link_prog(compile(GL_COMPUTE_SHADER, cs), 0, NULL, 0); }

#define W 64
#define H 64
typedef struct { GLuint fbo, tex0, tex1, depth; int samples; } Fbo;

static GLuint mk_tex2d(GLenum fmt, int w, int h)
{
    GLuint t; glGenTextures(1, &t); glBindTexture(GL_TEXTURE_2D, t);
    glTexStorage2D(GL_TEXTURE_2D, 1, fmt, w, h);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    return t;
}

// depth_kind: 0 none, 1 renderbuffer DEPTH24, 2 depth texture 32F
static Fbo mk_fbo(GLenum fmt0, GLenum fmt1, int samples, int depth_kind)
{
    Fbo f = {0}; f.samples = samples;
    glGenFramebuffers(1, &f.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo);
    if (samples) {
        glGenTextures(1, &f.tex0); glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, f.tex0);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, fmt0, W, H, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, f.tex0, 0);
        if (depth_kind) {
            glGenRenderbuffers(1, &f.depth); glBindRenderbuffer(GL_RENDERBUFFER, f.depth);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT24, W, H);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, f.depth);
        }
    } else {
        if (fmt0) { f.tex0 = mk_tex2d(fmt0, W, H); glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, f.tex0, 0); }
        if (fmt1) { f.tex1 = mk_tex2d(fmt1, W, H); glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 1, GL_TEXTURE_2D, f.tex1, 0); }
        if (depth_kind == 1) {
            glGenRenderbuffers(1, &f.depth); glBindRenderbuffer(GL_RENDERBUFFER, f.depth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, W, H);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, f.depth);
        } else if (depth_kind == 2) {
            f.depth = mk_tex2d(GL_DEPTH_COMPONENT32F, W, H);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, f.depth, 0);
        } else if (depth_kind == 3) {
            glGenRenderbuffers(1, &f.depth); glBindRenderbuffer(GL_RENDERBUFFER, f.depth);
            glRenderbufferStorage(GL_RENDERBUFFER, 0x88F0 /*DEPTH24_STENCIL8*/, W, H);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, 0x821A /*DEPTH_STENCIL_ATTACHMENT*/, GL_RENDERBUFFER, f.depth);
        } else if (depth_kind == 4) {
            f.depth = mk_tex2d(0x88F0, W, H);
            glFramebufferTexture2D(GL_FRAMEBUFFER, 0x821A, GL_TEXTURE_2D, f.depth, 0);
        }
    }
    GLenum bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0 + 1 };
    if (fmt0) glDrawBuffers(fmt1 ? 2 : 1, bufs); else glDrawBuffer(GL_NONE);
    GLenum st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (st != GL_FRAMEBUFFER_COMPLETE) printf("  FBO incomplete: 0x%x\n", st);
    glViewport(0, 0, W, H);
    return f;
}

static void clear_rgba(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a); glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// reads one pixel of color attachment `att` of the FBO bound as READ (0 = window)
static void px(GLuint fbo, int att, int x, int y, unsigned char out[4])
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glReadBuffer(fbo ? GL_COLOR_ATTACHMENT0 + att : GL_BACK);
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, out);
}
static int isc(const unsigned char *p, int r, int g, int b) { return abs(p[0] - r) <= 8 && abs(p[1] - g) <= 8 && abs(p[2] - b) <= 8; }
static char *pxs(const unsigned char *p) { static char b[4][32]; static int k; k = (k + 1) & 3; sprintf(b[k], "(%d,%d,%d,%d)", p[0], p[1], p[2], p[3]); return b[k]; }

// ---------------------------------------------------------------------------
static const char *VS_BASIC =
    "#version 430\n layout(location=0) in vec4 pos; uniform vec4 xf = vec4(1,1,0,0);\n"
    "void main(){ gl_Position = vec4(pos.xy*xf.xy+xf.zw, pos.z, 1.0); }\n";
static const char *FS_COLOR =
    "#version 430\n uniform vec4 color = vec4(1,0,0,1); out vec4 frag; void main(){ frag = color; }\n";

static GLuint g_vao, g_vbo, g_prog_basic, g_ibo;
// vertex 0-2: fullscreen CCW triangle; 3-5: the same with CW winding; 6-8: left-half triangle; 9-11: right-half triangle
static const float g_verts[] = {
    -1,-1,0,1,  3,-1,0,1,  -1,3,0,1,
    -1,-1,0,1,  -1,3,0,1,  3,-1,0,1,
    -1,-1,0,1,  0,-1,0,1,  -1,1,0,1,   // covers roughly x<0 bottom-left half (triangle)
     1,-1,0,1,  1,1,0,1,   0,-1,0,1,
};
static void setup_common(void)
{
    glGenVertexArrays(1, &g_vao); glBindVertexArray(g_vao);
    glGenBuffers(1, &g_vbo); glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof g_verts, g_verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexBuffer(0, g_vbo, 0, 16);
    glEnableVertexAttribArray(0);
    glVertexAttribFormat(0, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(0, 0);
    static const unsigned short idx[] = { 0, 1, 2, 3, 4, 5 };
    glGenBuffers(1, &g_ibo); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx, idx, GL_STATIC_DRAW);
    g_prog_basic = prog(VS_BASIC, FS_COLOR);
    glUseProgram(g_prog_basic);
    glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE); glDisable(GL_BLEND); glDisable(GL_SCISSOR_TEST);
    glDepthFunc(GL_LESS); glDepthMask(GL_TRUE); glColorMask(1, 1, 1, 1);
}
static void set_color(GLuint p, float r, float g, float b, float a) { glUniform4f(glGetUniformLocation(p, "color"), r, g, b, a); }
static void tri(int first) { glDrawArrays(GL_TRIANGLES, first, 3); }

// ---------------------------------------------------------------------------
static void t_basic(void)
{
    Fbo f = mk_fbo(GL_RGBA8, 0, 0, 1);
    clear_rgba(0, 0, 1, 1);
    glUseProgram(g_prog_basic); set_color(g_prog_basic, 1, 0, 0, 1);
    tri(0);
    unsigned char p[4]; px(f.fbo, 0, W / 2, H / 2, p);
    result("T01 FBO draw, attrib-binding VAO", isc(p, 255, 0, 0), pxs(p));
    // orientation: viewport bottom half only
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo);
    clear_rgba(0, 0, 1, 1);
    glViewport(0, 0, W, H / 2); tri(0); glViewport(0, 0, W, H);
    unsigned char lo[4], hi[4]; px(f.fbo, 0, W / 2, H / 4, lo); px(f.fbo, 0, W / 2, 3 * H / 4, hi);
    char d[96]; sprintf(d, "low=%s high=%s", pxs(lo), pxs(hi));
    result("T02 viewport bottom half in FBO", isc(lo, 255, 0, 0) && isc(hi, 0, 0, 255), d);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo);
    clear_rgba(0, 0, 1, 1);
    glEnable(GL_SCISSOR_TEST); glScissor(0, 0, W, H / 2); tri(0); glDisable(GL_SCISSOR_TEST);
    px(f.fbo, 0, W / 2, H / 4, lo); px(f.fbo, 0, W / 2, 3 * H / 4, hi);
    sprintf(d, "low=%s high=%s", pxs(lo), pxs(hi));
    result("T03 scissor bottom half in FBO", isc(lo, 255, 0, 0) && isc(hi, 0, 0, 255), d);
    // culling and gl_FrontFacing in FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo);
    glEnable(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
    clear_rgba(0, 0, 1, 1); tri(0); unsigned char ccw[4]; px(f.fbo, 0, W / 2, H / 2, ccw);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo);
    clear_rgba(0, 0, 1, 1); tri(3); unsigned char cw[4]; px(f.fbo, 0, W / 2, H / 2, cw);
    glDisable(GL_CULL_FACE);
    sprintf(d, "ccw=%s cw=%s", pxs(ccw), pxs(cw));
    result("T04 back-face culling in FBO (CCW kept)", isc(ccw, 255, 0, 0) && isc(cw, 0, 0, 255), d);
    GLuint pf = prog(VS_BASIC, "#version 430\n out vec4 frag; void main(){ frag = gl_FrontFacing ? vec4(0,1,0,1) : vec4(1,1,0,1); }\n");
    glUseProgram(pf);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 1, 1); tri(0); px(f.fbo, 0, W / 2, H / 2, ccw);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 1, 1); tri(3); px(f.fbo, 0, W / 2, H / 2, cw);
    sprintf(d, "ccw=%s cw=%s", pxs(ccw), pxs(cw));
    result("T05 gl_FrontFacing in FBO", isc(ccw, 0, 255, 0) && isc(cw, 255, 255, 0), d);
    glUseProgram(g_prog_basic);
    // same culling test on the window
    glBindFramebuffer(GL_FRAMEBUFFER, 0); glViewport(0, 0, W, H);
    glEnable(GL_CULL_FACE);
    clear_rgba(0, 0, 1, 1); tri(0); glFinish(); px(0, 0, W / 2, H / 2, ccw);
    clear_rgba(0, 0, 1, 1); tri(3); glFinish(); px(0, 0, W / 2, H / 2, cw);
    glDisable(GL_CULL_FACE);
    sprintf(d, "ccw=%s cw=%s", pxs(ccw), pxs(cw));
    result("T06 back-face culling on window", isc(ccw, 255, 0, 0) && isc(cw, 0, 0, 255), d);
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo);
}

static void t_depth(void)
{
    Fbo f = mk_fbo(GL_RGBA8, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    // LESS: red at z=0 (window 0.5) then green at z=0.4 (window 0.7) must lose
    clear_rgba(0, 0, 1, 1);
    glUseProgram(g_prog_basic);
    GLint xf = glGetUniformLocation(g_prog_basic, "xf");
    set_color(g_prog_basic, 1, 0, 0, 1); tri(0);
    // move the second triangle back: VS uses pos.z directly, so draw from a z-shifted copy via uniform trick: use glDepthRange instead
    glDepthRange(0.7, 0.7); set_color(g_prog_basic, 0, 1, 0, 1); tri(0); glDepthRange(0, 1);
    unsigned char p[4]; px(f.fbo, 0, W / 2, H / 2, p);
    result("T07 depth test LESS keeps nearer", isc(p, 255, 0, 0), pxs(p));
    // GREATER with clear depth 0
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo);
    glClearColor(0, 0, 1, 1); glClearDepth(0.0); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_GREATER);
    glDepthRange(0.5, 0.5); set_color(g_prog_basic, 1, 0, 0, 1); tri(0);
    glDepthRange(0.3, 0.3); set_color(g_prog_basic, 0, 1, 0, 1); tri(0);
    glDepthRange(0, 1); glDepthFunc(GL_LESS);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T08 depth test GREATER (reversed-Z)", isc(p, 255, 0, 0), pxs(p));
    // gl_FragDepth
    GLuint pd = prog(VS_BASIC, "#version 430\n uniform vec4 color; uniform float dz; out vec4 frag; void main(){ frag = color; gl_FragDepth = dz; }\n");
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo);
    clear_rgba(0, 0, 1, 1);
    glUseProgram(pd);
    glUniform4f(glGetUniformLocation(pd, "color"), 1, 0, 0, 1); glUniform1f(glGetUniformLocation(pd, "dz"), 0.9f); tri(0);
    glUniform4f(glGetUniformLocation(pd, "color"), 0, 1, 0, 1); glUniform1f(glGetUniformLocation(pd, "dz"), 0.5f); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T09 gl_FragDepth write + test", isc(p, 0, 255, 0), pxs(p));
    glUseProgram(g_prog_basic);
    // depth texture sampled in a second pass
    Fbo fd = mk_fbo(0, 0, 0, 2);
    glClearDepth(1.0); glClear(GL_DEPTH_BUFFER_BIT);
    glDepthRange(0.25, 0.25); tri(0); glDepthRange(0, 1);
    glDisable(GL_DEPTH_TEST);
    GLuint ps = prog(VS_BASIC, "#version 430\n uniform sampler2D t; out vec4 frag; void main(){ float d = texelFetch(t, ivec2(gl_FragCoord.xy), 0).r; frag = vec4(d, 0, 0, 1); }\n");
    GLuint psh = prog(VS_BASIC, "#version 430\n uniform sampler2DShadow t; out vec4 frag; void main(){ float c = texture(t, vec3(gl_FragCoord.xy/64.0, 0.5)); frag = vec4(0, c, 0, 1); }\n");
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 1, 1);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, fd.depth);
    glUseProgram(ps); glUniform1i(glGetUniformLocation(ps, "t"), 0); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T10 depth texture fetch (expect ~64)", p[0] >= 56 && p[0] <= 72, pxs(p));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 1, 1);
    glUseProgram(psh); glUniform1i(glGetUniformLocation(psh, "t"), 0); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T11 shadow sampler compare (0.5<=0.25 -> 0)", isc(p, 0, 0, 0), pxs(p));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(g_prog_basic);
}

static void t_queries(void)
{
    Fbo f = mk_fbo(GL_RGBA8, 0, 0, 1);
    GLuint q[3]; glGenQueries(3, q);
    glUseProgram(g_prog_basic); set_color(g_prog_basic, 1, 0, 0, 1);
    clear_rgba(0, 0, 1, 1);
    glBeginQuery(GL_SAMPLES_PASSED, q[0]); tri(0); glEndQuery(GL_SAMPLES_PASSED);
    glBeginQuery(GL_ANY_SAMPLES_PASSED, q[1]); tri(0); glEndQuery(GL_ANY_SAMPLES_PASSED);
    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_NEVER);
    glBeginQuery(GL_SAMPLES_PASSED, q[2]); tri(0); glEndQuery(GL_SAMPLES_PASSED);
    glDepthFunc(GL_LESS); glDisable(GL_DEPTH_TEST);
    GLuint r0 = 12345, r1 = 12345, r2 = 12345, av = 0;
    glGetQueryObjectuiv(q[0], GL_QUERY_RESULT, &r0);
    glGetQueryObjectuiv(q[1], GL_QUERY_RESULT, &r1);
    glGetQueryObjectuiv(q[2], GL_QUERY_RESULT, &r2);
    glGetQueryObjectuiv(q[0], GL_QUERY_RESULT_AVAILABLE, &av);
    char d[128]; sprintf(d, "samples=%u any=%u occluded=%u avail=%u", r0, r1, r2, av);
    result("T12 occlusion queries", r0 == W * H && r1 == 1 && r2 == 0, d);
    // conditional render on the passed query and on the zero query
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo);
    clear_rgba(0, 0, 1, 1);
    glBeginConditionalRender(q[0], GL_QUERY_WAIT); set_color(g_prog_basic, 1, 0, 0, 1); tri(0); glEndConditionalRender();
    unsigned char a[4]; px(f.fbo, 0, W / 2, H / 2, a);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo);
    clear_rgba(0, 0, 1, 1);
    glBeginConditionalRender(q[2], GL_QUERY_WAIT); tri(0); glEndConditionalRender();
    unsigned char b[4]; px(f.fbo, 0, W / 2, H / 2, b);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo);
    clear_rgba(0, 0, 1, 1);
    glBeginConditionalRender(q[1], GL_QUERY_NO_WAIT); tri(0); glEndConditionalRender();
    unsigned char c[4]; px(f.fbo, 0, W / 2, H / 2, c);
    sprintf(d, "visible->%s occluded->%s nowait->%s", pxs(a), pxs(b), pxs(c));
    result("T13 conditional render (draw/skip/draw)", isc(a, 255, 0, 0) && isc(b, 0, 0, 255) && isc(c, 255, 0, 0), d);
    // query result read in the same frame right after the draw, no flush, via a fresh query reused twice
    GLuint q4; glGenQueries(1, &q4);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 1, 1);
    glBeginQuery(GL_SAMPLES_PASSED, q4); tri(6); glEndQuery(GL_SAMPLES_PASSED);
    GLuint r4a = 0; glGetQueryObjectuiv(q4, GL_QUERY_RESULT, &r4a);
    glBeginQuery(GL_SAMPLES_PASSED, q4); tri(9); glEndQuery(GL_SAMPLES_PASSED);
    GLuint r4b = 0; glGetQueryObjectuiv(q4, GL_QUERY_RESULT, &r4b);
    sprintf(d, "half1=%u half2=%u (expect >0, <%d each)", r4a, r4b, W * H);
    result("T14 reused query object", r4a > 0 && r4b > 0 && r4a < (GLuint)(W * H) && r4b < (GLuint)(W * H), d);
}

static void t_instancing(void)
{
    Fbo f = mk_fbo(GL_RGBA8, 0, 0, 0);
    // per-instance color attribute at binding 1 with divisor 1; 4 instances' colors
    static const float cols[] = { 1,0,0,1,  0,1,0,1,  0,0,1,1,  1,1,0,1 };
    GLuint cb; glGenBuffers(1, &cb); glBindBuffer(GL_ARRAY_BUFFER, cb); glBufferData(GL_ARRAY_BUFFER, sizeof cols, cols, GL_STATIC_DRAW);
    glBindVertexBuffer(1, cb, 0, 16); glEnableVertexAttribArray(1); glVertexAttribFormat(1, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(1, 1); glVertexBindingDivisor(1, 1);
    GLuint pi = prog("#version 430\n layout(location=0) in vec4 pos; layout(location=1) in vec4 icol; out vec4 vcol; flat out int iid;\n"
                     "void main(){ gl_Position = vec4(pos.xy, 0, 1); vcol = icol; iid = gl_InstanceID; }\n",
                     "#version 430\n in vec4 vcol; flat in int iid; uniform int useId; out vec4 frag;\n"
                     "void main(){ frag = useId==1 ? vec4(float(iid)/4.0, 0.5, 0, 1) : vcol; }\n");
    glUseProgram(pi); glUniform1i(glGetUniformLocation(pi, "useId"), 0);
    unsigned char p[4]; char d[128];
    clear_rgba(0, 0, 0, 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 1);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T15 instanced attribute, instance 0", isc(p, 255, 0, 0), pxs(p));
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1);
    glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 3, 1, 2);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T16 DrawArraysInstancedBaseInstance=2 (blue)", isc(p, 0, 0, 255), pxs(p));
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1);
    glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0, 1, 0, 3);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T17 DrawElementsInst.BaseVertexBaseInstance=3", isc(p, 255, 255, 0), pxs(p));
    // gl_InstanceID must be 0 with baseInstance 2
    glUniform1i(glGetUniformLocation(pi, "useId"), 1);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1);
    glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 3, 1, 2);
    px(f.fbo, 0, W / 2, H / 2, p);
    sprintf(d, "%s (r=0 expected, r=128 means baseInstance added)", pxs(p));
    result("T18 gl_InstanceID with baseInstance", p[0] < 8 && isc(p + 1, 128, 0, 255) , d);
    // base vertex: indices 0..2 + basevertex 3 -> the CW copy (still drawn, culling off)
    glUniform1i(glGetUniformLocation(pi, "useId"), 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1);
    glDrawElementsBaseVertex(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0, 9);  // right-half triangle (verts 9..11)
    unsigned char l[4], r[4]; px(f.fbo, 0, 4, 4, l); px(f.fbo, 0, W - 4, H / 2, r);
    sprintf(d, "left=%s right=%s", pxs(l), pxs(r));
    result("T19 DrawElementsBaseVertex=9 (right half)", isc(l, 0, 0, 0) && isc(r, 255, 0, 0), d);
    glVertexBindingDivisor(1, 0); glBindVertexBuffer(1, 0, 0, 16);
    glUseProgram(g_prog_basic);
}

typedef struct { GLuint count, instanceCount, firstIndex, baseVertex, baseInstance; } DEIC;
typedef struct { GLuint count, instanceCount, first, baseInstance; } DAIC;

static void t_indirect(void)
{
    Fbo f = mk_fbo(GL_RGBA8, 0, 0, 0);
    glUseProgram(g_prog_basic); set_color(g_prog_basic, 1, 0, 0, 1);
    // indices for the two half triangles
    static const unsigned short idx[] = { 6, 7, 8, 9, 10, 11 };
    GLuint ib; glGenBuffers(1, &ib); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib); glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx, idx, GL_STATIC_DRAW);
    DEIC cmds[2] = { { 3, 1, 0, 0, 0 }, { 3, 1, 3, 0, 0 } };
    GLuint ind; glGenBuffers(1, &ind); glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ind); glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof cmds, cmds, GL_DYNAMIC_DRAW);
    unsigned char l[4], r[4]; char d[128];
    clear_rgba(0, 0, 0, 1);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, 0, 2, 0);
    px(f.fbo, 0, 4, 4, l); px(f.fbo, 0, W - 4, H / 2, r);
    sprintf(d, "left=%s right=%s", pxs(l), pxs(r));
    result("T20 MultiDrawElementsIndirect x2 (CPU cmds)", isc(l, 255, 0, 0) && isc(r, 255, 0, 0), d);
    // stride 32 variant
    struct { DEIC c; GLuint pad[3]; } cmds32[2] = { { { 3, 1, 0, 0, 0 }, {0} }, { { 3, 1, 3, 0, 0 }, {0} } };
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof cmds32, cmds32, GL_DYNAMIC_DRAW);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, 0, 2, 32);
    px(f.fbo, 0, 4, 4, l); px(f.fbo, 0, W - 4, H / 2, r);
    sprintf(d, "left=%s right=%s", pxs(l), pxs(r));
    result("T21 MultiDrawElementsIndirect stride 32", isc(l, 255, 0, 0) && isc(r, 255, 0, 0), d);
    // GPU-written commands: start with count 0, compute fills in counts
    DEIC zero[2] = { { 0, 1, 0, 0, 0 }, { 0, 1, 3, 0, 0 } };
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof zero, zero, GL_DYNAMIC_COPY);
    GLuint cs = cprog("#version 430\n layout(local_size_x=1) in; layout(std430, binding=0) buffer Cmd { uint c[]; };\n"
                      "void main(){ c[0] = 3u; c[5] = 3u; }\n");
    glUseProgram(cs); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ind);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
    glUseProgram(g_prog_basic);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ind);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, 0, 2, 0);
    px(f.fbo, 0, 4, 4, l); px(f.fbo, 0, W - 4, H / 2, r);
    DEIC back[2]; glBindBuffer(GL_SHADER_STORAGE_BUFFER, ind); glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof back, back);
    sprintf(d, "left=%s right=%s readback counts=%u,%u", pxs(l), pxs(r), back[0].count, back[1].count);
    result("T22 compute-written indirect commands", isc(l, 255, 0, 0) && isc(r, 255, 0, 0), d);
    // same through an atomic counter buffer incremented by the compute shader (count = 3 * atomic)
    GLuint ac; glGenBuffers(1, &ac); glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ac);
    GLuint zeroc = 0; glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4, &zeroc, GL_DYNAMIC_COPY);
    GLuint cs2 = cprog("#version 430\n layout(local_size_x=1) in; layout(binding=0, offset=0) uniform atomic_uint n;\n"
                       "layout(std430, binding=0) buffer Cmd { uint c[]; }; void main(){ uint k = atomicCounterIncrement(n); c[0] = 3u*(k+1u); }\n");
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof zero, zero, GL_DYNAMIC_COPY);
    glUseProgram(cs2); glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, ac); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ind);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
    GLuint acv = 99; glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ac); glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, 4, &acv);
    glUseProgram(g_prog_basic);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ind);
    glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, 0);
    px(f.fbo, 0, 4, 4, l);
    sprintf(d, "left=%s atomic=%u", pxs(l), acv);
    result("T23 atomic counter + indirect draw", isc(l, 255, 0, 0) && acv == 1, d);
    // arrays indirect
    DAIC ac2 = { 3, 1, 0, 0 };
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof ac2, &ac2, GL_DYNAMIC_DRAW);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1);
    glMultiDrawArraysIndirect(GL_TRIANGLES, 0, 1, 0);
    px(f.fbo, 0, W / 2, H / 2, l);
    result("T24 MultiDrawArraysIndirect", isc(l, 255, 0, 0), pxs(l));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
}


static void t_multidraw(void)
{
    Fbo f = mk_fbo(GL_RGBA8, 0, 0, 0);
    glUseProgram(g_prog_basic); set_color(g_prog_basic, 1, 0, 0, 1);
    unsigned char l[4], r[4]; char d[128];
#define HALVES(name, ok_extra) px(f.fbo, 0, 4, 4, l); px(f.fbo, 0, W - 4, H / 2, r); sprintf(d, "left=%s right=%s", pxs(l), pxs(r)); \
    result(name, isc(l, 255, 0, 0) && isc(r, 255, 0, 0) && (ok_extra), d); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo);
    static const GLint first[2] = { 6, 9 }; static const GLsizei cnt[2] = { 3, 3 };
    clear_rgba(0, 0, 0, 1);
    glMultiDrawArrays(GL_TRIANGLES, first, cnt, 2);
    HALVES("T50 glMultiDrawArrays x2", 1)
    static const unsigned short idx[] = { 6, 7, 8, 9, 10, 11, 0, 1, 2 };
    GLuint ib; glGenBuffers(1, &ib); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib); glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx, idx, GL_STATIC_DRAW);
    const void *offs[2] = { (const void *)0, (const void *)6 };
    clear_rgba(0, 0, 0, 1);
    glMultiDrawElements(GL_TRIANGLES, cnt, GL_UNSIGNED_SHORT, offs, 2);
    HALVES("T51 glMultiDrawElements x2", 1)
    const void *offs0[2] = { (const void *)12, (const void *)12 }; static const GLint bv[2] = { 6, 9 };
    clear_rgba(0, 0, 0, 1);
    glMultiDrawElementsBaseVertex(GL_TRIANGLES, cnt, GL_UNSIGNED_SHORT, offs0, 2, bv);
    HALVES("T52 glMultiDrawElementsBaseVertex x2", 1)
    DAIC acm[2] = { { 3, 1, 6, 0 }, { 3, 1, 9, 0 } };
    GLuint ind; glGenBuffers(1, &ind); glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ind); glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof acm, acm, GL_DYNAMIC_DRAW);
    clear_rgba(0, 0, 0, 1);
    glMultiDrawArraysIndirect(GL_TRIANGLES, 0, 2, 0);
    HALVES("T53 glMultiDrawArraysIndirect x2", 1)
    DEIC ecm[3] = { { 3, 1, 0, 0, 0 }, { 3, 1, 3, 0, 0 }, { 3, 1, 6, 0, 0 } };
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof ecm, ecm, GL_DYNAMIC_DRAW);
    clear_rgba(0, 0, 0, 1);
    glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void *)0);
    glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void *)20);
    HALVES("T54 two DrawElementsIndirect (emulation)", 1)
    clear_rgba(0, 0, 0, 1);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void *)20, 1, 0);
    px(f.fbo, 0, 4, 4, l); px(f.fbo, 0, W - 4, H / 2, r); sprintf(d, "left=%s right=%s", pxs(l), pxs(r));
    result("T55 MDI drawcount 1 at offset 20 (right only)", isc(l, 0, 0, 0) && isc(r, 255, 0, 0), d);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void *)0, 3, 0);
    unsigned char c[4]; px(f.fbo, 0, 4, 4, l); px(f.fbo, 0, W - 4, H / 2, r); px(f.fbo, 0, W / 2, H - 4, c);
    sprintf(d, "left=%s right=%s top=%s", pxs(l), pxs(r), pxs(c));
    result("T56 MDI drawcount 3 (all three)", isc(l, 255, 0, 0) && isc(r, 255, 0, 0) && isc(c, 255, 0, 0), d);
    // instanced command inside MDI: second command with instanceCount 0 must draw nothing
    DEIC ecm2[2] = { { 3, 1, 0, 0, 0 }, { 3, 0, 3, 0, 0 } };
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof ecm2, ecm2, GL_DYNAMIC_DRAW);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void *)0, 2, 0);
    px(f.fbo, 0, 4, 4, l); px(f.fbo, 0, W - 4, H / 2, r); sprintf(d, "left=%s right=%s", pxs(l), pxs(r));
    result("T57 MDI with instanceCount 0 in cmd 2", isc(l, 255, 0, 0) && isc(r, 0, 0, 0), d);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
#undef HALVES
}

static void t_buffers(void)
{
    Fbo f = mk_fbo(GL_RGBA8, 0, 0, 0);
    unsigned char p[4]; char d[128];
    // SSBO written by compute, read by the vertex shader (vertex pulling)
    GLuint sb; glGenBuffers(1, &sb); glBindBuffer(GL_SHADER_STORAGE_BUFFER, sb);
    float degenerate[12] = {0};
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof degenerate, degenerate, GL_DYNAMIC_COPY);
    GLuint cs = cprog("#version 430\n layout(local_size_x=3) in; layout(std430, binding=1) buffer P { vec4 p[]; };\n"
                      "void main(){ uint i = gl_GlobalInvocationID.x; p[i] = i==0u ? vec4(-1,-1,0,1) : i==1u ? vec4(3,-1,0,1) : vec4(-1,3,0,1); }\n");
    glUseProgram(cs); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sb); glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    GLuint pv = prog("#version 430\n layout(std430, binding=1) buffer P { vec4 p[]; }; void main(){ gl_Position = p[gl_VertexID]; }\n", FS_COLOR);
    glUseProgram(pv); set_color(pv, 1, 0, 0, 1);
    clear_rgba(0, 0, 0, 1); glDrawArrays(GL_TRIANGLES, 0, 3);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T25 compute -> SSBO -> vertex pulling", isc(p, 255, 0, 0), pxs(p));
    // SSBO read in the fragment shader
    float col[4] = { 0, 1, 0, 1 };
    GLuint sb2; glGenBuffers(1, &sb2); glBindBuffer(GL_SHADER_STORAGE_BUFFER, sb2); glBufferData(GL_SHADER_STORAGE_BUFFER, 16, col, GL_STATIC_DRAW);
    GLuint pf = prog(VS_BASIC, "#version 430\n layout(std430, binding=2) buffer M { vec4 c; }; out vec4 frag; void main(){ frag = c; }\n");
    glUseProgram(pf); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sb2);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T26 SSBO read in fragment shader", isc(p, 0, 255, 0), pxs(p));
    // UBO std140 array indexed by a uniform, binding 3 via BindBufferRange with offset
    GLint align = 0; glGetIntegerv(0x8A34 /*UNIFORM_BUFFER_OFFSET_ALIGNMENT*/, &align);
    GLsizeiptr off = align > 0 ? align : 256;
    float *ub = (float *)calloc(off / 4 + 64 * 4, 4);
    for (int i = 0; i < 64; i++) { ub[off / 4 + i * 4 + 0] = i / 64.0f; ub[off / 4 + i * 4 + 1] = 1 - i / 64.0f; ub[off / 4 + i * 4 + 2] = 0; ub[off / 4 + i * 4 + 3] = 1; }
    GLuint ubo; glGenBuffers(1, &ubo); glBindBuffer(GL_UNIFORM_BUFFER, ubo); glBufferData(GL_UNIFORM_BUFFER, off + 64 * 16, ub, GL_STATIC_DRAW);
    GLuint pu = prog(VS_BASIC, "#version 430\n layout(std140, binding=3) uniform B { vec4 c[64]; }; uniform int k; out vec4 frag; void main(){ frag = c[k]; }\n");
    glUseProgram(pu); glBindBufferRange(GL_UNIFORM_BUFFER, 3, ubo, off, 64 * 16); glUniform1i(glGetUniformLocation(pu, "k"), 32);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    sprintf(d, "%s align=%d (expect 128,128)", pxs(p), align);
    result("T27 UBO std140 array with BindBufferRange", isc(p, 128, 128, 0), d);
    // texture buffer in VS and FS
    float tb[16] = { -1,-1,0,1, 3,-1,0,1, -1,3,0,1, 0,0,1,1 };
    GLuint tbo; glGenBuffers(1, &tbo); glBindBuffer(GL_TEXTURE_BUFFER, tbo); glBufferData(GL_TEXTURE_BUFFER, sizeof tb, tb, GL_STATIC_DRAW);
    GLuint tbt; glGenTextures(1, &tbt); glActiveTexture(GL_TEXTURE0 + 2); glBindTexture(GL_TEXTURE_BUFFER, tbt); glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, tbo);
    GLuint pt = prog("#version 430\n uniform samplerBuffer tb; void main(){ gl_Position = texelFetch(tb, gl_VertexID); }\n",
                     "#version 430\n uniform samplerBuffer tb; out vec4 frag; void main(){ frag = texelFetch(tb, 3); }\n");
    glUseProgram(pt); glUniform1i(glGetUniformLocation(pt, "tb"), 2);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); glDrawArrays(GL_TRIANGLES, 0, 3);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T28 texture buffer fetch in VS+FS", isc(p, 0, 0, 255), pxs(p));
    glActiveTexture(GL_TEXTURE0);
    // image store from compute, then sampled
    GLuint img = mk_tex2d(GL_RGBA8, W, H);
    GLuint ci = cprog("#version 430\n layout(local_size_x=8, local_size_y=8) in; layout(rgba8, binding=0) uniform writeonly image2D im;\n"
                      "void main(){ imageStore(im, ivec2(gl_GlobalInvocationID.xy), vec4(1,0,1,1)); }\n");
    glUseProgram(ci); glBindImageTexture(0, img, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8); glDispatchCompute(W / 8, H / 8, 1);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    GLuint ps = prog(VS_BASIC, "#version 430\n uniform sampler2D t; out vec4 frag; void main(){ frag = texelFetch(t, ivec2(gl_FragCoord.xy), 0); }\n");
    glUseProgram(ps); glBindTexture(GL_TEXTURE_2D, img); glUniform1i(glGetUniformLocation(ps, "t"), 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T29 compute image store -> sample", isc(p, 255, 0, 255), pxs(p));
    // transform feedback -> vertex buffer
    const char *vary[] = { "opos" };
    GLuint pxf = link_prog(compile(GL_VERTEX_SHADER, "#version 430\n layout(location=0) in vec4 pos; out vec4 opos; void main(){ opos = vec4(pos.xy, 0.5, 1); gl_Position = opos; }\n"),
                           compile(GL_FRAGMENT_SHADER, FS_COLOR), vary, 1);
    GLuint xb; glGenBuffers(1, &xb); glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xb); glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 3 * 16, NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xb);
    glUseProgram(pxf); glEnable(GL_RASTERIZER_DISCARD);
    glBeginTransformFeedback(GL_TRIANGLES); glDrawArrays(GL_TRIANGLES, 0, 3); glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    glUseProgram(g_prog_basic); set_color(g_prog_basic, 0, 1, 1, 1);
    glBindVertexBuffer(0, xb, 0, 16);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    glBindVertexBuffer(0, g_vbo, 0, 16);
    px(f.fbo, 0, W / 2, H / 2, p);
    float fb[12] = {0}; glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xb); glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 48, fb);
    sprintf(d, "%s xfb[0]=(%g,%g,%g,%g)", pxs(p), fb[0], fb[1], fb[2], fb[3]);
    result("T30 transform feedback -> draw", isc(p, 0, 255, 255), d);
    // primitive restart (fixed index) on a strip: left tri, restart, right tri; the middle must stay black
    static const unsigned short sidx[] = { 6, 7, 8, 0xFFFF, 9, 10, 11 };
    GLuint sib; glGenBuffers(1, &sib); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sib); glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof sidx, sidx, GL_STATIC_DRAW);
    set_color(g_prog_basic, 1, 0, 0, 1);
    glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1);
    glDrawElements(GL_TRIANGLE_STRIP, 7, GL_UNSIGNED_SHORT, 0);
    glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    unsigned char l[4], m[4], r[4]; px(f.fbo, 0, 4, 4, l); px(f.fbo, 0, W / 2, H - 4, m); px(f.fbo, 0, W - 4, H / 2, r);
    sprintf(d, "left=%s mid-top=%s right=%s", pxs(l), pxs(m), pxs(r));
    result("T31 primitive restart fixed index", isc(l, 255, 0, 0) && isc(m, 0, 0, 0) && isc(r, 255, 0, 0), d);
    glEnable(GL_PRIMITIVE_RESTART); glPrimitiveRestartIndex(0xFFFF);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1);
    glDrawElements(GL_TRIANGLE_STRIP, 7, GL_UNSIGNED_SHORT, 0);
    glDisable(GL_PRIMITIVE_RESTART);
    px(f.fbo, 0, 4, 4, l); px(f.fbo, 0, W / 2, H - 4, m); px(f.fbo, 0, W - 4, H / 2, r);
    sprintf(d, "left=%s mid-top=%s right=%s", pxs(l), pxs(m), pxs(r));
    result("T32 primitive restart index 0xFFFF", isc(l, 255, 0, 0) && isc(m, 0, 0, 0) && isc(r, 255, 0, 0), d);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
}

static void t_targets(void)
{
    unsigned char p[4], q[4]; char d[160];
    // MRT RGBA8 + RGBA16F
    Fbo f = mk_fbo(GL_RGBA8, GL_RGBA16F, 0, 1);
    GLuint pm = prog(VS_BASIC, "#version 430\n layout(location=0) out vec4 a; layout(location=1) out vec4 b; void main(){ a = vec4(1,0,0,1); b = vec4(0,1,0,1); }\n");
    glUseProgram(pm);
    clear_rgba(0, 0, 1, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p); px(f.fbo, 1, W / 2, H / 2, q);
    sprintf(d, "att0=%s att1=%s", pxs(p), pxs(q));
    result("T33 MRT two attachments", isc(p, 255, 0, 0) && isc(q, 0, 255, 0), d);
    // ColorMaski on attachment 1 and ClearBufferfv on attachment 1
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo);
    clear_rgba(0, 0, 1, 1);
    glColorMaski(1, 0, 0, 0, 0); tri(0); glColorMaski(1, 1, 1, 1, 1);
    px(f.fbo, 0, W / 2, H / 2, p); px(f.fbo, 1, W / 2, H / 2, q);
    sprintf(d, "att0=%s att1=%s", pxs(p), pxs(q));
    result("T34 glColorMaski(1) blocks attachment 1", isc(p, 255, 0, 0) && isc(q, 0, 0, 255), d);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo);
    float yel[4] = { 1, 1, 0, 1 }; glClearBufferfv(GL_COLOR, 1, yel);
    px(f.fbo, 1, W / 2, H / 2, q);
    result("T35 glClearBufferfv attachment 1", isc(q, 255, 255, 0), pxs(q));
    glUseProgram(g_prog_basic);
    // MSAA 4x resolve
    Fbo ms = mk_fbo(GL_RGBA8, 0, 4, 1);
    Fbo rs = mk_fbo(GL_RGBA8, 0, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, ms.fbo); glViewport(0, 0, W, H);
    glEnable(GL_MULTISAMPLE);
    clear_rgba(0, 0, 1, 1); set_color(g_prog_basic, 1, 0, 0, 1); tri(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, ms.fbo); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rs.fbo);
    glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    px(rs.fbo, 0, W / 2, H / 2, p);
    result("T36 4x MSAA FBO + resolve blit", isc(p, 255, 0, 0), pxs(p));
    // MSAA with depth: near/far test inside the MS target
    glBindFramebuffer(GL_FRAMEBUFFER, ms.fbo); glEnable(GL_DEPTH_TEST);
    clear_rgba(0, 0, 1, 1);
    glDepthRange(0.5, 0.5); set_color(g_prog_basic, 1, 0, 0, 1); tri(0);
    glDepthRange(0.7, 0.7); set_color(g_prog_basic, 0, 1, 0, 1); tri(0);
    glDepthRange(0, 1); glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, ms.fbo); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rs.fbo);
    glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    px(rs.fbo, 0, W / 2, H / 2, p);
    result("T37 depth test inside MSAA FBO", isc(p, 255, 0, 0), pxs(p));
    // MS texture sampled directly (texelFetch on sampler2DMS)
    GLuint pms = prog(VS_BASIC, "#version 430\n uniform sampler2DMS t; out vec4 frag; void main(){ frag = texelFetch(t, ivec2(gl_FragCoord.xy), 0); }\n");
    glBindFramebuffer(GL_FRAMEBUFFER, rs.fbo); clear_rgba(0, 0, 0, 1);
    glUseProgram(pms); glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ms.tex0); glUniform1i(glGetUniformLocation(pms, "t"), 0); tri(0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    px(rs.fbo, 0, W / 2, H / 2, p);
    result("T38 sampler2DMS texelFetch", isc(p, 255, 0, 0), pxs(p));
    glUseProgram(g_prog_basic);
    // blit scaled (downsample) and blit between FBO and window
    Fbo big = mk_fbo(GL_RGBA8, 0, 0, 0);
    clear_rgba(1, 0, 0, 1);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, big.fbo); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rs.fbo);
    glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, W, H, 0, 0, W / 2, H / 2, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    unsigned char a[4], b[4]; px(rs.fbo, 0, W / 4, H / 4, a); px(rs.fbo, 0, 3 * W / 4, 3 * H / 4, b);
    sprintf(d, "inside=%s outside=%s", pxs(a), pxs(b));
    result("T39 scaled blit to quarter", isc(a, 255, 0, 0) && isc(b, 0, 0, 0), d);
    // sRGB attachment with FRAMEBUFFER_SRGB: writing 0.5 linear stores ~188
    Fbo sr = mk_fbo(GL_SRGB8_ALPHA8, 0, 0, 0);
    glEnable(GL_FRAMEBUFFER_SRGB);
    clear_rgba(0, 0, 0, 1); set_color(g_prog_basic, 0.5f, 0.5f, 0.5f, 1); tri(0);
    glDisable(GL_FRAMEBUFFER_SRGB);
    px(sr.fbo, 0, W / 2, H / 2, p);
    sprintf(d, "%s (expect ~188 with encoding, 128 without)", pxs(p));
    result("T40 sRGB framebuffer encoding", p[0] >= 180 && p[0] <= 196, d);
    // blending
    glBindFramebuffer(GL_FRAMEBUFFER, rs.fbo);
    clear_rgba(0, 0, 1, 1);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    set_color(g_prog_basic, 1, 0, 0, 0.5f); tri(0);
    glDisable(GL_BLEND);
    px(rs.fbo, 0, W / 2, H / 2, p);
    result("T41 alpha blending (expect 128,0,128)", isc(p, 128, 0, 128), pxs(p));
    // discard
    GLuint pdisc = prog(VS_BASIC, "#version 430\n out vec4 frag; void main(){ if (gl_FragCoord.x < 32.0) discard; frag = vec4(1,0,0,1); }\n");
    glUseProgram(pdisc);
    clear_rgba(0, 0, 1, 1); tri(0);
    px(rs.fbo, 0, 8, H / 2, a); px(rs.fbo, 0, W - 8, H / 2, b);
    sprintf(d, "left=%s right=%s", pxs(a), pxs(b));
    result("T42 fragment discard", isc(a, 0, 0, 255) && isc(b, 255, 0, 0), d);
    glUseProgram(g_prog_basic);
    // texture array + integer texture + TexStorage/TexSubImage upload
    GLuint ta; glGenTextures(1, &ta); glBindTexture(GL_TEXTURE_2D_ARRAY, ta);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 2, 2, 2);
    unsigned char l0[16] = { 255,0,0,255, 255,0,0,255, 255,0,0,255, 255,0,0,255 }, l1[16] = { 0,255,0,255, 0,255,0,255, 0,255,0,255, 0,255,0,255 };
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 2, 2, 1, GL_RGBA, GL_UNSIGNED_BYTE, l0);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, 2, 2, 1, GL_RGBA, GL_UNSIGNED_BYTE, l1);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLuint pta = prog(VS_BASIC, "#version 430\n uniform sampler2DArray t; out vec4 frag; void main(){ frag = texelFetch(t, ivec3(1,1,1), 0); }\n");
    glUseProgram(pta); glUniform1i(glGetUniformLocation(pta, "t"), 0);
    clear_rgba(0, 0, 1, 1); tri(0);
    px(rs.fbo, 0, W / 2, H / 2, p);
    result("T43 texture array layer 1 fetch", isc(p, 0, 255, 0), pxs(p));
    GLuint ti; glGenTextures(1, &ti); glBindTexture(GL_TEXTURE_2D, ti);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 2, 2);
    unsigned iv[4] = { 7, 7, 7, 7 };
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RED_INTEGER, GL_UNSIGNED_INT, iv);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLuint pti = prog(VS_BASIC, "#version 430\n uniform usampler2D t; out vec4 frag; void main(){ uint v = texelFetch(t, ivec2(0,0), 0).r; frag = v==7u ? vec4(1,1,0,1) : vec4(float(v)/255.0,0,1,1); }\n");
    glUseProgram(pti); glUniform1i(glGetUniformLocation(pti, "t"), 0);
    clear_rgba(0, 0, 1, 1); tri(0);
    px(rs.fbo, 0, W / 2, H / 2, p);
    result("T44 integer texture fetch", isc(p, 255, 255, 0), pxs(p));
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(g_prog_basic);
}


static unsigned short f2h(float f)
{
    unsigned u; memcpy(&u, &f, 4);
    unsigned sign = (u >> 16) & 0x8000; int exp = (int)((u >> 23) & 0xFF) - 127 + 15; unsigned man = u & 0x7FFFFF;
    if (f == 0) return (unsigned short)sign;
    if (exp <= 0) return (unsigned short)sign;
    return (unsigned short)(sign | ((unsigned)exp << 10) | (man >> 13));
}

static GLuint prog3(const char *vs, const char *gs, const char *fs)
{
    GLuint p = glCreateProgram();
    glAttachShader(p, compile(GL_VERTEX_SHADER, vs)); glAttachShader(p, compile(0x8DD9 /*GEOMETRY*/, gs)); glAttachShader(p, compile(GL_FRAGMENT_SHADER, fs));
    glLinkProgram(p);
    GLint ok = 0; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) { char log[1024]; GLsizei n; glGetProgramInfoLog(p, 1023, &n, log); log[n] = 0; printf("  link failed: %s\n", log); }
    return p;
}

static void t_formats(void)
{
    Fbo f = mk_fbo(GL_RGBA8, 0, 0, 0);
    unsigned char p[4]; char d[128];
    GLuint vb; glGenBuffers(1, &vb);
    // F1: half-float positions at binding 2 feeding attrib 0
    unsigned short hv[12]; const float fv[12] = { -1,-1,0,1, 3,-1,0,1, -1,3,0,1 };
    for (int i = 0; i < 12; i++) hv[i] = f2h(fv[i]);
    glBindBuffer(GL_ARRAY_BUFFER, vb); glBufferData(GL_ARRAY_BUFFER, sizeof hv, hv, GL_STATIC_DRAW);
    glBindVertexBuffer(2, vb, 0, 8); glVertexAttribFormat(0, 4, 0x140B /*HALF_FLOAT*/, GL_FALSE, 0); glVertexAttribBinding(0, 2);
    glUseProgram(g_prog_basic); set_color(g_prog_basic, 1, 0, 0, 1);
    clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T60 HALF_FLOAT position attribute", isc(p, 255, 0, 0), pxs(p));
    glVertexAttribFormat(0, 4, GL_FLOAT, GL_FALSE, 0); glVertexAttribBinding(0, 0);
    // color attribute (location 1) in several packed formats, program outputs it
    GLuint pc = prog("#version 430\n layout(location=0) in vec4 pos; layout(location=1) in vec4 col; out vec4 vc; void main(){ gl_Position = vec4(pos.xy,0,1); vc = col; }\n",
                     "#version 430\n in vec4 vc; out vec4 frag; void main(){ frag = vc; }\n");
    glUseProgram(pc); glEnableVertexAttribArray(1); glVertexAttribBinding(1, 3);
    GLuint cb; glGenBuffers(1, &cb);
    // F2: INT_2_10_10_10_REV normalized: x=511 (1.0), y=0, z=0, w=1 (1.0 for 2-bit normalized? w=3 -> 1.0)
    unsigned packed[3]; for (int i = 0; i < 3; i++) packed[i] = (511u) | (0u << 10) | (0u << 20) | (1u << 30);
    glBindBuffer(GL_ARRAY_BUFFER, cb); glBufferData(GL_ARRAY_BUFFER, sizeof packed, packed, GL_STATIC_DRAW);
    glBindVertexBuffer(3, cb, 0, 4); glVertexAttribFormat(1, 4, 0x8D9F /*INT_2_10_10_10_REV*/, GL_TRUE, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T61 INT_2_10_10_10_REV normalized attribute", isc(p, 255, 0, 0) && p[3] == 255, pxs(p));
    // F3: UNSIGNED_INT_2_10_10_10_REV normalized (1023 -> 1.0)
    for (int i = 0; i < 3; i++) packed[i] = (0u) | (1023u << 10) | (0u << 20) | (3u << 30);
    glBufferData(GL_ARRAY_BUFFER, sizeof packed, packed, GL_STATIC_DRAW);
    glVertexAttribFormat(1, 4, 0x8368 /*UNSIGNED_INT_2_10_10_10_REV*/, GL_TRUE, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T62 UNSIGNED_INT_2_10_10_10_REV normalized", isc(p, 0, 255, 0), pxs(p));
    // F4: UNSIGNED_BYTE normalized colors (BGRA order too)
    unsigned char ub[12] = { 0,0,255,255, 0,0,255,255, 0,0,255,255 };
    glBufferData(GL_ARRAY_BUFFER, sizeof ub, ub, GL_STATIC_DRAW);
    glVertexAttribFormat(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T63 UNSIGNED_BYTE normalized attribute", isc(p, 0, 0, 255), pxs(p));
    glVertexAttribFormat(1, 0x80E1 /*GL_BGRA*/, GL_UNSIGNED_BYTE, GL_TRUE, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T64 BGRA UNSIGNED_BYTE attribute (expect red)", isc(p, 255, 0, 0), pxs(p));
    // F5: SHORT normalized (32767 -> 1.0) as 4 components, stride 8
    short sh[12] = { 32767,32767,0,32767, 32767,32767,0,32767, 32767,32767,0,32767 };
    glBufferData(GL_ARRAY_BUFFER, sizeof sh, sh, GL_STATIC_DRAW);
    glBindVertexBuffer(3, cb, 0, 8); glVertexAttribFormat(1, 4, GL_SHORT, GL_TRUE, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T65 SHORT normalized attribute (yellow)", isc(p, 255, 255, 0), pxs(p));
    // F6: integer attribute (IFormat) read as uint -> color index
    unsigned ia[3] = { 2, 2, 2 };
    glBufferData(GL_ARRAY_BUFFER, sizeof ia, ia, GL_STATIC_DRAW);
    glBindVertexBuffer(3, cb, 0, 4); glVertexAttribIFormat(1, 1, GL_UNSIGNED_INT, 0);
    GLuint pi = prog("#version 430\n layout(location=0) in vec4 pos; layout(location=1) in uint k; flat out uint kk; void main(){ gl_Position = vec4(pos.xy,0,1); kk = k; }\n",
                     "#version 430\n flat in uint kk; out vec4 frag; void main(){ frag = kk==2u ? vec4(0,1,1,1) : vec4(float(kk)/8.0,0,0,1); }\n");
    glUseProgram(pi);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T66 integer attribute (IFormat uint)", isc(p, 0, 255, 255), pxs(p));
    // F7: interleaved vertex: pos at 0, color at 16, stride 32, both from binding 4 via relative offsets
    float il[24]; for (int i = 0; i < 3; i++) { memcpy(il + i * 8, fv + i * 4, 16); il[i*8+4] = 1; il[i*8+5] = 0; il[i*8+6] = 1; il[i*8+7] = 1; }
    GLuint ib; glGenBuffers(1, &ib); glBindBuffer(GL_ARRAY_BUFFER, ib); glBufferData(GL_ARRAY_BUFFER, sizeof il, il, GL_STATIC_DRAW);
    glBindVertexBuffer(4, ib, 0, 32);
    glVertexAttribFormat(0, 4, GL_FLOAT, GL_FALSE, 0); glVertexAttribBinding(0, 4);
    glVertexAttribFormat(1, 4, GL_FLOAT, GL_FALSE, 16); glVertexAttribBinding(1, 4);
    glUseProgram(pc);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T67 interleaved attribs, relative offset 16", isc(p, 255, 0, 255), pxs(p));
    // F8: same buffer bound with a non-zero binding offset (second vertex set starts at 32*0 .. use offset 0 but relative 16 swapped)
    glBindVertexBuffer(4, ib, 16, 32);   // now attrib 0 (reloff 0) reads the colour, attrib 1 (reloff 16) reads next pos -> expect garbage-free: swap roles
    glVertexAttribFormat(0, 4, GL_FLOAT, GL_FALSE, 16); glVertexAttribFormat(1, 4, GL_FLOAT, GL_FALSE, 0);
    // attrib0 = offset 16+16 = next vertex pos (wrong), so instead test offset handling with a copy shifted by 16 bytes
    float il2[28]; memset(il2, 0, sizeof il2); memcpy(il2 + 4, il, sizeof il);
    glBufferData(GL_ARRAY_BUFFER, sizeof il2, il2, GL_STATIC_DRAW);
    glVertexAttribFormat(0, 4, GL_FLOAT, GL_FALSE, 0); glVertexAttribFormat(1, 4, GL_FLOAT, GL_FALSE, 16);
    glBindVertexBuffer(4, ib, 16, 32);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T68 vertex buffer binding offset 16", isc(p, 255, 0, 255), pxs(p));
    // restore
    glVertexAttribFormat(0, 4, GL_FLOAT, GL_FALSE, 0); glVertexAttribBinding(0, 0);
    glDisableVertexAttribArray(1); glBindVertexBuffer(3, 0, 0, 16); glBindVertexBuffer(4, 0, 0, 16); glBindVertexBuffer(2, 0, 0, 16);
    glUseProgram(g_prog_basic);
}

static void t_misc(void)
{
    unsigned char p[4], q[4]; char d[160];
    glUseProgram(g_prog_basic); set_color(g_prog_basic, 1, 0, 0, 1);
    // UNSIGNED_INT indices + DrawRangeElements
    Fbo f = mk_fbo(GL_RGBA8, 0, 0, 3);
    static const unsigned ui[] = { 6, 7, 8 };
    GLuint ib; glGenBuffers(1, &ib); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib); glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof ui, ui, GL_STATIC_DRAW);
    clear_rgba(0, 0, 0, 1);
    glDrawRangeElements(GL_TRIANGLES, 6, 8, 3, GL_UNSIGNED_INT, 0);
    px(f.fbo, 0, 4, 4, p); px(f.fbo, 0, W - 4, H / 2, q);
    sprintf(d, "left=%s right=%s", pxs(p), pxs(q));
    result("T70 DrawRangeElements UNSIGNED_INT", isc(p, 255, 0, 0) && isc(q, 0, 0, 0), d);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
    // stencil mask: left triangle writes 1, full draw where ==1
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1);
    glClearStencil(0); glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST); glStencilFunc(GL_ALWAYS, 1, 0xFF); glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glColorMask(0, 0, 0, 0); tri(6); glColorMask(1, 1, 1, 1);
    glStencilFunc(GL_EQUAL, 1, 0xFF); glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    tri(0);
    glDisable(GL_STENCIL_TEST);
    px(f.fbo, 0, 4, 4, p); px(f.fbo, 0, W - 4, H / 2, q);
    sprintf(d, "left=%s right=%s", pxs(p), pxs(q));
    result("T71 stencil write + test (D24S8 rb)", isc(p, 255, 0, 0) && isc(q, 0, 0, 0), d);
    // depth-stencil texture: depth sampled
    Fbo fds = mk_fbo(GL_RGBA8, 0, 0, 4);
    glEnable(GL_DEPTH_TEST);
    clear_rgba(0, 0, 0, 1); glDepthRange(0.25, 0.25); tri(0); glDepthRange(0, 1);
    glDisable(GL_DEPTH_TEST);
    GLuint ps = prog(VS_BASIC, "#version 430\n uniform sampler2D t; out vec4 frag; void main(){ frag = vec4(texelFetch(t, ivec2(gl_FragCoord.xy), 0).r, 0, 0, 1); }\n");
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 1, 1);
    glUseProgram(ps); glBindTexture(GL_TEXTURE_2D, fds.depth); glUniform1i(glGetUniformLocation(ps, "t"), 0); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T72 DEPTH24_STENCIL8 texture depth fetch (~64)", p[0] >= 56 && p[0] <= 72, pxs(p));
    glBindTexture(GL_TEXTURE_2D, 0); glUseProgram(g_prog_basic);
    // geometry shader passthrough and point expansion
    GLuint pg = prog3("#version 430\n layout(location=0) in vec4 pos; void main(){ gl_Position = vec4(pos.xy,0,1); }\n",
                      "#version 430\n layout(triangles) in; layout(triangle_strip, max_vertices=3) out; void main(){ for (int i=0;i<3;i++){ gl_Position = gl_in[i].gl_Position; EmitVertex(); } EndPrimitive(); }\n",
                      FS_COLOR);
    glUseProgram(pg); set_color(pg, 0, 1, 0, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T73 geometry shader passthrough", isc(p, 0, 255, 0), pxs(p));
    GLuint pg2 = prog3("#version 430\n layout(location=0) in vec4 pos; void main(){ gl_Position = vec4(0,0,0,1); }\n",
                       "#version 430\n layout(points) in; layout(triangle_strip, max_vertices=4) out; void main(){ gl_Position=vec4(-1,-1,0,1); EmitVertex(); gl_Position=vec4(1,-1,0,1); EmitVertex(); gl_Position=vec4(-1,1,0,1); EmitVertex(); gl_Position=vec4(1,1,0,1); EmitVertex(); EndPrimitive(); }\n",
                       FS_COLOR);
    glUseProgram(pg2); set_color(pg2, 0, 1, 0, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); glDrawArrays(GL_POINTS, 0, 1);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T74 geometry shader point -> quad", isc(p, 0, 255, 0), pxs(p));
    glUseProgram(g_prog_basic);
    // per-attachment blend on MRT
    Fbo m = mk_fbo(GL_RGBA8, GL_RGBA8, 0, 0);
    GLuint pm = prog(VS_BASIC, "#version 430\n layout(location=0) out vec4 a; layout(location=1) out vec4 b; void main(){ a = vec4(1,0,0,0.5); b = vec4(1,0,0,0.5); }\n");
    glUseProgram(pm);
    clear_rgba(0, 0, 1, 1);
    glEnablei(GL_BLEND, 1); glBlendFunci(1, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    tri(0);
    glDisablei(GL_BLEND, 1);
    px(m.fbo, 0, W / 2, H / 2, p); px(m.fbo, 1, W / 2, H / 2, q);
    sprintf(d, "att0=%s att1=%s", pxs(p), pxs(q));
    result("T75 per-attachment blend (glBlendFunci)", isc(p, 255, 0, 0) && isc(q, 128, 0, 128), d);
    // draw buffers with a NONE gap
    glBindFramebuffer(GL_FRAMEBUFFER, m.fbo);
    GLenum gap[2] = { GL_NONE, GL_COLOR_ATTACHMENT0 + 1 };
    glDrawBuffers(2, gap);
    glClearColor(0, 0, 1, 1); glClear(GL_COLOR_BUFFER_BIT);
    GLuint pm2 = prog(VS_BASIC, "#version 430\n layout(location=0) out vec4 a; layout(location=1) out vec4 b; void main(){ a = vec4(1,1,0,1); b = vec4(0,1,0,1); }\n");
    glUseProgram(pm2); tri(0);
    GLenum both[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0 + 1 }; glDrawBuffers(2, both);
    px(m.fbo, 0, W / 2, H / 2, p); px(m.fbo, 1, W / 2, H / 2, q);
    sprintf(d, "att0=%s att1=%s", pxs(p), pxs(q));
    result("T76 glDrawBuffers {NONE, ATT1}", isc(q, 0, 255, 0) && !isc(p, 255, 255, 0), d);
    glUseProgram(g_prog_basic); set_color(g_prog_basic, 1, 0, 0, 1);
    // viewport indexed
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 1, 1);
    glViewportIndexedf(0, 0, 0, W, H / 2); tri(0); glViewport(0, 0, W, H);
    px(f.fbo, 0, W / 2, H / 4, p); px(f.fbo, 0, W / 2, 3 * H / 4, q);
    sprintf(d, "low=%s high=%s", pxs(p), pxs(q));
    result("T77 glViewportIndexedf(0, bottom half)", isc(p, 255, 0, 0) && isc(q, 0, 0, 255), d);
    // render target formats
    struct { GLenum fmt; const char *n; } fm[] = { { 0x8059, "RGB10_A2" }, { 0x8C3A, "R11F_G11F_B10F" }, { 0x822F, "RG16F" }, { GL_R32F, "R32F" }, { GL_RGBA32F, "RGBA32F" }, { 0x8229, "R8" }, { 0x822B, "RG8" }, { 0x8814, "RGBA32F" } };
    for (int i = 0; i < 7; i++) {
        Fbo t = mk_fbo(fm[i].fmt, 0, 0, 0);
        clear_rgba(0, 0, 0, 1); tri(0);
        px(t.fbo, 0, W / 2, H / 2, p);
        char nm[64]; sprintf(nm, "T78 render to %s", fm[i].n);
        result(nm, p[0] >= 247 && p[1] <= 8, pxs(p));
        glDeleteFramebuffers(1, &t.fbo);
    }
    // PBO readback
    Fbo r = mk_fbo(GL_RGBA8, 0, 0, 0);
    clear_rgba(0, 0, 0, 1); tri(0);
    GLuint pbo; glGenBuffers(1, &pbo); glBindBuffer(0x88EB /*PIXEL_PACK*/, pbo); glBufferData(0x88EB, W * H * 4, NULL, 0x88E1 /*STREAM_READ*/);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, r.fbo); glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    unsigned char *mp = (unsigned char *)glMapBufferRange(0x88EB, 0, W * H * 4, 0x1);
    if (mp) memcpy(p, mp + (H / 2 * W + W / 2) * 4, 4); else memset(p, 7, 4);
    glUnmapBuffer(0x88EB); glBindBuffer(0x88EB, 0);
    result("T79 glReadPixels into PBO + map", isc(p, 255, 0, 0), pxs(p));
    // CopyImageSubData
    GLuint dst = mk_tex2d(GL_RGBA8, W, H);
    glCopyImageSubData(r.tex0, GL_TEXTURE_2D, 0, 0, 0, 0, dst, GL_TEXTURE_2D, 0, 0, 0, 0, W, H, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 1, 1);
    glUseProgram(ps); glBindTexture(GL_TEXTURE_2D, dst); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T80 glCopyImageSubData texture copy", p[0] >= 247, pxs(p));
    glUseProgram(g_prog_basic);
    // depth blit between FBOs
    Fbo da = mk_fbo(GL_RGBA8, 0, 0, 1), db = mk_fbo(GL_RGBA8, 0, 0, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, da.fbo); glEnable(GL_DEPTH_TEST);
    clear_rgba(0, 0, 0, 1); glDepthRange(0.3, 0.3); tri(0); glDepthRange(0, 1);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, da.fbo); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, db.fbo);
    glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, db.fbo);
    set_color(g_prog_basic, 0, 1, 0, 1); glDepthRange(0.5, 0.5); tri(0);
    set_color(g_prog_basic, 1, 0, 0, 1); glDepthRange(0.2, 0.2); tri(0); glDepthRange(0, 1);
    glDisable(GL_DEPTH_TEST);
    px(db.fbo, 0, W / 2, H / 2, p);
    result("T81 depth blit then depth test", isc(p, 255, 0, 0), pxs(p));
    // T150-T152: multisample depth -> single-sample depth blit (3DEXPERIENCE copies its 4x MSAA scene depth into the occlusion-test FBO this way)
    {
        Fbo ms = mk_fbo(GL_RGBA8, 0, 4, 1);                       // 4x MSAA colour + D24 renderbuffer
        glBindFramebuffer(GL_FRAMEBUFFER, ms.fbo); glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
        clear_rgba(0, 0, 0, 1); glDepthRange(0.3, 0.3); tri(0); glDepthRange(0, 1);
        for (int kind = 1; kind <= 2; kind++) {
            Fbo ss = mk_fbo(GL_RGBA8, 0, 0, kind);                // non-MSAA, D24 renderbuffer (1) or D32F texture (2)
            glBindFramebuffer(GL_READ_FRAMEBUFFER, ms.fbo); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ss.fbo);
            glClearColor(0, 0, 0, 1); glClearDepth(1.0); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            GLenum eb = glGetError();
            glBindFramebuffer(GL_FRAMEBUFFER, ss.fbo); glDepthFunc(GL_LEQUAL); glDepthMask(GL_FALSE);
            set_color(g_prog_basic, 0, 1, 0, 1); glDepthRange(0.5, 0.5); tri(0);      // behind the copied 0.3 -> must fail
            set_color(g_prog_basic, 1, 0, 0, 1); glDepthRange(0.2, 0.2); tri(0);      // in front -> red
            set_color(g_prog_basic, 0, 0, 1, 1); glDepthRange(0.31, 0.31); tri(0);    // just behind -> must fail
            glDepthRange(0, 1); glDepthMask(GL_TRUE);
            px(ss.fbo, 0, W / 2, H / 2, p);
            sprintf(d, "%s errBlit=%x (expect red)", pxs(p), eb);
            result(kind == 1 ? "T150 MSAA depth -> D24 renderbuffer blit, then depth test" : "T151 MSAA depth -> D32F texture blit, then depth test", isc(p, 255, 0, 0), d);
            if (kind == 2) {   // read the copied depth value through a sampler
                GLuint pdz = prog(VS_BASIC, "#version 430\n uniform sampler2D dz; out vec4 frag; void main(){ float z = texelFetch(dz, ivec2(gl_FragCoord.xy), 0).r; frag = vec4(z, 0, 0, 1); }\n");
                Fbo rd = mk_fbo(GL_RGBA8, 0, 0, 0); glBindFramebuffer(GL_FRAMEBUFFER, rd.fbo); glDisable(GL_DEPTH_TEST);
                glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, ss.depth); glTexParameteri(GL_TEXTURE_2D, 0x884C /*COMPARE_MODE*/, 0);
                glUseProgram(pdz); glUniform1i(glGetUniformLocation(pdz, "dz"), 0); tri(0); glUseProgram(g_prog_basic); glEnable(GL_DEPTH_TEST);
                px(rd.fbo, 0, W / 2, H / 2, p);
                sprintf(d, "copied depth*255 = %d (expect ~77)", p[0]);
                result("T152 copied MSAA depth value readable (~0.3)", p[0] >= 70 && p[0] <= 84, d);
            }
        }
        // T153: 3DEXPERIENCE's exact flavour: MSAA DEPTH24_STENCIL8 renderbuffer -> non-MSAA DEPTH24_STENCIL8 texture, mask DEPTH|STENCIL
        {
            GLuint fms, rbc, rbd; glGenFramebuffers(1, &fms); glBindFramebuffer(GL_FRAMEBUFFER, fms);
            glGenRenderbuffers(1, &rbc); glBindRenderbuffer(GL_RENDERBUFFER, rbc); glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, W, H); glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbc);
            glGenRenderbuffers(1, &rbd); glBindRenderbuffer(GL_RENDERBUFFER, rbd); glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, 0x88F0, W, H); glFramebufferRenderbuffer(GL_FRAMEBUFFER, 0x821A, GL_RENDERBUFFER, rbd);
            GLenum st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); clear_rgba(0, 0, 0, 1); glDepthRange(0.3, 0.3); tri(0); glDepthRange(0, 1);
            Fbo ss = mk_fbo(GL_RGBA8, 0, 0, 4);                   // D24S8 texture
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fms); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ss.fbo);
            glClearDepth(1.0); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
            GLenum eb = glGetError();
            glBindFramebuffer(GL_FRAMEBUFFER, ss.fbo); glDepthFunc(GL_LEQUAL); glDepthMask(GL_FALSE);
            set_color(g_prog_basic, 0, 1, 0, 1); glDepthRange(0.5, 0.5); tri(0);
            set_color(g_prog_basic, 1, 0, 0, 1); glDepthRange(0.2, 0.2); tri(0);
            glDepthRange(0, 1); glDepthMask(GL_TRUE);
            px(ss.fbo, 0, W / 2, H / 2, p);
            sprintf(d, "%s srcStatus=%x errBlit=%x (expect red)", pxs(p), st, eb);
            result("T153 MSAA D24S8 -> D24S8 texture blit (depth|stencil), then depth test", isc(p, 255, 0, 0), d);
        }
        // T170-T172: 3DEXPERIENCE's exact depth copy chain: 4x MSAA DEPTH24_STENCIL8 *texture* -> colour-less FBOs with D24S8 textures, read back with glReadPixels(DEPTH)
        {
            GLuint fa, ca, da2; glGenFramebuffers(1, &fa); glBindFramebuffer(GL_FRAMEBUFFER, fa);
            glGenTextures(1, &ca); glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ca); glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, W, H, GL_TRUE); glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, ca, 0);
            glGenTextures(1, &da2); glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, da2); glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, 0x88F0, W, H, GL_TRUE); glFramebufferTexture2D(GL_FRAMEBUFFER, 0x821A, GL_TEXTURE_2D_MULTISAMPLE, da2, 0);
            GLenum sta = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); glDepthMask(GL_TRUE); glClearDepth(1.0); clear_rgba(0, 0, 0, 1);
            set_color(g_prog_basic, 1, 1, 1, 1); glDepthRange(0.3, 0.3); tri(6); glDepthRange(0, 1);      // left-half triangle at depth 0.3, rest stays 1.0
            float zc = -1, ze = -1;
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fa); glReadPixels(W / 4, H / 4, 1, 1, 0x1902, GL_FLOAT, &zc); glReadPixels(3 * W / 4, 3 * H / 4, 1, 1, 0x1902, GL_FLOAT, &ze);
            GLenum e0 = glGetError();
            sprintf(d, "msaa src: left=%.3f right=%.3f err=%x (expect 0.3, 1.0; reading MSAA depth directly may be refused)", zc, ze, e0);
            result("T170 ReadPixels(DEPTH) from the 4x MSAA D24S8 texture FBO (info)", 1, d);
            // colour-less destination FBOs with D24S8 textures, chained like 3DEXPERIENCE (A -> B -> C)
            GLuint fb3[3], td[3];
            for (int i = 0; i < 3; i++) { glGenFramebuffers(1, &fb3[i]); glBindFramebuffer(GL_FRAMEBUFFER, fb3[i]); td[i] = mk_tex2d(0x88F0, W, H); glFramebufferTexture2D(GL_FRAMEBUFFER, 0x821A, GL_TEXTURE_2D, td[i], 0); glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE); glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }
            GLenum stb = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fa); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb3[0]); glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST); GLenum e1 = glGetError();
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fb3[0]); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb3[1]); glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST); GLenum e2 = glGetError();
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fb3[1]); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb3[2]); glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST); GLenum e3 = glGetError();
            float z[3][2];
            for (int i = 0; i < 3; i++) { glBindFramebuffer(GL_READ_FRAMEBUFFER, fb3[i]); glReadPixels(W / 4, H / 4, 1, 1, 0x1902, GL_FLOAT, &z[i][0]); glReadPixels(3 * W / 4, 3 * H / 4, 1, 1, 0x1902, GL_FLOAT, &z[i][1]); }
            GLenum e4 = glGetError();
            sprintf(d, "src status=%x dst status=%x blit errs=%x,%x,%x read err=%x | A: %.3f/%.3f B: %.3f/%.3f C: %.3f/%.3f (expect 0.3/1.0 each)", sta, stb, e1, e2, e3, e4, z[0][0], z[0][1], z[1][0], z[1][1], z[2][0], z[2][1]);
            int okA = z[0][0] > 0.29f && z[0][0] < 0.31f && z[0][1] > 0.99f, okC = z[2][0] > 0.29f && z[2][0] < 0.31f && z[2][1] > 0.99f;
            result("T171 MSAA D24S8 texture -> colour-less D24S8 texture FBO (depth|stencil blit)", okA, d);
            result("T172 chained depth copies between colour-less FBOs keep the values", okC, d);
            // T173: depth test against the chained copy (what the occlusion boxes do), colour-less FBO + SSBO write
            glBindFramebuffer(GL_FRAMEBUFFER, fb3[2]); glDepthFunc(GL_LEQUAL); glDepthMask(GL_FALSE); glColorMask(0, 0, 0, 0);
            { unsigned zero2[4] = { 0 }, got[4] = { 0 }; GLuint sbv; glGenBuffers(1, &sbv); glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbv); glBufferData(GL_SHADER_STORAGE_BUFFER, 16, zero2, GL_DYNAMIC_DRAW); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sbv);
              GLuint pw = prog("#version 430\n layout(location=0) in vec4 p; uniform float zz; void main(){ gl_Position = vec4(p.xy, zz, 1.0); }\n",
                               "#version 430\n layout(early_fragment_tests) in; layout(std430, binding=0) buffer O { uint o[]; }; uniform int slot; void main(){ o[slot] = 1u; }\n");
              glUseProgram(pw);
              glUniform1f(glGetUniformLocation(pw, "zz"), -0.8f); glUniform1i(glGetUniformLocation(pw, "slot"), 0); tri(9);   // right half at depth 0.1 vs stored 1.0 -> passes
              glUniform1f(glGetUniformLocation(pw, "zz"), 0.8f);  glUniform1i(glGetUniformLocation(pw, "slot"), 1); tri(6);   // left half at depth 0.9 vs stored 0.3 -> fails
              glUniform1f(glGetUniformLocation(pw, "zz"), -0.8f); glUniform1i(glGetUniformLocation(pw, "slot"), 2); tri(6);   // left half at depth 0.1 vs 0.3 -> passes
              glMemoryBarrier(0xFFFFFFFF); glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 16, got);
              sprintf(d, "o=%u,%u,%u (expect 1,0,1)", got[0], got[1], got[2]);
              result("T173 early depth test against the copied depth in a colour-less FBO", got[0] == 1 && got[1] == 0 && got[2] == 1, d);
              glUseProgram(g_prog_basic); }
            glColorMask(1, 1, 1, 1); glDepthMask(GL_TRUE);
            // T174: workaround A - CopyImageSubData MSAA depth texture -> MSAA depth renderbuffer, then blit the renderbuffer
            {
                GLuint ft, rbt; glGenFramebuffers(1, &ft); glBindFramebuffer(GL_FRAMEBUFFER, ft);
                glGenRenderbuffers(1, &rbt); glBindRenderbuffer(GL_RENDERBUFFER, rbt); glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, 0x88F0, W, H); glFramebufferRenderbuffer(GL_FRAMEBUFFER, 0x821A, GL_RENDERBUFFER, rbt);
                glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
                while (glGetError()) {}
                glCopyImageSubData(da2, GL_TEXTURE_2D_MULTISAMPLE, 0, 0, 0, 0, rbt, GL_RENDERBUFFER, 0, 0, 0, 0, W, H, 1); GLenum ec = glGetError();
                glBindFramebuffer(GL_READ_FRAMEBUFFER, ft); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb3[0]); glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST); GLenum eb = glGetError();
                float za = -1, zb = -1; glBindFramebuffer(GL_READ_FRAMEBUFFER, fb3[0]); glReadPixels(W / 4, H / 4, 1, 1, 0x1902, GL_FLOAT, &za); glReadPixels(3 * W / 4, 3 * H / 4, 1, 1, 0x1902, GL_FLOAT, &zb);
                sprintf(d, "copyErr=%x blitErr=%x dst: %.3f/%.3f (expect 0.3/1.0)", ec, eb, za, zb);
                result("T174 workaround A: CopyImageSubData to MSAA renderbuffer, then blit", ec == 0 && eb == 0 && za > 0.29f && za < 0.31f && zb > 0.99f, d);
            }
            // T175: workaround B - shader resolve: sample the MSAA depth texture (sample 0) and write gl_FragDepth into the destination
            {
                GLuint pr = prog("#version 430\n layout(location=0) in vec4 p; void main(){ gl_Position = vec4(p.xy, 0.0, 1.0); }\n",
                                 "#version 430\n uniform sampler2DMS dz; void main(){ gl_FragDepth = texelFetch(dz, ivec2(gl_FragCoord.xy), 0).r; }\n");
                glBindFramebuffer(GL_FRAMEBUFFER, fb3[1]); glEnable(GL_DEPTH_TEST); glDepthFunc(GL_ALWAYS); glDepthMask(GL_TRUE); glColorMask(0, 0, 0, 0);
                glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, da2);
                glUseProgram(pr); glUniform1i(glGetUniformLocation(pr, "dz"), 0); tri(0); GLenum er = glGetError();
                float za = -1, zb = -1; glBindFramebuffer(GL_READ_FRAMEBUFFER, fb3[1]); glReadPixels(W / 4, H / 4, 1, 1, 0x1902, GL_FLOAT, &za); glReadPixels(3 * W / 4, 3 * H / 4, 1, 1, 0x1902, GL_FLOAT, &zb);
                sprintf(d, "err=%x dst: %.3f/%.3f (expect 0.3/1.0)", er, za, zb);
                result("T175 workaround B: shader resolve of the MSAA depth texture via gl_FragDepth", er == 0 && za > 0.29f && za < 0.31f && zb > 0.99f, d);
                glUseProgram(g_prog_basic); glColorMask(1, 1, 1, 1); glDepthFunc(GL_LESS); glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
            }
            // T176: is it the stencil? MSAA DEPTH_COMPONENT24 texture -> D24 texture, depth-only blit
            {
                GLuint f2, c2, d2; glGenFramebuffers(1, &f2); glBindFramebuffer(GL_FRAMEBUFFER, f2);
                glGenTextures(1, &c2); glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, c2); glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, W, H, GL_TRUE); glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, c2, 0);
                glGenTextures(1, &d2); glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, d2); glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT24, W, H, GL_TRUE); glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, d2, 0);
                glEnable(GL_DEPTH_TEST); glDepthMask(GL_TRUE); clear_rgba(0, 0, 0, 1); set_color(g_prog_basic, 1, 1, 1, 1); glDepthRange(0.3, 0.3); tri(6); glDepthRange(0, 1);
                GLuint fd2; glGenFramebuffers(1, &fd2); glBindFramebuffer(GL_FRAMEBUFFER, fd2); GLuint td2 = mk_tex2d(GL_DEPTH_COMPONENT24, W, H); glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, td2, 0); glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
                while (glGetError()) {}
                glBindFramebuffer(GL_READ_FRAMEBUFFER, f2); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fd2); glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_DEPTH_BUFFER_BIT, GL_NEAREST); GLenum eb = glGetError();
                float za = -1, zb = -1; glBindFramebuffer(GL_READ_FRAMEBUFFER, fd2); glReadPixels(W / 4, H / 4, 1, 1, 0x1902, GL_FLOAT, &za); glReadPixels(3 * W / 4, 3 * H / 4, 1, 1, 0x1902, GL_FLOAT, &zb);
                sprintf(d, "blitErr=%x dst: %.3f/%.3f (expect 0.3/1.0)", eb, za, zb);
                result("T176 MSAA DEPTH_COMPONENT24 texture -> D24 texture, depth-only blit", eb == 0 && za > 0.29f && za < 0.31f && zb > 0.99f, d);
            }
        }
        glDepthFunc(GL_LESS); glDisable(GL_DEPTH_TEST);
    }
    // clip distance
    GLuint pcl = prog("#version 430\n layout(location=0) in vec4 pos; out float gl_ClipDistance[1]; void main(){ gl_Position = vec4(pos.xy,0,1); gl_ClipDistance[0] = pos.x; }\n", FS_COLOR);
    glUseProgram(pcl); set_color(pcl, 1, 0, 0, 1);
    glEnable(0x3000 /*CLIP_DISTANCE0*/);
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    glDisable(0x3000);
    px(f.fbo, 0, 4, H / 2, p); px(f.fbo, 0, W - 4, H / 2, q);
    sprintf(d, "left=%s right=%s", pxs(p), pxs(q));
    result("T82 gl_ClipDistance clips x<0", isc(p, 0, 0, 0) && isc(q, 255, 0, 0), d);
    // depth clamp
    GLuint pz = prog("#version 430\n layout(location=0) in vec4 pos; uniform float z; void main(){ gl_Position = vec4(pos.xy, z, 1); }\n", FS_COLOR);
    glUseProgram(pz); set_color(pz, 1, 0, 0, 1); glUniform1f(glGetUniformLocation(pz, "z"), 2.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    glEnable(GL_DEPTH_CLAMP); clear_rgba(0, 0, 0, 1); tri(0); glDisable(GL_DEPTH_CLAMP);
    px(f.fbo, 0, W / 2, H / 2, q);
    sprintf(d, "noclamp=%s clamp=%s", pxs(p), pxs(q));
    result("T83 depth clamp (z=2 hidden/shown)", isc(p, 0, 0, 0) && isc(q, 255, 0, 0), d);
    // flat shading / provoking vertex
    GLuint pfl = prog("#version 430\n layout(location=0) in vec4 pos; flat out int vid; void main(){ gl_Position = vec4(pos.xy,0,1); vid = gl_VertexID; }\n",
                      "#version 430\n flat in int vid; out vec4 frag; void main(){ frag = vid==0 ? vec4(1,0,0,1) : vid==1 ? vec4(0,1,0,1) : vec4(0,0,1,1); }\n");
    glUseProgram(pfl);
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    glProvokingVertex(0x8E4D /*FIRST_VERTEX_CONVENTION*/); clear_rgba(0, 0, 0, 1); tri(0); glProvokingVertex(0x8E4E);
    px(f.fbo, 0, W / 2, H / 2, q);
    sprintf(d, "last=%s first=%s", pxs(p), pxs(q));
    result("T84 flat interpolation + provoking vertex", isc(p, 0, 0, 255) && isc(q, 255, 0, 0), d);
    // cube map and sRGB texture sampling
    GLuint cm; glGenTextures(1, &cm); glBindTexture(0x8513, cm); glTexStorage2D(0x8513, 1, GL_RGBA8, 2, 2);
    for (int i = 0; i < 6; i++) { unsigned char fc[16]; for (int k = 0; k < 4; k++) { fc[k*4] = i == 0 ? 255 : 0; fc[k*4+1] = i == 2 ? 255 : 0; fc[k*4+2] = i == 4 ? 255 : 0; fc[k*4+3] = 255; }
        glTexSubImage2D(0x8515 + i, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, fc); }
    glTexParameteri(0x8513, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glTexParameteri(0x8513, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLuint pcm = prog(VS_BASIC, "#version 430\n uniform samplerCube c; out vec4 frag; void main(){ frag = texture(c, vec3(0,1,0)); }\n");
    glUseProgram(pcm); glUniform1i(glGetUniformLocation(pcm, "c"), 0);
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T85 cube map +Y face sample (green)", isc(p, 0, 255, 0), pxs(p));
    glBindTexture(0x8513, 0);
    GLuint st = mk_tex2d(GL_SRGB8_ALPHA8, 2, 2);
    unsigned char sv[16]; for (int k = 0; k < 4; k++) { sv[k*4] = 188; sv[k*4+1] = 188; sv[k*4+2] = 188; sv[k*4+3] = 255; }
    glBindTexture(GL_TEXTURE_2D, st); glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, sv);
    GLuint pst = prog(VS_BASIC, "#version 430\n uniform sampler2D t; out vec4 frag; void main(){ frag = texelFetch(t, ivec2(0,0), 0); }\n");
    glUseProgram(pst); glUniform1i(glGetUniformLocation(pst, "t"), 0);
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    sprintf(d, "%s (expect ~128 after sRGB decode)", pxs(p));
    result("T86 sRGB texture decode", p[0] >= 120 && p[0] <= 136, d);
    // mipmap generation: 2x2 texture with red/green/blue/white -> level 1 average
    GLuint mt; glGenTextures(1, &mt); glBindTexture(GL_TEXTURE_2D, mt);
    unsigned char mm[16] = { 255,0,0,255, 0,255,0,255, 0,0,255,255, 255,255,255,255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, mm);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLuint pmm = prog(VS_BASIC, "#version 430\n uniform sampler2D t; out vec4 frag; void main(){ frag = texelFetch(t, ivec2(0,0), 1); }\n");
    glUseProgram(pmm); glUniform1i(glGetUniformLocation(pmm, "t"), 0);
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T87 glGenerateMipmap level 1 (~128,128,128)", isc(p, 128, 128, 128), pxs(p));
    glBindTexture(GL_TEXTURE_2D, 0);
    // lines through the centre
    static const float lv[] = { -1, 0, 0, 1, 1, 0, 0, 1 };
    GLuint lb; glGenBuffers(1, &lb); glBindBuffer(GL_ARRAY_BUFFER, lb); glBufferData(GL_ARRAY_BUFFER, sizeof lv, lv, GL_STATIC_DRAW);
    glBindVertexBuffer(0, lb, 0, 16);
    glUseProgram(g_prog_basic); set_color(g_prog_basic, 1, 0, 0, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 0, 1); glDrawArrays(GL_LINES, 0, 2);
    glBindVertexBuffer(0, g_vbo, 0, 16);
    px(f.fbo, 0, W / 2, H / 2, p); px(f.fbo, 0, W / 2, H / 2 - 1, q);
    sprintf(d, "y=32:%s y=31:%s", pxs(p), pxs(q));
    result("T88 GL_LINES through centre", isc(p, 255, 0, 0) || isc(q, 255, 0, 0), d);
    // polygon offset: fill at depth then coplanar lines with offset must show
    glBindFramebuffer(GL_FRAMEBUFFER, da.fbo); glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
    clear_rgba(0, 0, 0, 1); set_color(g_prog_basic, 0, 0, 1, 1); tri(0);
    glEnable(0x2A02 /*POLYGON_OFFSET_FILL*/); glPolygonOffset(1.0f, 1.0f);
    clear_rgba(0, 0, 0, 1); tri(0);   // fill pushed back
    glDisable(0x2A02);
    glBindVertexBuffer(0, lb, 0, 16); set_color(g_prog_basic, 1, 0, 0, 1); glDrawArrays(GL_LINES, 0, 2); glBindVertexBuffer(0, g_vbo, 0, 16);
    glDepthFunc(GL_LESS); glDisable(GL_DEPTH_TEST);
    px(da.fbo, 0, W / 2, H / 2, p); px(da.fbo, 0, W / 2, H / 2 - 1, q);
    sprintf(d, "y=32:%s y=31:%s", pxs(p), pxs(q));
    result("T89 polygon offset fill, coplanar line on top", isc(p, 255, 0, 0) || isc(q, 255, 0, 0), d);
    glUseProgram(g_prog_basic);
    // T90-T97: uniform-buffer rebinding patterns like a CAD material buffer (same buffer, new offset per draw)
    {
        GLint al = 0; glGetIntegerv(0x8A34, &al); GLsizeiptr o = al > 0 ? al : 256;
        const float cols[4][4] = { { 0, 0, 0, 0 }, { 1, 0, 0, 1 }, { 0, 1, 0, 1 }, { 0, 0, 1, 1 } };
        unsigned char *mb = (unsigned char *)calloc(4, (size_t)o);
        for (int k = 0; k < 4; k++) memcpy(mb + k * o, cols[k], 16);
        GLuint mub; glGenBuffers(1, &mub); glBindBuffer(GL_UNIFORM_BUFFER, mub); glBufferData(GL_UNIFORM_BUFFER, 4 * o, mb, GL_DYNAMIC_DRAW);
        GLuint mub2; glGenBuffers(1, &mub2); glBindBuffer(GL_UNIFORM_BUFFER, mub2); glBufferData(GL_UNIFORM_BUFFER, 16, cols[2], GL_DYNAMIC_DRAW);
        GLuint pm = prog(VS_BASIC, "#version 430\n layout(std140, binding=3) uniform M { vec4 c; }; out vec4 frag; void main(){ frag = c; }\n");
        GLuint pm2 = prog(VS_BASIC, "#version 430\n layout(std140, binding=3) uniform M { vec4 c; }; out vec4 frag; void main(){ frag = vec4(c.rgb, 1.0); }\n");
        Fbo fm = mk_fbo(GL_RGBA8, 0, 0, 1);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fm.fbo); glDisable(GL_DEPTH_TEST); glDisable(GL_BLEND);
        unsigned char pl[4], pr[4];
#define LR(exp) px(fm.fbo, 0, W / 4, H / 4, pl); px(fm.fbo, 0, 3 * W / 4, H / 4, pr); sprintf(d, "left %d,%d,%d right %d,%d,%d (expect %s)", pl[0], pl[1], pl[2], pr[0], pr[1], pr[2], exp)
        glUseProgram(pm); clear_rgba(0, 0, 0, 1);
        glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, o, 16); tri(6);
        glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, 2 * o, 16); tri(9);
        LR("red / green"); result("T90 UBO rebind same buffer, new offset between draws", isc(pl, 255, 0, 0) && isc(pr, 0, 255, 0), d);
        clear_rgba(0, 0, 0, 1);
        glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, 0, 16); tri(6);
        glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, 3 * o, 16); tri(9);
        LR("black / blue"); result("T91 UBO rebind offset 0 then 3*align", isc(pl, 0, 0, 0) && isc(pr, 0, 0, 255), d);
        clear_rgba(0, 0, 0, 1);
        glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, o, 16); tri(6);
        glBindBuffer(GL_UNIFORM_BUFFER, mub); glBufferSubData(GL_UNIFORM_BUFFER, o, 16, cols[2]); tri(9);
        glBufferSubData(GL_UNIFORM_BUFFER, o, 16, cols[1]);
        LR("red / green"); result("T92 UBO BufferSubData between draws", isc(pl, 255, 0, 0) && isc(pr, 0, 255, 0), d);
        clear_rgba(0, 0, 0, 1);
        glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, o, 16); tri(6);
        { void *mp = glMapBufferRange(GL_UNIFORM_BUFFER, o, 16, 0x0002 | 0x0004); if (mp) memcpy(mp, cols[3], 16); glUnmapBuffer(GL_UNIFORM_BUFFER); }
        tri(9);
        glBufferSubData(GL_UNIFORM_BUFFER, o, 16, cols[1]);
        LR("red / blue"); result("T93 UBO MapBufferRange(write|invalidate range) between draws", isc(pl, 255, 0, 0) && isc(pr, 0, 0, 255), d);
        clear_rgba(0, 0, 0, 1);
        glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, o, 16); tri(6);
        glBindBufferBase(GL_UNIFORM_BUFFER, 3, mub2); tri(9);
        LR("red / green"); result("T94 UBO switch to another buffer between draws", isc(pl, 255, 0, 0) && isc(pr, 0, 255, 0), d);
        clear_rgba(0, 0, 0, 1);
        glUseProgram(pm); glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, o, 16); tri(6);
        glUseProgram(pm2); glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, 2 * o, 16); tri(9);
        LR("red / green"); result("T95 UBO offset change across programs", isc(pl, 255, 0, 0) && isc(pr, 0, 255, 0), d);
        clear_rgba(0, 0, 0, 1); glUseProgram(pm);
        for (int k = 0; k < 64; k++) { glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, (k % 4) * o, 16); tri(k % 2 ? 9 : 6); }
        LR("green / blue"); result("T96 64 UBO rebinds in one frame (last two draws)", isc(pl, 0, 255, 0) && isc(pr, 0, 0, 255), d);
        clear_rgba(0, 0, 0, 1);
        glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, o, 16); tri(6);
        glBindBuffer(GL_UNIFORM_BUFFER, mub); glBufferData(GL_UNIFORM_BUFFER, 4 * o, NULL, GL_DYNAMIC_DRAW);
        for (int k = 0; k < 4; k++) memcpy(mb + k * o, cols[(k + 1) % 4], 16);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * o, mb); tri(9);
        LR("red / green"); result("T97 UBO orphan+refill between draws (range still bound)", isc(pl, 255, 0, 0) && isc(pr, 0, 255, 0), d);
        // T98-T104: update patterns 3DEXPERIENCE uses (invalidate, copy, clear, MapBuffer, BufferData) between draws
        unsigned char *mb0 = (unsigned char *)calloc(4, (size_t)o);
        for (int k = 0; k < 4; k++) memcpy(mb0 + k * o, cols[k], 16);
#define RESTORE() glBindBuffer(GL_UNIFORM_BUFFER, mub); glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * o, mb0); glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, o, 16); clear_rgba(0, 0, 0, 1); tri(6)
        RESTORE(); glInvalidateBufferSubData(mub, o, 16); glBufferSubData(GL_UNIFORM_BUFFER, o, 16, cols[2]); tri(9);
        LR("red / green"); result("T98 InvalidateBufferSubData + BufferSubData between draws", isc(pl, 255, 0, 0) && isc(pr, 0, 255, 0), d);
        RESTORE(); glInvalidateBufferData(mub); memcpy(mb + o, cols[3], 16); memcpy(mb, cols[0], 16); memcpy(mb + 2 * o, cols[2], 16); memcpy(mb + 3 * o, cols[3], 16); glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * o, mb); tri(9);
        LR("red / blue"); result("T99 InvalidateBufferData + full BufferSubData between draws", isc(pl, 255, 0, 0) && isc(pr, 0, 0, 255), d);
        RESTORE(); glInvalidateBufferSubData(mub, o, 16);
        { void *mp = glMapBufferRange(GL_UNIFORM_BUFFER, o, 16, 0x0002 | 0x0020); if (mp) memcpy(mp, cols[2], 16); glUnmapBuffer(GL_UNIFORM_BUFFER); } tri(9);
        LR("red / green"); result("T100 InvalidateBufferSubData + MapBufferRange(unsynchronized)", isc(pl, 255, 0, 0) && isc(pr, 0, 255, 0), d);
        GLuint stg; glGenBuffers(1, &stg); glBindBuffer(0x8F36 /*COPY_READ*/, stg); glBufferData(0x8F36, 16, cols[3], GL_STATIC_DRAW);
        RESTORE(); glBindBuffer(0x8F36, stg); glBindBuffer(0x8F37 /*COPY_WRITE*/, mub); glCopyBufferSubData(0x8F36, 0x8F37, 0, o, 16); tri(9);
        LR("red / blue"); result("T101 CopyBufferSubData staging->UBO between draws", isc(pl, 255, 0, 0) && isc(pr, 0, 0, 255), d);
        RESTORE(); glClearBufferSubData(GL_UNIFORM_BUFFER, 0x8814 /*RGBA32F*/, o, 16, GL_RGBA, GL_FLOAT, cols[2]); tri(9);
        LR("red / green"); result("T102 ClearBufferSubData on UBO between draws", isc(pl, 255, 0, 0) && isc(pr, 0, 255, 0), d);
        RESTORE(); { void *mp = glMapBuffer(GL_UNIFORM_BUFFER, 0x88B9 /*WRITE_ONLY*/); if (mp) memcpy((char *)mp + o, cols[3], 16); glUnmapBuffer(GL_UNIFORM_BUFFER); } tri(9);
        LR("red / blue"); result("T103 MapBuffer(WRITE_ONLY) whole UBO between draws", isc(pl, 255, 0, 0) && isc(pr, 0, 0, 255), d);
        RESTORE(); memcpy(mb + o, cols[2], 16); glBufferData(GL_UNIFORM_BUFFER, 4 * o, mb, GL_DYNAMIC_DRAW); tri(9);
        LR("red / green"); result("T104 BufferData re-specify with data between draws", isc(pl, 255, 0, 0) && isc(pr, 0, 255, 0), d);
        // T105: same, but with the draw in between going to another FBO (pass switch) and the UBO updated while a different range is bound
        RESTORE(); glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, 2 * o, 16); glBufferSubData(GL_UNIFORM_BUFFER, o, 16, cols[3]); glBindBufferRange(GL_UNIFORM_BUFFER, 3, mub, o, 16); tri(9);
        LR("red / blue"); result("T105 BufferSubData while another range of the UBO is bound", isc(pl, 255, 0, 0) && isc(pr, 0, 0, 255), d);
#undef RESTORE
#undef LR
        // T106-T109: query results written into a bound QUERY_BUFFER (ARB_query_buffer_object), as used for GPU occlusion culling
        {
            int adv = 0; GLint ne = 0; glGetIntegerv(0x821D /*NUM_EXTENSIONS*/, &ne);
            for (GLint i = 0; i < ne; i++) { const char *e = (const char *)glGetStringi(GL_EXTENSIONS, (GLuint)i); if (e && strcmp(e, "GL_ARB_query_buffer_object") == 0) adv = 1; }
            GLuint q; glGenQueries(1, &q);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fm.fbo); glUseProgram(g_prog_basic); set_color(g_prog_basic, 1, 1, 1, 1);
            clear_rgba(0, 0, 0, 1);
            glBeginQuery(GL_SAMPLES_PASSED, q); tri(0); glEndQuery(GL_SAMPLES_PASSED);
            GLuint cpu = 0; glGetQueryObjectuiv(q, 0x8866 /*QUERY_RESULT*/, &cpu);
            unsigned zero[8] = { 0 };
            sprintf(d, "GL_ARB_query_buffer_object advertised=%d cpu result=%u", adv, cpu);
            result("T106a query buffer object advertised (info)", 1, d);
            if (!adv) { result("T106-T110 skipped: QUERY_BUFFER not advertised (writing through it crashes)", 1, ""); goto qb_done; }
            GLuint qb; glGenBuffers(1, &qb); glBindBuffer(0x9192 /*QUERY_BUFFER*/, qb); glBufferData(0x9192, 32, zero, 0x88E9);
            glGetQueryObjectuiv(q, 0x8866, (GLuint *)(uintptr_t)4);           // result -> buffer offset 4
            glGetQueryObjectuiv(q, 0x8867 /*QUERY_RESULT_AVAILABLE*/, (GLuint *)(uintptr_t)8);
            glGetQueryObjectui64v(q, 0x8866, (unsigned long long *)(uintptr_t)16);
            glGetQueryObjectuiv(q, 0x9194 /*QUERY_RESULT_NO_WAIT*/, (GLuint *)(uintptr_t)12);
            glBindBuffer(0x9192, 0);
            GLenum e1 = glGetError();
            unsigned out[8] = { 0 }; glBindBuffer(0x8F36, qb); glGetBufferSubData(0x8F36, 0, 32, out);
            sprintf(d, "advertised=%d cpu=%u buf[4]=%u avail=%u nowait=%u u64=%u err=%x (expect buf[4]==cpu==%d)", adv, cpu, out[1], out[2], out[3], out[4], e1, W * H);
            result("T106 QUERY_BUFFER: GetQueryObjectuiv writes result into buffer", out[1] == cpu && cpu == (GLuint)(W * H), d);
            result("T107 QUERY_BUFFER: QUERY_RESULT_AVAILABLE written (1)", out[2] == 1, d);
            result("T108 QUERY_BUFFER: ui64 result written", out[4] == cpu, d);
            result("T109 QUERY_BUFFER: QUERY_RESULT_NO_WAIT written after wait", out[3] == cpu, d);
            // T110: same through a persistent mapped buffer when ARB_buffer_storage is present (the adapter emulates it)
            if (glBufferStorage) {
                GLuint pb; glGenBuffers(1, &pb); glBindBuffer(0x9192, pb); glBufferStorage(0x9192, 16, zero, 0x0001 | 0x0040 | 0x0080 | 0x0200);
                void *map = glMapBufferRange(0x9192, 0, 16, 0x0001 | 0x0040 | 0x0080);
                glGetQueryObjectuiv(q, 0x8866, (GLuint *)(uintptr_t)0);
                glGetQueryObjectuiv(q, 0x8867, (GLuint *)(uintptr_t)4);
                glBindBuffer(0x9192, 0);
                GLsync s = glFenceSync(0x9117, 0); glClientWaitSync(s, 1, 1000000000ull); glDeleteSync(s);
                unsigned m0 = map ? ((unsigned *)map)[0] : 0, m1 = map ? ((unsigned *)map)[1] : 0;
                sprintf(d, "mapped[0]=%u mapped[1]=%u (expect %u, 1) map=%p", m0, m1, cpu, map);
                result("T110 QUERY_BUFFER persistent-mapped: result visible after fence", m0 == cpu && m1 == 1, d);
            }
        qb_done:;
        }
        // T120-T122: compute shaders with double-precision (dmat4) storage, like 3DEXPERIENCE's world->view matrix pass
        {
            while (glGetError()) {}
            double wm[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 5,6,7,1 };
            float vm[16] = { 2,0,0,0, 0,2,0,0, 0,0,2,0, 0,0,0,1 };
            int vis[4] = { 1, 1, 1, 1 };
            float out[16] = { 0 };
            GLuint sb[3]; glGenBuffers(3, sb);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, sb[0]); glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof wm, wm, GL_STATIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, sb[1]); glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof out, out, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, sb[2]); glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof vis, vis, GL_STATIC_DRAW);
            GLuint ub; glGenBuffers(1, &ub); glBindBuffer(GL_UNIFORM_BUFFER, ub);
            struct { float view[16]; int nb, batch, pad0, pad1; } cbv; memcpy(cbv.view, vm, sizeof vm); cbv.nb = 1; cbv.batch = 1;
            glBufferData(GL_UNIFORM_BUFFER, sizeof cbv, &cbv, GL_STATIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sb[0]); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sb[1]); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sb[2]);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, ub);
            const char *cs_d = "#version 430\n#extension GL_ARB_gpu_shader_fp64 : enable\n layout(local_size_x=16) in;\n"
                "layout(std430, binding=0) readonly buffer W { dmat4 w[]; };\n layout(std430, binding=1) writeonly buffer V { mat4 wv[]; };\n"
                "layout(std430, binding=2) readonly buffer S { int vis[]; };\n layout(std140, binding=0) uniform C { mat4 view; int nb; int batch; };\n"
                "void main(){ int p = int(gl_GlobalInvocationID.x) * batch; for (int i = 0; i < 16; i++) { int f = p + i; if (f < nb && vis[f] != 0) wv[f] = mat4(view * w[f]); } }\n";
            const char *cs_f = "#version 430\n layout(local_size_x=16) in;\n"
                "layout(std430, binding=0) readonly buffer W { mat4 w[]; };\n layout(std430, binding=1) writeonly buffer V { mat4 wv[]; };\n"
                "layout(std430, binding=2) readonly buffer S { int vis[]; };\n layout(std140, binding=0) uniform C { mat4 view; int nb; int batch; };\n"
                "void main(){ int p = int(gl_GlobalInvocationID.x) * batch; for (int i = 0; i < 16; i++) { int f = p + i; if (f < nb && vis[f] != 0) wv[f] = view * w[f]; } }\n";
            GLuint pd = cprog(cs_d);
            GLenum e_link = glGetError();
            glUseProgram(pd); glDispatchCompute(3, 1, 1); GLenum e_disp = glGetError();
            glMemoryBarrier(0xFFFFFFFF);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, sb[1]); glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof out, out);
            sprintf(d, "prog=%u errLink=%x errDispatch=%x out[0]=%g out[12]=%g out[13]=%g (expect 2, 10, 12)", pd, e_link, e_disp, out[0], out[12], out[13]);
            result("T120 compute with dmat4 SSBO (ARB_gpu_shader_fp64) dispatch", e_disp == 0 && out[0] == 2.0f && out[12] == 10.0f, d);
            // float version of the same shader as control
            float wmf[16]; for (int i = 0; i < 16; i++) wmf[i] = (float)wm[i];
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, sb[0]); glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof wmf, wmf, GL_STATIC_DRAW);
            memset(out, 0, sizeof out); glBindBuffer(GL_SHADER_STORAGE_BUFFER, sb[1]); glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof out, out, GL_DYNAMIC_DRAW);
            GLuint pf2 = cprog(cs_f); glUseProgram(pf2); glDispatchCompute(3, 1, 1); GLenum e_disp2 = glGetError();
            glMemoryBarrier(0xFFFFFFFF); glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof out, out);
            sprintf(d, "errDispatch=%x out[0]=%g out[12]=%g (expect 2, 10)", e_disp2, out[0], out[12]);
            result("T121 same compute with mat4 (control)", e_disp2 == 0 && out[0] == 2.0f && out[12] == 10.0f, d);
            // T122/T123: optional external shader test (not shipped); skipped
            FILE *fp = NULL;
            if (fp) {
                fseek(fp, 0, SEEK_END); long n = ftell(fp); fseek(fp, 0, SEEK_SET);
                char *src = (char *)calloc((size_t)n + 1, 1); fread(src, 1, (size_t)n, fp); fclose(fp);
                while (glGetError()) {}
                // the real uniform block: std140 { dmat4 ViewMatrix; int batchSize; int nbMatrices; } = 128 + 16 bytes
                struct { double view[16]; int batch, nb, pad0, pad1; } cbd; memset(&cbd, 0, sizeof cbd);
                cbd.view[0] = cbd.view[5] = cbd.view[10] = 2.0; cbd.view[15] = 1.0; cbd.batch = 1; cbd.nb = 1;
                GLuint ubd; glGenBuffers(1, &ubd); glBindBuffer(GL_UNIFORM_BUFFER, ubd); glBufferData(GL_UNIFORM_BUFFER, sizeof cbd, &cbd, GL_STATIC_DRAW); glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubd);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, sb[0]); glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof wm, wm, GL_STATIC_DRAW);      // dmat4 world (translation 5,6,7)
                memset(out, 0, sizeof out); glBindBuffer(GL_SHADER_STORAGE_BUFFER, sb[1]); glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof out, out, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sb[0]); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sb[1]); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sb[2]);
                GLuint p77 = cprog(src); GLenum e77l = glGetError();
                glUseProgram(p77); glDispatchCompute(3, 1, 1); GLenum e77 = glGetError();
                GLint ok = 0; glGetProgramiv(p77, 0x8B82 /*LINK_STATUS*/, &ok);
                glMemoryBarrier(0xFFFFFFFF); glBindBuffer(GL_SHADER_STORAGE_BUFFER, sb[1]); glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof out, out);
                sprintf(d, "prog=%u link=%d errAfterLink=%x errDispatch=%x out[0]=%g out[5]=%g out[12]=%g out[13]=%g out[15]=%g (expect 2,2,10,12,1)", p77, ok, e77l, e77, out[0], out[5], out[12], out[13], out[15]);
                result("T122 3DEXPERIENCE worldView compute shader: dispatch + result (dmat4 UBO view * dmat4 SSBO world)", ok && e77 == 0 && out[0] == 2.0f && out[12] == 10.0f && out[15] == 1.0f, d);
                // T123: minimal shader with the same dmat4-in-std140-UBO pattern
                GLuint pdu = cprog("#version 430\n#extension GL_ARB_gpu_shader_fp64 : enable\n layout(local_size_x=16) in;\n"
                    "layout(std140, binding=0) uniform C { dmat4 view; int batch; int nb; };\n layout(std430, binding=0) readonly buffer W { dmat4 w[]; };\n layout(std430, binding=1) writeonly buffer V { mat4 wv[]; };\n"
                    "void main(){ if (gl_GlobalInvocationID.x == 0u) { wv[0] = mat4(view * w[0]); wv[1] = mat4(view); wv[2][0] = vec4(float(batch), float(nb), 0, 0); } }\n");
                memset(out, 0, sizeof out); float out3[48]; memset(out3, 0, sizeof out3); glBindBuffer(GL_SHADER_STORAGE_BUFFER, sb[1]); glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof out3, out3, GL_DYNAMIC_DRAW); glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sb[1]);
                glUseProgram(pdu); glDispatchCompute(1, 1, 1); GLenum edu = glGetError(); glMemoryBarrier(0xFFFFFFFF); glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof out3, out3);
                sprintf(d, "err=%x view*w: %g,%g,%g | view: %g,%g,%g,%g | batch,nb: %g,%g (expect 2,10,1 | 2,2,2,1 | 1,1)", edu, out3[0], out3[12], out3[15], out3[16], out3[21], out3[26], out3[31], out3[32], out3[33]);
                result("T123 dmat4 in std140 UBO read by compute", edu == 0 && out3[0] == 2.0f && out3[12] == 10.0f && out3[16] == 2.0f && out3[32] == 1.0f && out3[33] == 1.0f, d);
            }
            glUseProgram(g_prog_basic);
        }
        // T130-T134: shader side effects as used by 3DEXPERIENCE's occlusion culling
        {
            while (glGetError()) {}
            unsigned zero[16] = { 0 }, rb[16];
            GLuint ob; glGenBuffers(1, &ob); glBindBuffer(GL_SHADER_STORAGE_BUFFER, ob); glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof zero, zero, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ob);
            Fbo fd = mk_fbo(0, 0, 0, 2);   // depth-only, no colour attachment
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fd.fbo); glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); glDepthMask(GL_TRUE); glClearDepth(1.0); glClear(GL_DEPTH_BUFFER_BIT);
#define RB(n) memset(rb, 0, sizeof rb); glMemoryBarrier(0xFFFFFFFF); glBindBuffer(GL_SHADER_STORAGE_BUFFER, ob); glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof rb, rb); glBindBuffer(GL_SHADER_STORAGE_BUFFER, ob); glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof zero, zero, GL_DYNAMIC_DRAW)
            // T130: vertex-shader-only program writing an SSBO, one point, depth-only FBO (3DEXPERIENCE's bit-packing pass)
            GLuint pvs = link_prog(compile(GL_VERTEX_SHADER, "#version 430\n layout(std430, binding=0) buffer O { uint o[]; }; void main(){ o[gl_VertexID] = 7u + uint(gl_VertexID); }\n"), 0, NULL, 0);
            glUseProgram(pvs); glDrawArrays(GL_POINTS, 0, 1); GLenum e130 = glGetError();
            RB(); sprintf(d, "o[0]=%u err=%x (expect 7)", rb[0], e130);
            result("T130 VS-only program writes SSBO (1 point, no FS, depth-only FBO)", rb[0] == 7, d);
            // T131: same with gl_Position set off-screen
            GLuint pvs2 = link_prog(compile(GL_VERTEX_SHADER, "#version 430\n layout(std430, binding=0) buffer O { uint o[]; }; void main(){ o[gl_VertexID] = 9u; gl_Position = vec4(5.0, 5.0, 5.0, 1.0); }\n"), 0, NULL, 0);
            glUseProgram(pvs2); glDrawArrays(GL_POINTS, 0, 1); GLenum e131 = glGetError();
            RB(); sprintf(d, "o[0]=%u err=%x (expect 9)", rb[0], e131);
            result("T131 VS-only SSBO write, point clipped away", rb[0] == 9, d);
            // T132: fragment shader with early_fragment_tests writes SSBO, colour mask off, depth mask off, depth-only FBO
            GLuint pfs = prog("#version 430\n layout(location=0) in vec4 p; void main(){ gl_Position = vec4(p.xy, 0.5, 1.0); }\n",
                              "#version 430\n layout(early_fragment_tests) in; layout(std430, binding=0) buffer O { uint o[]; }; void main(){ o[1] = 1u; }\n");
            glColorMask(0, 0, 0, 0); glDepthMask(GL_FALSE); glDepthFunc(GL_LEQUAL);
            glUseProgram(pfs); tri(0); GLenum e132 = glGetError();
            RB(); sprintf(d, "o[1]=%u err=%x (expect 1)", rb[1], e132);
            result("T132 FS early_fragment_tests + SSBO write, cmask 0, dmask 0, depth-only FBO", rb[1] == 1, d);
            // T133: same but the geometry is behind the stored depth (0.5 stored by a prior write? we store 0.3 first) -> must NOT write
            glDepthMask(GL_TRUE); glUseProgram(g_prog_basic);
            { GLuint pz = prog("#version 430\n layout(location=0) in vec4 p; void main(){ gl_Position = vec4(p.xy, -0.4, 1.0); }\n", "#version 430\n out vec4 f; void main(){ f = vec4(1); }\n"); glUseProgram(pz); tri(0); }
            glDepthMask(GL_FALSE); glUseProgram(pfs); tri(0);
            RB(); sprintf(d, "o[1]=%u (expect 0: occluded, early depth test must discard)", rb[1]);
            result("T133 early_fragment_tests honours depth-only FBO depth (occluded -> no write)", rb[1] == 0, d);
            glDepthMask(GL_TRUE); glClear(GL_DEPTH_BUFFER_BIT); glDepthMask(GL_FALSE);
            // T134: geometry shader with invocations=3, each invocation emits a fullscreen triangle and writes o[invocation]
            GLuint pgs = prog3("#version 430\n void main(){ gl_Position = vec4(0); }\n",
                               "#version 430\n layout(points, invocations=3) in; layout(triangle_strip, max_vertices=3) out; flat out int inv; layout(std430, binding=0) buffer O { uint o[]; };"
                               " void main(){ o[4 + gl_InvocationID] = 11u; inv = gl_InvocationID; gl_Position = vec4(-1,-1,0,1); EmitVertex(); gl_Position = vec4(3,-1,0,1); EmitVertex(); gl_Position = vec4(-1,3,0,1); EmitVertex(); EndPrimitive(); }\n",
                               "#version 430\n layout(early_fragment_tests) in; flat in int inv; layout(std430, binding=0) buffer O { uint o[]; }; void main(){ o[8 + inv] = 1u; }\n");
            glUseProgram(pgs); glDrawArrays(GL_POINTS, 0, 1); GLenum e134 = glGetError();
            RB(); sprintf(d, "gs o[4..6]=%u,%u,%u fs o[8..10]=%u,%u,%u err=%x (expect 11,11,11 / 1,1,1)", rb[4], rb[5], rb[6], rb[8], rb[9], rb[10], e134);
            result("T134 GS invocations=3: SSBO writes from GS and FS per invocation", rb[4] == 11 && rb[5] == 11 && rb[6] == 11 && rb[8] == 1 && rb[9] == 1 && rb[10] == 1, d);
            // T135: samplerBuffer fetch inside a geometry shader (TBO via glTexBufferRange at an offset)
            { GLint talign = 0; glGetIntegerv(0x919F /*TEXTURE_BUFFER_OFFSET_ALIGNMENT*/, &talign); if (talign < 16) talign = 16;
              float *tbd = (float *)calloc((size_t)talign + 32, 1); float tb[8] = { 0, 0, 0, 0, 0.25f, 0.5f, 0.75f, 1.0f };
              memcpy((char *)tbd + talign, tb + 4, 16); memcpy((char *)tbd + talign + 16, tb + 4, 16);   // texel at the aligned offset = (0.25,0.5,0.75,1)
              GLuint tbo, tex; glGenBuffers(1, &tbo); glBindBuffer(GL_TEXTURE_BUFFER, tbo); glBufferData(GL_TEXTURE_BUFFER, talign + 32, tbd, GL_STATIC_DRAW);
              glGenTextures(1, &tex); glActiveTexture(GL_TEXTURE0 + 5); glBindTexture(GL_TEXTURE_BUFFER, tex); glTexBufferRange(GL_TEXTURE_BUFFER, 0x8814 /*RGBA32F*/, tbo, talign, 32);
              GLenum etr = glGetError();
              { GLuint pvt0 = link_prog(compile(GL_VERTEX_SHADER, "#version 430\n layout(binding=5) uniform samplerBuffer t; layout(std430, binding=0) buffer O { uint o[]; }; void main(){ o[12] = uint(texelFetch(t, 0).x * 1000.0); o[13] = uint(textureSize(t)); gl_Position = vec4(5); }\n"), 0, NULL, 0);
                glUseProgram(pvt0); glDrawArrays(GL_POINTS, 0, 1);
                RB(); sprintf(d, "align=%d errTexBufferRange=%x o[12]=%u size=%u (expect 250, 2)", talign, etr, rb[12], rb[13]);
                result("T143 samplerBuffer via TexBufferRange (aligned offset) fetched in VS", rb[12] == 250 && rb[13] == 2, d); }
              { GLuint pft = prog("#version 430\n layout(location=0) in vec4 p; void main(){ gl_Position = vec4(p.xy, 0.0, 1.0); }\n",
                                  "#version 430\n layout(binding=5) uniform samplerBuffer t; layout(std430, binding=0) buffer O { uint o[]; }; void main(){ o[12] = uint(texelFetch(t, 0).x * 1000.0); }\n");
                glDisable(GL_DEPTH_TEST); glUseProgram(pft); tri(0); glEnable(GL_DEPTH_TEST);
                RB(); sprintf(d, "o[12]=%u (expect 250)", rb[12]);
                result("T144 samplerBuffer via TexBufferRange (aligned offset) fetched in FS", rb[12] == 250, d); }
              GLuint pgt = prog3("#version 430\n void main(){ gl_Position = vec4(0); }\n",
                                 "#version 430\n layout(points) in; layout(points, max_vertices=1) out; layout(binding=5) uniform samplerBuffer t; layout(std430, binding=0) buffer O { uint o[]; };"
                                 " void main(){ vec4 v = texelFetch(t, 0); o[12] = uint(v.x * 1000.0); o[13] = uint(v.w * 1000.0); gl_Position = vec4(5); EmitVertex(); EndPrimitive(); }\n",
                                 "#version 430\n void main(){ }\n");
              glUseProgram(pgt); glDrawArrays(GL_POINTS, 0, 1); GLenum e135 = glGetError();
              RB(); sprintf(d, "o[12]=%u o[13]=%u err=%x (expect 250, 1000)", rb[12], rb[13], e135);
              result("T135 samplerBuffer (TexBufferRange offset 16) fetched in GS", rb[12] == 250 && rb[13] == 1000, d);
              // T136: same fetch, but output a visible triangle and keep the result in the FS via a flat varying
              GLuint pgt2 = prog3("#version 430\n void main(){ gl_Position = vec4(0); }\n",
                                  "#version 430\n layout(points) in; layout(triangle_strip, max_vertices=3) out; layout(binding=5) uniform samplerBuffer t; flat out uint vx; layout(std430, binding=0) buffer O { uint o[]; };"
                                  " void main(){ vec4 v = texelFetch(t, 0); vx = uint(v.x * 1000.0); o[14] = vx; gl_Position = vec4(-1,-1,0,1); EmitVertex(); gl_Position = vec4(3,-1,0,1); EmitVertex(); gl_Position = vec4(-1,3,0,1); EmitVertex(); EndPrimitive(); }\n",
                                  "#version 430\n flat in uint vx; layout(std430, binding=0) buffer O { uint o[]; }; void main(){ o[15] = vx; }\n");
              glUseProgram(pgt2); glDrawArrays(GL_POINTS, 0, 1);
              RB(); sprintf(d, "gs o[14]=%u fs o[15]=%u (expect 250, 250)", rb[14], rb[15]);
              result("T136 samplerBuffer fetched in GS, visible output", rb[14] == 250 && rb[15] == 250, d);
              // T137: samplerBuffer bound with glTexBuffer (whole buffer) instead of a range
              glTexBuffer(GL_TEXTURE_BUFFER, 0x8814, tbo);
              glUseProgram(pgt2); glDrawArrays(GL_POINTS, 0, 1);
              RB(); sprintf(d, "gs o[14]=%u fs o[15]=%u (expect 0 (first texel) -> uses texel 1: see next)", rb[14], rb[15]);
              result("T137 samplerBuffer via glTexBuffer fetched in GS (texel 0 = 0 expected)", rb[14] == 0 && rb[15] == 0, d);
              { GLuint pgt3 = prog3("#version 430\n void main(){ gl_Position = vec4(0); }\n",
                                    "#version 430\n layout(points) in; layout(triangle_strip, max_vertices=3) out; layout(binding=5) uniform samplerBuffer t; layout(std430, binding=0) buffer O { uint o[]; };"
                                    " void main(){ vec4 v = texelFetch(t, 1); o[14] = uint(v.x * 1000.0); o[15] = uint(textureSize(t)); gl_Position = vec4(-1,-1,0,1); EmitVertex(); gl_Position = vec4(3,-1,0,1); EmitVertex(); gl_Position = vec4(-1,3,0,1); EmitVertex(); EndPrimitive(); }\n",
                                    "#version 430\n void main(){ }\n");
                glUseProgram(pgt3); glDrawArrays(GL_POINTS, 0, 1);
                RB(); sprintf(d, "texel1.x=%u size=%u (expect 250, 2)", rb[14], rb[15]);
                result("T138 samplerBuffer texel 1 + textureSize in GS (glTexBuffer)", rb[14] == 250 && rb[15] == 2, d); }
              // T139: the same samplerBuffer fetched in a vertex shader (control)
              { GLuint pvt = link_prog(compile(GL_VERTEX_SHADER, "#version 430\n layout(binding=5) uniform samplerBuffer t; layout(std430, binding=0) buffer O { uint o[]; }; void main(){ o[14] = uint(texelFetch(t, 1).x * 1000.0); gl_Position = vec4(5); }\n"), 0, NULL, 0);
                glUseProgram(pvt); glDrawArrays(GL_POINTS, 0, 1);
                RB(); sprintf(d, "o[14]=%u (expect 250)", rb[14]);
                result("T139 samplerBuffer fetched in VS (control)", rb[14] == 250, d); }
              // T140: sampler2D texelFetch in GS (is it only buffer textures?)
              { unsigned char px4[4] = { 64, 128, 192, 255 }; GLuint t2; glGenTextures(1, &t2); glActiveTexture(GL_TEXTURE0 + 6); glBindTexture(GL_TEXTURE_2D, t2);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px4); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                GLuint pg2d = prog3("#version 430\n void main(){ gl_Position = vec4(0); }\n",
                                    "#version 430\n layout(points) in; layout(points, max_vertices=1) out; layout(binding=6) uniform sampler2D t; layout(std430, binding=0) buffer O { uint o[]; };"
                                    " void main(){ o[14] = uint(texelFetch(t, ivec2(0), 0).x * 255.0 + 0.5); gl_Position = vec4(5); EmitVertex(); EndPrimitive(); }\n",
                                    "#version 430\n void main(){ }\n");
                glUseProgram(pg2d); glDrawArrays(GL_POINTS, 0, 1);
                RB(); sprintf(d, "o[14]=%u (expect 64)", rb[14]);
                result("T140 sampler2D texelFetch in GS", rb[14] == 64, d); glActiveTexture(GL_TEXTURE0); }
              // T141: GS without invocations, emitting 3 primitives in a loop (workaround shape for T134)
              { GLuint pgl = prog3("#version 430\n void main(){ gl_Position = vec4(0); }\n",
                                   "#version 430\n layout(points) in; layout(triangle_strip, max_vertices=9) out; flat out int inv; layout(std430, binding=0) buffer O { uint o[]; };"
                                   " void main(){ for (int k = 0; k < 3; k++) { o[4 + k] = 12u; inv = k; gl_Position = vec4(-1,-1,0,1); EmitVertex(); gl_Position = vec4(3,-1,0,1); EmitVertex(); gl_Position = vec4(-1,3,0,1); EmitVertex(); EndPrimitive(); } }\n",
                                   "#version 430\n layout(early_fragment_tests) in; flat in int inv; layout(std430, binding=0) buffer O { uint o[]; }; void main(){ o[8 + inv] = 2u; }\n");
                glUseProgram(pgl); glDrawArrays(GL_POINTS, 0, 1);
                RB(); sprintf(d, "gs o[4..6]=%u,%u,%u fs o[8..10]=%u,%u,%u (expect 12,12,12 / 2,2,2)", rb[4], rb[5], rb[6], rb[8], rb[9], rb[10]);
                result("T141 GS loop emitting 3 primitives (no invocations): FS sees all", rb[4] == 12 && rb[5] == 12 && rb[6] == 12 && rb[8] == 2 && rb[9] == 2 && rb[10] == 2, d); }
              // T142: invocations=3 but FS writes colour instead of SSBO (does anything rasterize?)
              { Fbo fc = mk_fbo(GL_RGBA8, 0, 0, 1); glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fc.fbo); glColorMask(1, 1, 1, 1); glDisable(GL_DEPTH_TEST); clear_rgba(0, 0, 0, 1);
                GLuint pgi = prog3("#version 430\n void main(){ gl_Position = vec4(0); }\n",
                                   "#version 430\n layout(points, invocations=3) in; layout(triangle_strip, max_vertices=3) out; flat out int inv;"
                                   " void main(){ inv = gl_InvocationID; float x0 = -1.0 + float(gl_InvocationID) * 0.6667; gl_Position = vec4(x0,-1,0,1); EmitVertex(); gl_Position = vec4(x0 + 0.6667,-1,0,1); EmitVertex(); gl_Position = vec4(x0,1,0,1); EmitVertex(); EndPrimitive(); }\n",
                                   "#version 430\n flat in int inv; out vec4 f; void main(){ f = vec4(inv == 0 ? 1.0 : 0.0, inv == 1 ? 1.0 : 0.0, inv == 2 ? 1.0 : 0.0, 1.0); }\n");
                glUseProgram(pgi); glDrawArrays(GL_POINTS, 0, 1);
                unsigned char c0[4], c1[4], c2[4]; px(fc.fbo, 0, 2, 2, c0); px(fc.fbo, 0, W / 2 - 4, 2, c1); px(fc.fbo, 0, W - 10, 2, c2);
                sprintf(d, "inv0=%s inv1=%s inv2=%s (expect red, green, blue)", pxs(c0), pxs(c1), pxs(c2));
                result("T142 GS invocations=3 colour output per invocation", isc(c0, 255, 0, 0) && isc(c1, 0, 255, 0) && isc(c2, 0, 0, 255), d);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fd.fbo); glEnable(GL_DEPTH_TEST); glColorMask(0, 0, 0, 0); glDepthMask(GL_FALSE); }
              glActiveTexture(GL_TEXTURE0); }
            // T160-T162: two buffer textures on units 0 and 1 (glActiveTexture + glBindTexture), as 3DEXPERIENCE binds matricesTex/bboxesTex
            { float da[4] = { 0.111f, 0, 0, 0 }, db2[4] = { 0.222f, 0, 0, 0 };
              GLuint ba, bb, ta, tb;
              glGenBuffers(1, &ba); glBindBuffer(GL_TEXTURE_BUFFER, ba); glBufferData(GL_TEXTURE_BUFFER, 16, da, GL_STATIC_DRAW);
              glGenBuffers(1, &bb); glBindBuffer(GL_TEXTURE_BUFFER, bb); glBufferData(GL_TEXTURE_BUFFER, 16, db2, GL_STATIC_DRAW);
              glGenTextures(1, &ta); glGenTextures(1, &tb);
              glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_BUFFER, ta); glTexBuffer(GL_TEXTURE_BUFFER, 0x8814, ba);
              glActiveTexture(GL_TEXTURE0 + 1); glBindTexture(GL_TEXTURE_BUFFER, tb); glTexBuffer(GL_TEXTURE_BUFFER, 0x8814, bb);
              GLint q0 = 0, q1 = 0; glActiveTexture(GL_TEXTURE0); glGetIntegerv(0x8C2C, &q0); glActiveTexture(GL_TEXTURE0 + 1); glGetIntegerv(0x8C2C, &q1); glActiveTexture(GL_TEXTURE0);
              sprintf(d, "unit0=%d unit1=%d (expect %u, %u)", q0, q1, ta, tb);
              result("T160 TEXTURE_BINDING_BUFFER query per unit", q0 == (GLint)ta && q1 == (GLint)tb, d);
              GLuint pg2 = prog3("#version 430\n void main(){ gl_Position = vec4(0); }\n",
                                 "#version 430\n layout(points) in; layout(points, max_vertices=1) out; layout(binding=0) uniform samplerBuffer m; layout(binding=1) uniform samplerBuffer b; layout(std430, binding=0) buffer O { uint o[]; };"
                                 " void main(){ o[0] = uint(texelFetch(m, 0).x * 1000.0); o[1] = uint(texelFetch(b, 0).x * 1000.0); gl_Position = vec4(5); EmitVertex(); EndPrimitive(); }\n",
                                 "#version 430\n void main(){ }\n");
              glUseProgram(pg2); glDrawArrays(GL_POINTS, 0, 1);
              RB(); sprintf(d, "unit0->%u unit1->%u (expect 111, 222)", rb[0], rb[1]);
              result("T161 two samplerBuffers (binding=0/1) fetched in GS", rb[0] == 111 && rb[1] == 222, d);
              // same in a VS (control) and with explicit glUniform1i instead of layout(binding)
              GLuint pv2 = link_prog(compile(GL_VERTEX_SHADER, "#version 430\n uniform samplerBuffer m; uniform samplerBuffer b; layout(std430, binding=0) buffer O { uint o[]; }; void main(){ o[0] = uint(texelFetch(m, 0).x * 1000.0); o[1] = uint(texelFetch(b, 0).x * 1000.0); gl_Position = vec4(5); }\n"), 0, NULL, 0);
              glUseProgram(pv2); glUniform1i(glGetUniformLocation(pv2, "m"), 0); glUniform1i(glGetUniformLocation(pv2, "b"), 1); glDrawArrays(GL_POINTS, 0, 1);
              RB(); sprintf(d, "unit0->%u unit1->%u (expect 111, 222)", rb[0], rb[1]);
              result("T162 two samplerBuffers (glUniform1i 0/1) fetched in VS", rb[0] == 111 && rb[1] == 222, d);
              glActiveTexture(GL_TEXTURE0 + 1); glBindTexture(GL_TEXTURE_BUFFER, 0); glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_BUFFER, 0); }
            // T165-T168: does a buffer texture follow later changes of its buffer (BufferSubData, BufferData re-specification, orphaning, growth)?
            { float va[4] = { 0.1f, 0, 0, 0 }, vb[4] = { 0.2f, 0, 0, 0 }, vc[4] = { 0.3f, 0, 0, 0 }, vd[4] = { 0.4f, 0, 0, 0 };
              GLuint bx, tx; glGenBuffers(1, &bx); glBindBuffer(GL_TEXTURE_BUFFER, bx); glBufferData(GL_TEXTURE_BUFFER, 16, va, GL_DYNAMIC_DRAW);
              glGenTextures(1, &tx); glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_BUFFER, tx); glTexBuffer(GL_TEXTURE_BUFFER, 0x8814, bx);
              GLuint pvt = link_prog(compile(GL_VERTEX_SHADER, "#version 430\n layout(binding=0) uniform samplerBuffer t; layout(std430, binding=0) buffer O { uint o[]; }; void main(){ o[0] = uint(texelFetch(t, 0).x * 1000.0 + 0.5); o[1] = uint(textureSize(t)); gl_Position = vec4(5); }\n"), 0, NULL, 0);
              glUseProgram(pvt); glDrawArrays(GL_POINTS, 0, 1); RB(); unsigned r0 = rb[0];
              glBindBuffer(GL_TEXTURE_BUFFER, bx); glBufferSubData(GL_TEXTURE_BUFFER, 0, 16, vb); glDrawArrays(GL_POINTS, 0, 1); RB();
              sprintf(d, "initial=%u after BufferSubData=%u (expect 100, 200)", r0, rb[0]);
              result("T165 buffer texture sees BufferSubData update", r0 == 100 && rb[0] == 200, d);
              glBufferData(GL_TEXTURE_BUFFER, 16, vc, GL_DYNAMIC_DRAW); glDrawArrays(GL_POINTS, 0, 1); RB();
              sprintf(d, "after BufferData(same size)=%u (expect 300)", rb[0]);
              result("T166 buffer texture sees BufferData re-specification (same size)", rb[0] == 300, d);
              glBufferData(GL_TEXTURE_BUFFER, 16, NULL, GL_DYNAMIC_DRAW); glBufferSubData(GL_TEXTURE_BUFFER, 0, 16, vd); glDrawArrays(GL_POINTS, 0, 1); RB();
              sprintf(d, "after orphan+SubData=%u (expect 400)", rb[0]);
              result("T167 buffer texture sees orphaned buffer refill", rb[0] == 400, d);
              float big[8] = { 0.5f, 0, 0, 0, 0.6f, 0, 0, 0 };
              glBufferData(GL_TEXTURE_BUFFER, 32, big, GL_DYNAMIC_DRAW); glDrawArrays(GL_POINTS, 0, 1); RB();
              sprintf(d, "after BufferData(bigger)=%u size=%u (expect 500, 2)", rb[0], rb[1]);
              result("T168 buffer texture follows buffer growth", rb[0] == 500 && rb[1] == 2, d);
              // T169: buffer texture attached BEFORE the buffer gets storage (attach, then BufferData)
              GLuint by, ty; glGenBuffers(1, &by); glBindBuffer(GL_TEXTURE_BUFFER, by); glGenTextures(1, &ty); glBindTexture(GL_TEXTURE_BUFFER, ty); glTexBuffer(GL_TEXTURE_BUFFER, 0x8814, by);
              glBufferData(GL_TEXTURE_BUFFER, 16, vb, GL_DYNAMIC_DRAW); glDrawArrays(GL_POINTS, 0, 1); RB();
              sprintf(d, "value=%u size=%u (expect 200, 1)", rb[0], rb[1]);
              result("T169 buffer texture attached before the buffer had storage", rb[0] == 200 && rb[1] == 1, d);
              glBindTexture(GL_TEXTURE_BUFFER, 0); }
#undef RB
            glColorMask(1, 1, 1, 1); glDepthMask(GL_TRUE); glDepthFunc(GL_LESS); glDisable(GL_DEPTH_TEST); glUseProgram(g_prog_basic);
        }
        glBindBufferBase(GL_UNIFORM_BUFFER, 3, 0); glEnable(GL_DEPTH_TEST); glUseProgram(g_prog_basic);
    }
}

static void t_pipelines(void)
{
    Fbo f = mk_fbo(GL_RGBA8, 0, 0, 0);
    unsigned char p[4];
    const char *vs = "#version 430\n layout(location=0) in vec4 pos; out gl_PerVertex { vec4 gl_Position; }; layout(location=3) out vec2 uv;\n"
                     "void main(){ gl_Position = vec4(pos.xy, 0, 1); uv = pos.xy; }\n";
    const char *fs = "#version 430\n layout(location=3) in vec2 uv; uniform vec4 color; out vec4 frag; void main(){ frag = color; }\n";
    GLuint pv = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vs);
    GLuint pf = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fs);
    GLint okv = 0, okf = 0; glGetProgramiv(pv, GL_LINK_STATUS, &okv); glGetProgramiv(pf, GL_LINK_STATUS, &okf);
    if (!okv || !okf) { char log[1024]; GLsizei n; glGetProgramInfoLog(okv ? pf : pv, 1023, &n, log); log[n] = 0; printf("  separable link failed: %s\n", log); }
    GLuint pipe; glGenProgramPipelines(1, &pipe);
    glUseProgramStages(pipe, GL_VERTEX_SHADER_BIT, pv);
    glUseProgramStages(pipe, GL_FRAGMENT_SHADER_BIT, pf);
    glProgramUniform4f(pf, glGetUniformLocation(pf, "color"), 0, 1, 0, 1);
    glUseProgram(0); glBindProgramPipeline(pipe);
    glValidateProgramPipeline(pipe);
    GLint val = 0; glGetProgramPipelineiv(pipe, 0x8B83 /*VALIDATE_STATUS*/, &val);
    clear_rgba(0, 0, 1, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    char d[96]; sprintf(d, "%s validate=%d", pxs(p), val);
    result("T45 separate shader objects pipeline", isc(p, 0, 255, 0), d);
    // change the uniform after binding, draw again
    glProgramUniform4f(pf, glGetUniformLocation(pf, "color"), 1, 1, 0, 1);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.fbo); clear_rgba(0, 0, 1, 1); tri(0);
    px(f.fbo, 0, W / 2, H / 2, p);
    result("T46 glProgramUniform update on pipeline", isc(p, 255, 255, 0), pxs(p));
    glBindProgramPipeline(0); glUseProgram(g_prog_basic);
}

// second context renders into a shared texture, first context samples it
static void t_sharing(HDC dc, HGLRC main, HGLRC other)
{
    unsigned char p[4]; char d[96];
    if (!other) { result("T47 cross-context texture (no ctx)", 0, "no shared context"); return; }
    GLuint shared = mk_tex2d(GL_RGBA8, W, H);
    GLuint ps = prog(VS_BASIC, "#version 430\n uniform sampler2D t; out vec4 frag; void main(){ frag = texelFetch(t, ivec2(gl_FragCoord.xy), 0); }\n");
    Fbo f = mk_fbo(GL_RGBA8, 0, 0, 0);
    for (int mode = 0; mode < 3; mode++) {
        // clear the shared texture to black from the main context first
        glBindTexture(GL_TEXTURE_2D, shared);
        unsigned char *blk = (unsigned char *)calloc(W * H * 4, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, blk); free(blk);
        glFinish();
        wglMakeCurrent(dc, other);
        GLuint fb2; glGenFramebuffers(1, &fb2); glBindFramebuffer(GL_FRAMEBUFFER, fb2);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shared, 0);
        glViewport(0, 0, W, H); glClearColor(1, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT);
        GLsync s = NULL;
        if (mode == 0) glFlush();
        else if (mode == 1) s = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0), glFlush();
        else s = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);   // no flush: relies on the driver
        wglMakeCurrent(dc, main);
        if (mode == 1) glWaitSync(s, 0, GL_TIMEOUT_IGNORED);
        if (mode == 2) { GLenum r = glClientWaitSync(s, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000ull); (void)r; }
        glBindFramebuffer(GL_FRAMEBUFFER, f.fbo); glViewport(0, 0, W, H);
        glUseProgram(ps); glBindTexture(GL_TEXTURE_2D, shared); glUniform1i(glGetUniformLocation(ps, "t"), 0);
        clear_rgba(0, 0, 1, 1); tri(0);
        px(f.fbo, 0, W / 2, H / 2, p);
        sprintf(d, "%s", pxs(p));
        result(mode == 0 ? "T47 cross-context texture, glFlush" : mode == 1 ? "T48 cross-context, fence + glWaitSync" : "T49 cross-context, fence + ClientWaitSync", isc(p, 255, 0, 0), d);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(g_prog_basic);
}

static void t_limits(void)
{
    struct { GLenum e; const char *n; } lim[] = {
        { 0x8A30, "MAX_UNIFORM_BLOCK_SIZE" }, { 0x90DE, "MAX_SHADER_STORAGE_BLOCK_SIZE" }, { 0x8C2B, "MAX_TEXTURE_BUFFER_SIZE" },
        { 0x82DA, "MAX_VERTEX_ATTRIB_BINDINGS" }, { 0x8869, "MAX_VERTEX_ATTRIBS" }, { 0x8CDF, "MAX_COLOR_ATTACHMENTS" },
        { 0x8D57, "MAX_SAMPLES" }, { 0x8824, "MAX_DRAW_BUFFERS" }, { 0x8A34, "UNIFORM_BUFFER_OFFSET_ALIGNMENT" },
        { 0x90DF, "SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT" }, { 0x8B4A, "MAX_VERTEX_UNIFORM_COMPONENTS" },
        { 0x8A2D, "MAX_FRAGMENT_UNIFORM_BLOCKS" }, { 0x8A2B, "MAX_VERTEX_UNIFORM_BLOCKS" }, { 0x90DB, "MAX_FRAGMENT_SHADER_STORAGE_BLOCKS" },
        { 0x90D6, "MAX_VERTEX_SHADER_STORAGE_BLOCKS" }, { 0x8B4D, "MAX_COMBINED_TEXTURE_IMAGE_UNITS" }, { 0x0D33, "MAX_TEXTURE_SIZE" },
        { 0x88FF, "MAX_ARRAY_TEXTURE_LAYERS" }, { 0x8073, "MAX_3D_TEXTURE_SIZE" }, { 0x91BB, "MAX_COMPUTE_WORK_GROUP_INVOCATIONS" },
        { 0x8265, "MAX_COMPUTE_SHARED_MEMORY_SIZE" }, { 0x92D7, "MAX_FRAGMENT_ATOMIC_COUNTERS" }, { 0x8F39, "MAX_IMAGE_UNITS" },
        { 0x8DFB, "MAX_VARYING_VECTORS" }, { 0x80E9, "MAX_ELEMENTS_INDICES" }, { 0x80E8, "MAX_ELEMENTS_VERTICES" },
        { 0x8E8A, "MAX_PATCH_VERTICES" }, { 0x8E5E, "MIN_PROGRAM_TEXTURE_GATHER_OFFSET" }, { 0x82E5, "MAX_VERTEX_ATTRIB_STRIDE" },
        { 0x9122, "MAX_VERTEX_OUTPUT_COMPONENTS" }, { 0x9125, "MAX_FRAGMENT_INPUT_COMPONENTS" }, { 0x8DE1, "MAX_GEOMETRY_OUTPUT_VERTICES" },
        { 0x9100, "TEXTURE_2D_MULTISAMPLE(dummy)" },
    };
    printf("-- limits:");
    for (size_t i = 0; i + 1 < sizeof lim / sizeof lim[0]; i++) {
        GLint v = -1; glGetIntegerv(lim[i].e, &v);
        printf(" %s=%d", lim[i].n, v);
    }
    GLint64 v64 = -1; glGetInteger64v(0x8A30, &v64);
    printf(" (block64=%lld)\n", v64);
    drain();
}

int main(int argc, char **argv)
{
    int profile = (argc > 1 && argv[1][0] == 'c') ? 1 : 2;   // default: compatibility like 3DEXPERIENCE
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = DefWindowProcA; wc.hInstance = GetModuleHandleA(NULL); wc.lpszClassName = "gltest2"; wc.style = CS_OWNDC;
    RegisterClassA(&wc);
    HWND w = CreateWindowA("gltest2", "gltest2", WS_OVERLAPPEDWINDOW, 0, 0, W + 16, H + 40, NULL, NULL, wc.hInstance, NULL);
    HDC dc = GetDC(w);
    PIXELFORMATDESCRIPTOR pfd = { sizeof pfd, 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32 };
    pfd.cDepthBits = 24; pfd.cStencilBits = 8;
    SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd);
    HGLRC legacy = wglCreateContext(dc);
    wglMakeCurrent(dc, legacy);
    PFN_CCA cca = (PFN_CCA)wglGetProcAddress("wglCreateContextAttribsARB");
    int attr[] = { 0x2091, 4, 0x2092, 3, 0x9126, profile, 0 };
    HGLRC ctx = cca ? cca(dc, NULL, attr) : NULL;
    HGLRC ctx2 = ctx && cca ? cca(dc, ctx, attr) : NULL;
    if (!ctx) { printf("context creation failed\n"); return 1; }
    wglMakeCurrent(dc, ctx);
    GLint prof = 0; glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &prof);
    printf("== %s | %s | profile %d | second ctx %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER), prof, ctx2 ? "yes" : "no");
    load();
    if (glGetStringi) {
        GLint ne = 0; glGetIntegerv(0x821D, &ne); printf("== extensions (%d):", ne);
        for (GLint i = 0; i < ne; i++) { const char *e = (const char *)glGetStringi(GL_EXTENSIONS, (GLuint)i); if (e) printf(" %s", e + (strncmp(e, "GL_", 3) == 0 ? 3 : 0)); }
        printf("\n");
    }
    if (g_missing) printf("  (%d functions missing)\n", g_missing);
    drain();
    setup_common();
    t_limits();
    t_basic(); SwapBuffers(dc);
    t_depth(); SwapBuffers(dc);
    t_queries(); SwapBuffers(dc);
    t_instancing(); SwapBuffers(dc);
    t_indirect(); SwapBuffers(dc);
    t_multidraw(); SwapBuffers(dc);
    t_buffers(); SwapBuffers(dc);
    t_targets(); SwapBuffers(dc);
    t_formats(); SwapBuffers(dc);
    t_misc(); SwapBuffers(dc);
    t_pipelines(); SwapBuffers(dc);
    t_sharing(dc, ctx, ctx2); SwapBuffers(dc);
    printf("== done: %d pass, %d fail\n", g_pass, g_fail);
    fflush(stdout);
    return 0;
}

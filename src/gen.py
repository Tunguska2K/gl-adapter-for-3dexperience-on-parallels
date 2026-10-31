#!/usr/bin/env python3
"""Generate an x64 opengl32.dll proxy for 3DEXPERIENCE on Parallels.

Every export jumps to the real C:\\Windows\\System32\\opengl32.dll (loaded
lazily). Exports in OVERRIDDEN are implemented in shim.c. Exports in
SYNC_EXPORTS and the extension functions in SYNC_PROCS first call
shim_sync_before_gpu() (uploads CPU-written persistent buffers) and then
jump to the real function with all argument registers intact.
"""
import pathlib

here = pathlib.Path(__file__).parent
names = [n.strip() for n in (here / 'exports.txt').read_text().split() if n.strip()]

OVERRIDDEN = {'glGetString', 'wglGetProcAddress', 'glGetIntegerv', 'glFinish', 'wglMakeCurrent', 'wglSwapBuffers', 'glDrawArrays', 'glDrawElements', 'glClear'}

# GL 1.1 exports that can read from buffers or hand work to the GPU.
SYNC_EXPORTS = {
    'glDrawArrays', 'glDrawElements', 'glTexImage1D', 'glTexImage2D',
    'glTexSubImage1D', 'glTexSubImage2D', 'glReadPixels', 'glFlush',
    'wglSwapLayerBuffers', 'wglSwapMultipleBuffers',
}

# Extension entry points (via wglGetProcAddress) that consume buffer data.
SYNC_PROCS = [
    'glDrawArraysInstanced', 'glDrawElementsInstanced', 'glDrawRangeElements',
    'glDrawElementsBaseVertex', 'glDrawRangeElementsBaseVertex',
    'glDrawElementsInstancedBaseVertex', 'glDrawArraysInstancedBaseInstance',
    'glDrawElementsInstancedBaseInstance', 'glDrawElementsInstancedBaseVertexBaseInstance',
    'glMultiDrawArrays', 'glMultiDrawElements', 'glMultiDrawElementsBaseVertex',
    'glDrawArraysIndirect', 'glDrawElementsIndirect',
    'glMultiDrawArraysIndirect', 'glMultiDrawElementsIndirect',
    'glMultiDrawArraysIndirectCountARB', 'glMultiDrawElementsIndirectCountARB',
    'glMultiDrawArraysIndirectCount', 'glMultiDrawElementsIndirectCount',
    'glDispatchCompute', 'glDispatchComputeIndirect', 'glDispatchComputeGroupSizeARB',
    'glDrawTransformFeedback', 'glDrawTransformFeedbackStream',
    'glDrawTransformFeedbackInstanced', 'glDrawTransformFeedbackStreamInstanced',
    'glCopyBufferSubData', 'glNamedCopyBufferSubDataEXT',
    'glTexImage3D', 'glTexSubImage3D',
    'glCompressedTexImage1D', 'glCompressedTexImage2D', 'glCompressedTexImage3D',
    'glCompressedTexSubImage1D', 'glCompressedTexSubImage2D', 'glCompressedTexSubImage3D',
    'glTextureSubImage1DEXT', 'glTextureSubImage2DEXT', 'glTextureSubImage3DEXT',
    'glTextureImage1DEXT', 'glTextureImage2DEXT', 'glTextureImage3DEXT',
    'glCompressedTextureSubImage1DEXT', 'glCompressedTextureSubImage2DEXT',
    'glCompressedTextureSubImage3DEXT', 'glGetBufferSubData', 'glGetNamedBufferSubDataEXT',
    'glBlitFramebuffer', 'glCopyImageSubData',
]

SAVE = ['    push rax', '    push rcx', '    push rdx', '    push r8', '    push r9',
        '    sub rsp, 0x60',
        '    movdqu [rsp+0x20], xmm0', '    movdqu [rsp+0x30], xmm1',
        '    movdqu [rsp+0x40], xmm2', '    movdqu [rsp+0x50], xmm3']
RESTORE = ['    movdqu xmm0, [rsp+0x20]', '    movdqu xmm1, [rsp+0x30]',
           '    movdqu xmm2, [rsp+0x40]', '    movdqu xmm3, [rsp+0x50]',
           '    add rsp, 0x60',
           '    pop r9', '    pop r8', '    pop rdx', '    pop rcx', '    pop rax']

asm = ['.intel_syntax noprefix', '.text']
for i, n in enumerate(names):
    if n in OVERRIDDEN:
        continue
    target = 'shim_common_sync' if n in SYNC_EXPORTS else 'shim_common'
    asm += [f'.globl {n}', f'{n}:', f'    mov eax, {i}', f'    jmp {target}']

# optional call trace (debug captures): g_tr_on != 0 -> shim_trace_export(index, saved arg regs)
TRACE_EXPORT = ['    cmp byte ptr [rip + g_tr_on], 0', '    je 3f'] + SAVE + [
        '    mov ecx, eax', '    lea rdx, [rsp + 0x60]', '    call shim_trace_export'] + RESTORE + ['3:']
asm += ['shim_common:'] + TRACE_EXPORT + [
        '    lea r10, [rip + g_tbl]', '    mov r11, [r10 + rax*8]',
        '    test r11, r11', '    jz 1f', '    jmp r11',
        '1:'] + SAVE + ['    call shim_init'] + RESTORE + [
        '    lea r10, [rip + g_tbl]', '    jmp qword ptr [r10 + rax*8]', '']
asm += ['shim_common_sync:'] + TRACE_EXPORT + SAVE + ['    call shim_init', '    call shim_sync_before_gpu'] + RESTORE + [
        '    lea r10, [rip + g_tbl]', '    jmp qword ptr [r10 + rax*8]', '']
# generic trace stubs for extension procs: tr_<k> -> shim_trace_call(k, args), then g_tr_real[k]
TR_COUNT = 2048
for k in range(TR_COUNT):
    asm += [f'.globl tr_{k}', f'tr_{k}:', f'    mov eax, {k}', '    jmp tr_common']
asm += ['tr_common:', '    cmp byte ptr [rip + g_tr_on], 0', '    je 5f'] + SAVE + [
        '    mov ecx, eax', '    lea rdx, [rsp + 0x60]', '    call shim_trace_call'] + RESTORE + ['5:',
        '    lea r10, [rip + g_tr_real]', '    jmp qword ptr [r10 + rax*8]', '']
# pre-hook stubs for extension functions: ph_<k> -> sync, then g_ph_real[k]
for k, n in enumerate(SYNC_PROCS):
    asm += [f'.globl ph_{k}', f'ph_{k}:', f'    mov eax, {k}', '    jmp ph_common']
asm += ['ph_common:'] + SAVE + ['    call shim_sync_before_gpu'] + RESTORE + [
        '    lea r10, [rip + g_ph_real]', '    jmp qword ptr [r10 + rax*8]', '']
(here / 'thunks.S').write_text('\n'.join(asm))

hdr = [f'#define SHIM_COUNT {len(names)}']
for n in ['glGetString', 'wglGetProcAddress', 'glGetIntegerv', 'glBindTexture', 'glFinish', 'glFlush', 'glEnable', 'glDisable', 'glIsEnabled', 'wglMakeCurrent', 'wglSwapBuffers', 'glDrawArrays', 'glDrawElements', 'glClear', 'glGetBooleanv', 'glPolygonMode', 'glPixelStorei', 'glReadPixels', 'glReadBuffer', 'glGenTextures', 'glDeleteTextures', 'glTexImage2D', 'glTexParameteri', 'glViewport', 'glScissor', 'glDepthFunc', 'glDepthMask', 'glColorMask', 'glDepthRange', 'glGetDoublev']:
    hdr.append(f'#define IDX_{n} {names.index(n)}')
hdr.append('static const char *const g_names[SHIM_COUNT] = {')
hdr += [f'    "{n}",' for n in names] + ['};']
hdr.append(f'#define PH_COUNT {len(SYNC_PROCS)}')
hdr.append('static const char *const g_ph_names[PH_COUNT] = {')
hdr += [f'    "{n}",' for n in SYNC_PROCS] + ['};']
hdr += [f'void ph_{k}(void);' for k in range(len(SYNC_PROCS))]
hdr.append('static void (*const g_ph_stubs[PH_COUNT])(void) = {')
hdr += [f'    ph_{k},' for k in range(len(SYNC_PROCS))] + ['};']
hdr.append(f'#define TR_COUNT {TR_COUNT}')
hdr += [f'void tr_{k}(void);' for k in range(TR_COUNT)]
hdr.append('static void (*const g_tr_stubs[TR_COUNT])(void) = {')
hdr += [f'    tr_{k},' for k in range(TR_COUNT)] + ['};']
(here / 'names.h').write_text('\n'.join(hdr) + '\n')

(here / 'opengl32.def').write_text('LIBRARY opengl32\nEXPORTS\n' + ''.join(f'    {n}\n' for n in names))
print(f'{len(names)} exports ({len(SYNC_EXPORTS)} synced), {len(SYNC_PROCS)} synced extension procs')

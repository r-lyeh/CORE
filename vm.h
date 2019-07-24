// vm: argcv/cvars/ini, math/physics, occlusion/ray, level/stream/world, scene/graph, worker/task, ram/lerp, message/event/loop/frameskip
// dummy systems (plugins): audio, database, dev, gameplay, input, network, os, package, power, render, rpc/ipc, script, sysdir, vr, window...

#if 0
#ifdef _WIN
#   define dlopen(name,mode)    (void*)LoadLibraryA( name )
#   define dlsym(handle,symbol) GetProcAddress((HMODULE)handle, symbol )
#   define dlclose(handle)      0
#endif

void* dllfind(const char *filename, const char *symbol) {
    void *dll = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
    if( dll != 0 ) return dlsym(dll, symbol);
/*
    if( file_exists(buf = va("%s", filename)) ||
        file_exists(buf = va("%s.dll", filename)) ||
        file_exists(buf = va("%s.so", filename)) ||
        file_exists(buf = va("lib%s.so", filename)) ||
        file_exists(buf = va("%s.dylib", filename)) ) {
    }
*/
    return 0;
}

extern void (*on_init[256])(int argc, char ** argv);
extern void (*on_draw[256])(int w, int h);
extern void (*on_step[256])(int t, int dt);
extern void (*on_swap[256])(void *pixels);
extern void (*on_quit[256])(void);
//extern void (*on_menu[256])();
//extern void (*on_edit[256])();
//extern void (*on_undo[256])();
//extern void (*on_redo[256])();

int start(int argc, char **argv);
#endif

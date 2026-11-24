#pragma once
#include <stddef.h>
#include <stdint.h>
int main();

struct AppDefinition {
    const char *name;
    void *icon; // MUST be a 64x64 RGB bitmap or NULL
    int (*entry)(void);
    void **userdata_ptr_loc;
    void *ram_base;
    void *userdata_base;

    uint32_t ram_size_def;
    uint32_t ram_size_full;
    uint32_t userdata_size;
    struct AppDefinition *next;
};

struct AppDefinition *env_get_installed_apps();
struct AppDefinition *env_get_app_to_init_to();
void env_commit_userdata(struct AppDefinition *this_app, void *userdata);
void env_load_userdata(struct AppDefinition *this_app, void *userdata);
[[noreturn]] void env_load_application(struct AppDefinition *app);
[[noreturn]] void env_reset();

#ifndef SYSTEM_IMAGE
#ifndef PICOTINKER_APP_ICON
#define PICOTINKER_APP_ICON (void *)0
#endif // PICOTINKER_APP_ICON

#ifdef PICOTINKER_USERDATA_STRUCT
const volatile struct PICOTINKER_USERDATA_STRUCT *USERDATA = 0xA110CF01;
#define PICOTINKER_COMMIT_USERDATA env_commit_userdata(&APPLICATION, USERDATA)
#define __USERDATA_LOC &USERDATA
#else // PICOTINKER_USERDATA_STRUCT
struct _mock {};
#define PICOTINKER_USERDATA_STRUCT _mock
#define __USERDATA_LOC NULL
#endif // else/PICOTINKER_USERDATA_STRUCT

#define PICOTINKER_APP(name)                                                   \
    const struct AppDefinition APPLICATION = {                                 \
        name,                                                                  \
        PICOTINKER_APP_ICON,                                                   \
        &main,                                                                 \
        __USERDATA_LOC,                                                        \
        (void *)0xF00FF00F,                                                    \
        (void *)0xA110CF00,                                                    \
        0,                                                                     \
        0,                                                                     \
        sizeof(struct PICOTINKER_USERDATA_STRUCT),                             \
        (void *)0xDEADBEEF};
#endif // SYSTEM_IMAGE

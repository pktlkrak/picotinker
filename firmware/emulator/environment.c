#include "environment.h"

#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define APPLICATIONS_DIR "./applications"

static struct AppDefinition *ALL_APPS = NULL;
static struct AppDefinition *APP_TO_LOAD = NULL;

static int env_load_single_app(struct AppDefinition **last,
                               const char *filename) {
    printf("Loading application from %s\n", filename);
    void *app = dlopen(filename, RTLD_NOW | RTLD_DEEPBIND);
    if (app == NULL) {
        printf("Failed to dlopen()! %s\n", dlerror());
        return 2;
    }
    struct AppDefinition *in_app_def = dlsym(app, "APPLICATION");
    if (!in_app_def) {
        printf("Not a valid format - APPLICATION is not defined!\n");
        return 1;
    }
    size_t page_mask = ~(getpagesize() - 1);
    bool is_unfortunately_positioned =
        (((size_t)in_app_def) & page_mask) !=
        (((size_t)(in_app_def + sizeof(struct AppDefinition))) & page_mask);
    mprotect((void *)(((size_t)in_app_def) & page_mask),
             (1 + is_unfortunately_positioned) * getpagesize(),
             PROT_READ | PROT_WRITE);
    // Build linked list:
    if (*last)
        (*last)->next = in_app_def;
    *last = in_app_def;
    in_app_def->next = NULL;

    // The struct should have been filled out by the compiler already.
    // that is all.
    return 0;
}

static int pstrcmp(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

static void env_load_all_applications_list() {
    if (ALL_APPS != NULL)
        return;
    DIR *rootDirOfExtensions;
    struct dirent *entry;
    char full_path_name[256];
    strcpy(full_path_name, APPLICATIONS_DIR);
    full_path_name[sizeof(APPLICATIONS_DIR) - 1] = '/';

    struct AppDefinition *last_app_loaded = NULL;
    char **files_to_load = malloc(sizeof(char *) * 256);
    int files_to_load_ptr = 0;

    if ((rootDirOfExtensions = opendir(APPLICATIONS_DIR)) != NULL) {
        while ((entry = readdir(rootDirOfExtensions)) != NULL) {
            if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
                if (files_to_load_ptr < 256)
                    files_to_load[files_to_load_ptr++] = strdup(entry->d_name);
            }
        }
        closedir(rootDirOfExtensions);
    } else {
        return;
    }

    qsort(files_to_load, files_to_load_ptr, sizeof(char *), pstrcmp);

    for (int i = 0; i < files_to_load_ptr; i++) {
        if (strlen(files_to_load[i]) >
            sizeof(full_path_name) - sizeof(APPLICATIONS_DIR) - 2) {
            printf("Skipped due to too long filename (%s).\n",
                   files_to_load[i]);
            continue;
        }
        strcpy(full_path_name + sizeof(APPLICATIONS_DIR), files_to_load[i]);
        free(files_to_load[i]);

        if (env_load_single_app(&last_app_loaded, full_path_name) == 0) {
            if (ALL_APPS == NULL)
                ALL_APPS = last_app_loaded;
        }
    }
    free(files_to_load);
    APP_TO_LOAD = ALL_APPS;
}

struct AppDefinition *env_get_installed_apps() {
    env_load_all_applications_list();
    return ALL_APPS;
}

struct AppDefinition *env_get_app_to_init_to() {
    env_load_all_applications_list();
    return APP_TO_LOAD;
}

void env_commit_userdata(struct AppDefinition *this_app, void *userdata) {}

void env_load_userdata(struct AppDefinition *this_app, void *userdata) {}

extern jmp_buf before_application_loading;
void env_load_application(struct AppDefinition *app) {
    printf("Switching applications...\n");
    APP_TO_LOAD = app;
    // industrial-strength goto
    longjmp(before_application_loading, 1);
}

void env_reset() {
    printf("Can't reset the emulator!!\n");
    exit(0);
}

#ifndef MAIN_H
#define MAIN_H

#include <limits.h>
#include <pthread.h>
#include <stdbool.h>

#include "copy.h"
#include "log.h"
#include "sync.h"
#include "test.h"

#define P_mutex pthread_mutex_lock
#define V_mutex pthread_mutex_unlock

bool a_fini;
bool est_bloque;

void* synchroniser();
void* copier(void* arg);
void *main_update_server(void *arg);
void *main_server_backup(void *arg);
void *main_server_prod(void *arg);
void *main_nouveau_server();

#endif


#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h> 

// Format des logs : [jj/mm/aa-hh:mm:ss][xxxx] : msg
// xxxx : module (copy | sync | test)
// message : maximum 100 octets
void ecrire_log(char* module, char* message);

#endif


#ifndef MODLOAD_H
#define MODLOAD_H

#include "calibpar.h"

/* Load MODL.TXT into CalibParams.
   Returns 1 on success, 0 on failure. */
int load_model_params(const char *filename, CalibParams *p);

#endif

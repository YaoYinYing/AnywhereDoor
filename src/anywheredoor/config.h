#ifndef ANYWHEREDOOR_CONFIG_H
#define ANYWHEREDOOR_CONFIG_H

#include "proxy.h"

/* Load configuration from a .anywheredoorrc file.
   Returns 0 on success, -1 on error.
   Populates *table and *noproxy. Caller must free with proxy_table_free / noproxy_free. */
int config_load(const char *config_path, ProxyTable *table, NoProxy *noproxy);

#endif /* ANYWHEREDOOR_CONFIG_H */

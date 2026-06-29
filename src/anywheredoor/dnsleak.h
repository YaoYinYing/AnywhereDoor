#ifndef ANYWHEREDOOR_DNSLEAK_H
#define ANYWHEREDOOR_DNSLEAK_H

/* Run the full DNS leak test using the bash.ws service.
   Fetches a leak ID, pings domains, retrieves and displays results.
   Returns 0 on success, -1 on error. */
int dns_leak_test(void);

#endif /* ANYWHEREDOOR_DNSLEAK_H */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "common/types.h"

#define CPUINFO_PROC "/proc/cpuinfo"
static int g_cpukhz = 0;

int get_cpukhz(void)
{
#define LINE_LEN 1024
#define MATCH_STR "cpu MHz"
    int cpu_khz = -1;
    char strline[LINE_LEN] = {'\0'};
    char *pline = NULL;

    if (g_cpukhz > 0) {
        return g_cpukhz;
    }

    FILE *pfile = fopen(CPUINFO_PROC, "r");
    if (pfile == NULL) {
        return -1;
    }

    while (memset(strline, 0, LINE_LEN) != NULL && (pline = fgets(strline, LINE_LEN, pfile)) != NULL) {
        if (strncasecmp(pline, MATCH_STR, strlen(MATCH_STR)) == 0) {
            pline += strlen(MATCH_STR);

            // "cpu MHz             : 1695.357"
            // skip tab, space, :
            while ((*pline == '\t') || (*pline == 0x20) || (*pline == ':')) {
                ++pline;
            }

            cpu_khz = (int) (atof(pline) * 1000);
            if (cpu_khz == 0) {
                cpu_khz = -1;
            }
            break;
        }
    }

    (void) fclose(pfile);
    g_cpukhz = cpu_khz;

    return cpu_khz;
}

uint64_t cputick2ms(uint64_t diff_track)
{
    return (diff_track / g_cpukhz);
}

uint64_t cputick2ns(uint64_t diff_track)
{
    return ((diff_track * 1000) / g_cpukhz);
}


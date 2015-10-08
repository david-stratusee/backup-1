
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

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

unsigned long cputick2ms(unsigned long diff_track)
{
    return (diff_track / g_cpukhz);
}

unsigned long cputick2ns(unsigned long diff_track)
{
    return ((diff_track * 1000) / g_cpukhz);
}

#define CPU_PREFIX "cpu cores"
int get_cpu_num(void)
{
    char *cpu_filename = "/proc/cpuinfo";    /* input-file name    */
#define CPU_LINE_LEN 1024
    char readline[CPU_LINE_LEN] = {0};
    char *pline = readline;
    char *pline_end = NULL;

    FILE *fp_cpu = fopen((const char *)cpu_filename, "r");
    if (fp_cpu == NULL) {
        fprintf (stderr, "couldn't open file '%s'; %s\n",
                cpu_filename, strerror(errno));
        goto err;
    }

    int ret_val = 0;
    while (memset(readline, 0, CPU_LINE_LEN) && fgets(readline, CPU_LINE_LEN, fp_cpu)) {
        pline = readline;
        while (*pline == ' ' || *pline == '\t' || *pline == '\r' || *pline == '\n') {
            pline++;
        }

        if (pline[0] == '\0' || pline[0] == '#') {
            continue;
        }

        pline_end = pline + strlen(pline) - 1;
        while ((*pline_end == '\r' || *pline_end == '\n') && pline_end >= pline) {
            pline_end--;
        }
        *(pline_end + 1) = '\0';

        /***************************************************
         * TODO: use pline here, filtered # and empty lines
         ***************************************************/
        if (strncasecmp(pline, CPU_PREFIX, strlen(CPU_PREFIX)) == 0) {
            pline += strlen(CPU_PREFIX);
            pline = strchr(pline, ':') + 1;
            while (*pline == ' ' || *pline == '\t') {
                pline++;
            }

            ret_val = atoi(pline);
            break;
        }
    }

    fclose(fp_cpu);
    fp_cpu = NULL;
    return ret_val;

err:
    if (fp_cpu) {
        fclose(fp_cpu);
        fp_cpu = NULL;
    }
    return -1;

}

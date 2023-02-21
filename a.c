#include <stdio.h>
#include "cpucycles.h"

#include "console.h"
#include "queue.h"

int main()
{
    simulation = true;
    char *s = "1234567";
    LIST_HEAD(l);
    FILE *fp = fopen("a.txt", "w");
    for (int i = 0; i < 1000000; i++) {
        int64_t t_start = cpucycles();
        q_insert_head(&l, s);
        int64_t t_end = cpucycles();
        fprintf(fp, "%ld\n", t_end - t_start);
    }
    fclose(fp);
    fp = fopen("b.txt", "w");
    char buf[8];
    for (int i = 0; i < 1000000; i++) {
        int16_t t_start = cpucycles();
        element_t *t = q_remove_head(&l, buf, 8);
        int16_t t_end = cpucycles();
        q_release_element(t);
        fprintf(fp, "%d\n", t_end - t_start);
    }
}
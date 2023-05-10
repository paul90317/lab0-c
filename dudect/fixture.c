/** dude, is my code constant time?
 *
 * This file measures the execution time of a given function many times with
 * different inputs and performs a Welch's t-test to determine if the function
 * runs in constant time or not. This is essentially leakage detection, and
 * not a timing attack.
 *
 * Notes:
 *
 *  - the execution time distribution tends to be skewed towards large
 *    timings, leading to a fat right tail. Most executions take little time,
 *    some of them take a lot. We try to speed up the test process by
 *    throwing away those measurements with large cycle count. (For example,
 *    those measurements could correspond to the execution being interrupted
 *    by the OS.) Setting a threshold value for this is not obvious; we just
 *    keep the x% percent fastest timings, and repeat for several values of x.
 *
 *  - the previous observation is highly heuristic. We also keep the uncropped
 *    measurement time and do a t-test on that.
 *
 *  - we also test for unequal variances (second order test), but this is
 *    probably redundant since we're doing as well a t-test on cropped
 *    measurements (non-linear transform)
 *
 *  - as long as any of the different test fails, the code will be deemed
 *    variable time.
 */

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../console.h"
#include "../random.h"

#include "constant.h"
#include "fixture.h"
#include "ttest.h"

#define ENOUGH_MEASURE 10000
#define TEST_TRIES 10
#define N_THRESHOLDS 100

/* N_THRESHOLDS test with thresholds and one without without cropping */
static bool first;
static t_context_t restricted_distributions[N_THRESHOLDS + 1];
static int64_t percentiles[N_THRESHOLDS];

typedef enum {
    LEAKAGE_FOUND = 0,
    NO_LEAKAGE_EVIDENCE_YET,
    NOT_ENOUGH_MEASURE,
    ERROR_IMPLEMENT
} report_state_t;

static int cmp(const int64_t *a, const int64_t *b)
{
    return (int) (*a - *b);
}

static int64_t percentile(int64_t *exec_times, double which)
{
    size_t array_position = (size_t) ((double) N_THRESHOLDS * (double) which);
    assert(array_position >= 0 && array_position < N_THRESHOLDS);
    return exec_times[array_position];
}

static void prepare_percentiles(int64_t *exec_times)
{
    qsort(exec_times, N_MEASURES, sizeof(int64_t),
          (int (*)(const void *, const void *)) cmp);
    for (size_t i = 0; i < N_THRESHOLDS; i++) {
        percentiles[i] = percentile(
            exec_times, 1 - pow(0.5, 10 * (double) (i + 1) / N_THRESHOLDS));
    }
}

/* threshold values for Welch's t-test */
enum {
    t_threshold_bananas = 500, /* Test failed with overwhelming probability */
    t_threshold_moderate = 10, /* Test failed */
};

static void differentiate(int64_t *exec_times,
                          const int64_t *before_ticks,
                          const int64_t *after_ticks)
{
    for (size_t i = 0; i < N_MEASURES; i++)
        exec_times[i] = after_ticks[i] - before_ticks[i];
}

static void update_statistics(const int64_t *exec_times, uint8_t *classes)
{
    for (size_t i = 20; i < N_MEASURES; i++) {
        int64_t difference = exec_times[i];
        /* CPU cycle counter overflowed or dropped measurement */
        if (difference <= 0)
            continue;

        /* do a t-test on the execution time */
        t_push(restricted_distributions + N_THRESHOLDS, difference, classes[i]);

        for (size_t crop_index = 0; crop_index < N_THRESHOLDS; crop_index++) {
            if (difference < percentiles[crop_index]) {
                t_push(restricted_distributions + crop_index, difference,
                       classes[i]);
            }
        }
    }
}

static t_context_t *max_test()
{
    size_t ret = N_THRESHOLDS;
    double max = 0;
    for (size_t i = 0; i <= N_THRESHOLDS - 20; i++) {
        if (restricted_distributions[i].n[0] +
                restricted_distributions[i].n[1] >
            ENOUGH_MEASURE) {
            double x = fabs(t_compute(restricted_distributions + i));
            if (max < x) {
                max = x;
                ret = i;
            }
        }
    }
    return restricted_distributions + ret;
}

static report_state_t report(void)
{
    t_context_t *t = max_test();
    double max_t = fabs(t_compute(t));
    double number_traces_max_t = t->n[0] + t->n[1];
    if (number_traces_max_t < ENOUGH_MEASURE) {
        return NOT_ENOUGH_MEASURE;
    }

    /* Definitely not constant time */
    if (max_t > t_threshold_bananas)
        return LEAKAGE_FOUND;

    /* Probably not constant time. */
    if (max_t > t_threshold_moderate)
        return LEAKAGE_FOUND;

    /* For the moment, maybe constant time. */
    return NO_LEAKAGE_EVIDENCE_YET;
}

int64_t before_ticks[N_MEASURES];
int64_t after_ticks[N_MEASURES];
int64_t exec_times[N_MEASURES];
uint8_t classes[N_MEASURES];
uint8_t input_data[N_MEASURES * CHUNK_SIZE];
FILE *dudect_out;
static report_state_t dudect_main(int mode)
{
    prepare_inputs(input_data, classes);
    if (!measure(before_ticks, after_ticks, input_data, mode))
        return ERROR_IMPLEMENT;
    differentiate(exec_times, before_ticks, after_ticks);
    for (int i = 0; i < N_MEASURES; ++i) {
        fprintf(dudect_out, "%d %ld\n", classes[i], exec_times[i]);
    }
    if (first) {
        prepare_percentiles(exec_times);
        first = false;
        return NOT_ENOUGH_MEASURE;
    }
    update_statistics(exec_times, classes);
    return report();
}

static void dudect_init(void)
{
    first = true;
    dudect_out = fopen("dudect.out", "a");
    for (int i = 0; i <= N_THRESHOLDS; ++i) {
        t_init(restricted_distributions + i);
    }
}

static void dudect_free(void)
{
    fclose(dudect_out);
}

static bool test_const(char *text, int mode)
{
    dudect_out = fopen("dudect.out", "w");
    fclose(dudect_out);
    for (int cnt = 0; cnt < TEST_TRIES; ++cnt) {
        printf("Testing %s...(%d/%d)\n", text, cnt, TEST_TRIES);
        dudect_init();
        report_state_t result = NOT_ENOUGH_MEASURE;
        while (result == NOT_ENOUGH_MEASURE) {
            result = dudect_main(mode);
        }
        dudect_free();
        if (result == NO_LEAKAGE_EVIDENCE_YET)
            return true;
        if (result == ERROR_IMPLEMENT)
            return false;
    }
    return false;
}

#define DUT_FUNC_IMPL(op)                \
    bool is_##op##_const(void)           \
    {                                    \
        return test_const(#op, DUT(op)); \
    }

#define _(x) DUT_FUNC_IMPL(x)
DUT_FUNCS
#undef _

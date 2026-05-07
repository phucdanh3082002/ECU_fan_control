#include "unity.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int g_tests_run;
static int g_tests_failed;
static int g_current_test_failed;

void UnityBegin(const char *name)
{
    g_tests_run = 0;
    g_tests_failed = 0;
    g_current_test_failed = 0;
    printf("Unity test run: %s\n", name);
}

int UnityEnd(void)
{
    printf("\n%d Tests %d Failures\n", g_tests_run, g_tests_failed);
    return g_tests_failed == 0 ? 0 : 1;
}

void UnityDefaultTestRun(void (*test_func)(void), const char *test_name, int line_number)
{
    g_tests_run++;
    g_current_test_failed = 0;

    setUp();
    test_func();
    tearDown();

    if (g_current_test_failed) {
        g_tests_failed++;
        printf("FAIL: %s:%d:%s\n", __FILE__, line_number, test_name);
        return;
    }

    printf("PASS: %s\n", test_name);
}

void UnityAssertTrue(int condition, const char *expression, const char *file, int line)
{
    if (!condition) {
        g_current_test_failed = 1;
        printf("%s:%d: assertion failed: %s\n", file, line, expression);
    }
}

void UnityAssertEqualInt(long expected, long actual, const char *expression, const char *file, int line)
{
    if (expected != actual) {
        g_current_test_failed = 1;
        printf("%s:%d: assertion failed: %s expected %ld got %ld\n", file, line, expression, expected, actual);
    }
}

void UnityAssertEqualString(const char *expected, const char *actual, const char *expression, const char *file, int line)
{
    const int matches = expected != NULL && actual != NULL && strcmp(expected, actual) == 0;
    if (!matches) {
        g_current_test_failed = 1;
        printf("%s:%d: assertion failed: %s expected \"%s\" got \"%s\"\n",
               file,
               line,
               expression,
               expected != NULL ? expected : "(null)",
               actual != NULL ? actual : "(null)");
    }
}

void UnityAssertFloatWithin(float delta, float expected, float actual, const char *expression, const char *file, int line)
{
    if (fabsf(expected - actual) > delta) {
        g_current_test_failed = 1;
        printf("%s:%d: assertion failed: %s expected %.4f got %.4f delta %.4f\n",
               file,
               line,
               expression,
               expected,
               actual,
               delta);
    }
}

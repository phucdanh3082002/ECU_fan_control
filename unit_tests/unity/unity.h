#ifndef UNITY_H
#define UNITY_H

#include <stddef.h>

void UnityBegin(const char *name);
int UnityEnd(void);
void UnityDefaultTestRun(void (*test_func)(void), const char *test_name, int line_number);

void UnityAssertTrue(int condition, const char *expression, const char *file, int line);
void UnityAssertEqualInt(long expected, long actual, const char *expression, const char *file, int line);
void UnityAssertEqualString(const char *expected, const char *actual, const char *expression, const char *file, int line);
void UnityAssertFloatWithin(float delta, float expected, float actual, const char *expression, const char *file, int line);

void setUp(void);
void tearDown(void);

#define UNITY_BEGIN() UnityBegin(__FILE__)
#define UNITY_END() UnityEnd()
#define RUN_TEST(test_func) UnityDefaultTestRun(test_func, #test_func, __LINE__)

#define TEST_ASSERT_TRUE(condition) UnityAssertTrue((condition), #condition, __FILE__, __LINE__)
#define TEST_ASSERT_FALSE(condition) UnityAssertTrue(!(condition), "!(" #condition ")", __FILE__, __LINE__)
#define TEST_ASSERT_NOT_NULL(pointer) UnityAssertTrue((pointer) != NULL, #pointer " != NULL", __FILE__, __LINE__)

#define TEST_ASSERT_EQUAL_INT(expected, actual) UnityAssertEqualInt((expected), (actual), #actual, __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL_UINT32(expected, actual) UnityAssertEqualInt((long)(expected), (long)(actual), #actual, __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL_FLOAT(expected, actual) UnityAssertFloatWithin(0.0001f, (expected), (actual), #actual, __FILE__, __LINE__)
#define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual) UnityAssertFloatWithin((delta), (expected), (actual), #actual, __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL_STRING(expected, actual) UnityAssertEqualString((expected), (actual), #actual, __FILE__, __LINE__)

#endif

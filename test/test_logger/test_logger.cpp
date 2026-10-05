#include <unity.h>
#include "Logger.h"

void setUp(void) {
    logger.begin(DEBUG);
}

void tearDown(void) {
    logger.clearLogs();
}

void test_logger_initialization() {
    TEST_ASSERT_EQUAL(DEBUG, logger.getLogLevel());
}

void test_logger_log_levels() {
    logger.setLogLevel(WARNING);
    logger.debug("Test", "This should not appear");
    logger.info("Test", "This should not appear");
    logger.warning("Test", "This should appear");
    logger.error("Test", "This should appear");

    auto logs = logger.getRecentLogs(10);
    TEST_ASSERT_EQUAL(2, logs.size());
}

void test_logger_circular_buffer() {
    logger.setLogLevel(DEBUG);
    for (int i = 0; i < 150; i++) {
        logger.info("Test", "Message");
    }
    auto logs = logger.getRecentLogs(200);
    TEST_ASSERT_LESS_OR_EQUAL(100, logs.size());
}

void test_logger_formatted_messages() {
    logger.logf(INFO, "Test", "Value: %d, String: %s", 42, "test");
    auto logs = logger.getRecentLogs(1);
    TEST_ASSERT_EQUAL(1, logs.size());
    TEST_ASSERT_TRUE(logs[0].message.indexOf("42") >= 0);
    TEST_ASSERT_TRUE(logs[0].message.indexOf("test") >= 0);
}

void test_logger_clear() {
    logger.info("Test", "Message 1");
    logger.info("Test", "Message 2");
    TEST_ASSERT_EQUAL(2, logger.getRecentLogs(10).size());
    logger.clearLogs();
    TEST_ASSERT_EQUAL(0, logger.getRecentLogs(10).size());
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_logger_initialization);
    RUN_TEST(test_logger_log_levels);
    RUN_TEST(test_logger_circular_buffer);
    RUN_TEST(test_logger_formatted_messages);
    RUN_TEST(test_logger_clear);
    UNITY_END();
}

void loop() {
}

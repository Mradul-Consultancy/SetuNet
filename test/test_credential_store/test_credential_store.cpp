#include <unity.h>
#include "CredentialStore.h"
#include "Logger.h"

CredentialStore credStore;

void setUp(void) {
    logger.begin(INFO);
    TEST_ASSERT_TRUE(credStore.begin());
    credStore.clearCredentials();
}

void tearDown(void) {
    credStore.clearCredentials();
}

static Config validConfig() {
    Config config = credStore.getDefaultConfig();
    config.wifiSSID = "TestSSID";
    config.wifiPassword = "TestPassword";
    config.portalUsername = "testuser";
    config.portalPassword = "testpass";
    config.portalURL = "https://portal.example/login";
    return config;
}

void test_credential_store_initialization() {
    TEST_ASSERT_TRUE(credStore.begin());
}

void test_save_and_load_credentials() {
    Config config = validConfig();
    TEST_ASSERT_TRUE(credStore.saveCredentials(config));
    TEST_ASSERT_TRUE(credStore.hasCredentials());

    Config loadedConfig;
    TEST_ASSERT_TRUE(credStore.loadCredentials(loadedConfig));
    TEST_ASSERT_EQUAL_STRING("TestSSID", loadedConfig.wifiSSID.c_str());
    TEST_ASSERT_EQUAL_STRING("TestPassword", loadedConfig.wifiPassword.c_str());
    TEST_ASSERT_EQUAL_STRING("testuser", loadedConfig.portalUsername.c_str());
    TEST_ASSERT_EQUAL_STRING("testpass", loadedConfig.portalPassword.c_str());
}

void test_validate_ssid() {
    Config config = validConfig();
    config.wifiSSID = "";
    TEST_ASSERT_FALSE(credStore.validateCredentials(config));
    config.wifiSSID = "ValidSSID";
    TEST_ASSERT_TRUE(credStore.validateCredentials(config));
    config.wifiSSID = "ThisSSIDIsWayTooLongForWiFiStandards123456789";
    TEST_ASSERT_FALSE(credStore.validateCredentials(config));
}

void test_validate_url() {
    Config config = validConfig();
    config.portalURL = "invalid-url";
    TEST_ASSERT_FALSE(credStore.validateCredentials(config));
    config.portalURL = "http://valid.com";
    TEST_ASSERT_TRUE(credStore.validateCredentials(config));
    config.portalURL = "https://valid.com";
    TEST_ASSERT_TRUE(credStore.validateCredentials(config));
    config.portalURL = "https://";
    TEST_ASSERT_FALSE(credStore.validateCredentials(config));
}

void test_clear_credentials() {
    TEST_ASSERT_TRUE(credStore.saveCredentials(validConfig()));
    TEST_ASSERT_TRUE(credStore.hasCredentials());
    TEST_ASSERT_TRUE(credStore.clearCredentials());
    TEST_ASSERT_FALSE(credStore.hasCredentials());
}

void test_default_config_does_not_embed_credentials() {
    Config config = credStore.getDefaultConfig();
    TEST_ASSERT_EQUAL_STRING("", config.wifiSSID.c_str());
    TEST_ASSERT_EQUAL_STRING("", config.wifiPassword.c_str());
    TEST_ASSERT_EQUAL_STRING("", config.portalUsername.c_str());
    TEST_ASSERT_EQUAL_STRING("", config.portalPassword.c_str());
    TEST_ASSERT_EQUAL(60000, config.checkIntervalMs);
    TEST_ASSERT_EQUAL(15000, config.httpTimeoutMs);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_credential_store_initialization);
    RUN_TEST(test_save_and_load_credentials);
    RUN_TEST(test_validate_ssid);
    RUN_TEST(test_validate_url);
    RUN_TEST(test_clear_credentials);
    RUN_TEST(test_default_config_does_not_embed_credentials);
    UNITY_END();
}

void loop() {
}

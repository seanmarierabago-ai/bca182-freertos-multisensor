#include <unity.h>

#include "alarm_logic.h"
#include "display_navigation.h"
#include "system_state_logic.h"

void setUp(void) {}
void tearDown(void) {}

void test_temperature_below_lower_threshold(void)
{
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AlarmState::LOW_TEMPERATURE),
                          static_cast<int>(evaluateTemperature(17.9f)));
}

void test_temperature_at_lower_threshold_is_normal(void)
{
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AlarmState::NORMAL),
                          static_cast<int>(evaluateTemperature(18.0f)));
}

void test_temperature_in_normal_range(void)
{
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AlarmState::NORMAL),
                          static_cast<int>(evaluateTemperature(25.0f)));
}

void test_temperature_at_upper_threshold_is_normal(void)
{
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AlarmState::NORMAL),
                          static_cast<int>(evaluateTemperature(30.0f)));
}

void test_temperature_above_upper_threshold(void)
{
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AlarmState::HIGH_TEMPERATURE),
                          static_cast<int>(evaluateTemperature(30.1f)));
}

void test_next_display_mode_moves_forward(void)
{
    TEST_ASSERT_EQUAL_INT(static_cast<int>(DisplayMode::HUMIDITY),
                          static_cast<int>(nextDisplayMode(DisplayMode::TEMPERATURE)));
}

void test_previous_display_mode_moves_backward(void)
{
    TEST_ASSERT_EQUAL_INT(static_cast<int>(DisplayMode::TEMPERATURE),
                          static_cast<int>(previousDisplayMode(DisplayMode::HUMIDITY)));
}

void test_next_display_mode_wraps_to_temperature(void)
{
    TEST_ASSERT_EQUAL_INT(static_cast<int>(DisplayMode::TEMPERATURE),
                          static_cast<int>(nextDisplayMode(DisplayMode::MOTION)));
}

void test_previous_display_mode_wraps_to_motion(void)
{
    TEST_ASSERT_EQUAL_INT(static_cast<int>(DisplayMode::MOTION),
                          static_cast<int>(previousDisplayMode(DisplayMode::TEMPERATURE)));
}

void test_active_without_timeout_stays_active(void)
{
    TEST_ASSERT_EQUAL_INT(static_cast<int>(SystemState::ACTIVE),
                          static_cast<int>(evaluateSystemState(SystemState::ACTIVE, false, false)));
}

void test_active_timeout_becomes_inactive(void)
{
    TEST_ASSERT_EQUAL_INT(static_cast<int>(SystemState::INACTIVE),
                          static_cast<int>(evaluateSystemState(SystemState::ACTIVE, false, true)));
}

void test_inactive_without_motion_stays_inactive(void)
{
    TEST_ASSERT_EQUAL_INT(static_cast<int>(SystemState::INACTIVE),
                          static_cast<int>(evaluateSystemState(SystemState::INACTIVE, false, false)));
}

void test_motion_restores_active_state(void)
{
    TEST_ASSERT_EQUAL_INT(static_cast<int>(SystemState::ACTIVE),
                          static_cast<int>(evaluateSystemState(SystemState::INACTIVE, true, false)));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_temperature_below_lower_threshold);
    RUN_TEST(test_temperature_at_lower_threshold_is_normal);
    RUN_TEST(test_temperature_in_normal_range);
    RUN_TEST(test_temperature_at_upper_threshold_is_normal);
    RUN_TEST(test_temperature_above_upper_threshold);
    RUN_TEST(test_next_display_mode_moves_forward);
    RUN_TEST(test_previous_display_mode_moves_backward);
    RUN_TEST(test_next_display_mode_wraps_to_temperature);
    RUN_TEST(test_previous_display_mode_wraps_to_motion);
    RUN_TEST(test_active_without_timeout_stays_active);
    RUN_TEST(test_active_timeout_becomes_inactive);
    RUN_TEST(test_inactive_without_motion_stays_inactive);
    RUN_TEST(test_motion_restores_active_state);
    return UNITY_END();
}
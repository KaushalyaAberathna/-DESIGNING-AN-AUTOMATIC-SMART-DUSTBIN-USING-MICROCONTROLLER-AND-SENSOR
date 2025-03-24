#include <stdio.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "esp_timer.h"

#define TRIG_PIN 32
#define ECHO_PIN 33
#define IR_SENSOR_PIN 34
#define SERVO_PIN 18
#define MAX_HEIGHT 15

void delay_ms(uint32_t ms) {
    uint64_t start_time = esp_timer_get_time();
    while ((esp_timer_get_time() - start_time) < (ms * 1000));
}

void init_servo() {
    rmt_config_t rmt_cfg = {
        .rmt_mode = RMT_MODE_TX,
        .channel = RMT_CHANNEL_0,
        .clk_div = 80,  // 1µs per tick
        .gpio_num = SERVO_PIN,
        .mem_block_num = 1,
        .tx_config = {
            .carrier_en = false,
            .loop_en = false,
            .idle_level = RMT_IDLE_LEVEL_LOW,
            .idle_output_en = true
        }
    };
    rmt_config(&rmt_cfg);
    rmt_driver_install(RMT_CHANNEL_0, 0, 0);
}

void set_servo_angle(int angle) {
    int pulse_width = (angle * (2000 - 500) / 180) + 500; // Convert angle to microseconds (500us - 2500us)
    rmt_item32_t item = {
        .duration0 = pulse_width,
        .level0 = 1,
        .duration1 = 20000 - pulse_width,
        .level1 = 0
    };
    rmt_write_items(RMT_CHANNEL_0, &item, 1, true);
}

void init_ultrasonic() {
    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
}

int measure_distance() {
    gpio_set_level(TRIG_PIN, 0);
    delay_ms(2);
    gpio_set_level(TRIG_PIN, 1);
    delay_ms(10);
    gpio_set_level(TRIG_PIN, 0);

    uint64_t start = esp_timer_get_time();
    while (gpio_get_level(ECHO_PIN) == 0) {
        if (esp_timer_get_time() - start > 50000) return -1;
    }
    uint64_t pulse_start = esp_timer_get_time();

    while (gpio_get_level(ECHO_PIN) == 1) {
        if (esp_timer_get_time() - pulse_start > 50000) return -1;
    }
    uint64_t pulse_end = esp_timer_get_time();

    return (pulse_end - pulse_start) / 58; // Convert time to cm
}

void app_main() {
    init_servo();
    init_ultrasonic();
    gpio_set_direction(IR_SENSOR_PIN, GPIO_MODE_INPUT);

    while (1) {
        int distance = measure_distance();
        int fillingPercentage = (distance >= 0 && distance <= MAX_HEIGHT) ? 100 - ((distance * 100) / MAX_HEIGHT) : 0;

        printf("Distance: %d cm, Filling Percentage: %d%%\n", distance, fillingPercentage);

        if (gpio_get_level(IR_SENSOR_PIN) == 1) {
            set_servo_angle(90);
            printf("Object detected, opening lid...\n");
        } else {
            set_servo_angle(0);
            printf("Lid closed.\n");
        }

        delay_ms(500);
    }
}

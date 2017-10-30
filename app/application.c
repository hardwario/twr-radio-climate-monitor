#include <application.h>

#define FIRMWARE "bcf-kit-climate-monitor:" TAG

#define BATTERY_UPDATE_INTERVAL (60 * 60 * 1000)

#define UPDATE_INTERVAL (1 * 1000)

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL (5 * 60 * 1000)
#define TEMPERATURE_TAG_PUB_VALUE_CHANGE 0.2f

#define HUMIDITY_TAG_PUB_NO_CHANGE_INTEVAL (5 * 60 * 1000)
#define HUMIDITY_TAG_PUB_VALUE_CHANGE 5.0f

#define LUX_METER_TAG_PUB_NO_CHANGE_INTEVAL (5 * 60 * 1000)
#define LUX_METER_TAG_PUB_VALUE_CHANGE 100.0f

#define BAROMETER_TAG_PUB_NO_CHANGE_INTEVAL (5 * 60 * 1000)
#define BAROMETER_TAG_PUB_VALUE_CHANGE 20.0f

struct {
    event_param_t temperature;
    event_param_t humidity;
    event_param_t illuminance;
    event_param_t pressure;

} params;

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;
uint16_t button_event_count = 0;

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    uint16_t *event_count = (uint16_t *) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&led, 100);

        (*event_count)++;

        bc_radio_pub_push_button(event_count);
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_radio_enroll_to_gateway();

        bc_led_set_mode(&led, BC_LED_MODE_OFF);

        bc_led_pulse(&led, 1000);

        bc_radio_pub_info(FIRMWARE);
    }
}

void battery_event_handler(bc_module_battery_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    float voltage;

    if (bc_module_battery_get_voltage(&voltage))
    {
        bc_radio_pub_battery(BC_MODULE_BATTERY_FORMAT_MINI, &voltage);
    }
}


void climate_module_event_handler(bc_module_climate_event_t event, void *event_param)
{
    (void) event_param;

    float value;

    if (event == BC_MODULE_CLIMATE_EVENT_UPDATE_THERMOMETER)
    {
        if (bc_module_climate_get_temperature_celsius(&value))
        {
            if ((fabs(value - params.temperature.value) >= TEMPERATURE_TAG_PUB_VALUE_CHANGE) || (params.temperature.next_pub < bc_scheduler_get_spin_tick()))
            {
                bc_radio_pub_thermometer(BC_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT, &value);
                params.temperature.value = value;
                params.temperature.next_pub = bc_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL;
            }
        }
    }
    else if (event == BC_MODULE_CLIMATE_EVENT_UPDATE_HYGROMETER)
    {
        if (bc_module_climate_get_humidity_percentage(&value))
        {
            if ((fabs(value - params.humidity.value) >= HUMIDITY_TAG_PUB_VALUE_CHANGE) || (params.humidity.next_pub < bc_scheduler_get_spin_tick()))
            {
                bc_radio_pub_humidity((0x40 | 0x0f), &value);
                params.humidity.value = value;
                params.humidity.next_pub = bc_scheduler_get_spin_tick() + HUMIDITY_TAG_PUB_NO_CHANGE_INTEVAL;
            }
        }
    }
    else if (event == BC_MODULE_CLIMATE_EVENT_UPDATE_LUX_METER)
    {
        if (bc_module_climate_get_illuminance_lux(&value))
        {
            if ((fabs(value - params.illuminance.value) >= LUX_METER_TAG_PUB_VALUE_CHANGE) || (params.illuminance.next_pub < bc_scheduler_get_spin_tick()))
            {
                bc_radio_pub_luminosity(BC_TAG_LUX_METER_I2C_ADDRESS_DEFAULT, &value);
                params.illuminance.value = value;
                params.illuminance.next_pub = bc_scheduler_get_spin_tick() + LUX_METER_TAG_PUB_NO_CHANGE_INTEVAL;
            }
        }
    }
    else if (event == BC_MODULE_CLIMATE_EVENT_UPDATE_BAROMETER)
    {
        if (bc_module_climate_get_pressure_pascal(&value))
        {
            if ((fabs(value - params.pressure.value) >= BAROMETER_TAG_PUB_VALUE_CHANGE) || (params.pressure.next_pub < bc_scheduler_get_spin_tick()))
            {
                float meter;

                if (!bc_module_climate_get_altitude_meter(&meter))
                {
                    return;
                }

                bc_radio_pub_barometer(0x60, &value, &meter);
                params.pressure.value = value;
                params.pressure.next_pub = bc_scheduler_get_spin_tick() + BAROMETER_TAG_PUB_NO_CHANGE_INTEVAL;
            }
        }
    }
}

void application_init(void)
{
    // Initialize LED
   bc_led_init(&led, BC_GPIO_LED, false, false);
   bc_led_set_mode(&led, BC_LED_MODE_ON);

   bc_radio_init();

   // Initialize button
   bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
   bc_button_set_event_handler(&button, button_event_handler, &button_event_count);

   // Initialize battery
   bc_module_battery_init(BC_MODULE_BATTERY_FORMAT_MINI);
   bc_module_battery_set_event_handler(battery_event_handler, NULL);
   bc_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);

   // Initialize climate module
   bc_module_climate_init();
   bc_module_climate_set_update_interval_all_sensors(UPDATE_INTERVAL);
   bc_module_climate_set_event_handler(climate_module_event_handler, NULL);
   bc_module_climate_measure_all_sensors();

   bc_radio_enroll_to_gateway();
   bc_radio_pub_info(FIRMWARE);

   bc_led_set_mode(&led, BC_LED_MODE_OFF);
}

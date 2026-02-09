#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C Settings
#define MS5611 i2c0
#define MS5611_SDA 16
#define MS5611_SCL 17

const uint8_t addr = 0x77;

//Variables and Definitions
#define P0 1013.25

int32_t pressure, temperature, t2;
uint16_t calib_datas[8];
uint32_t raw_pres, raw_temp, dT;
int32_t off, off2, sens, sens2;

// ----- MS5611 Functions ----

// starts ms5611
void ms5611_reset() {
    uint8_t val = 0x1E;
    i2c_write_blocking(MS5611, addr, &val, 1, false);
}

// reads calibration datas from ms5611
void ms5611_read_calib_data() {
    uint8_t buf[2];
    for(uint8_t val = 0xA0, i = 0; val < 0xAF; val += 2, i++) {
        i2c_write_blocking(MS5611, addr, &val, 1, false);
        i2c_read_blocking(MS5611, addr, buf, 2, false);

        calib_datas[i] = (buf[0] << 8) | buf[1];
        printf("%d\n", calib_datas[i]);
    }
}

// reads adc data from ms5611
void ms5611_read_raw() {
    uint8_t buf[3] = {0};
    uint8_t adc = 0x00;

    uint8_t val = 0x48;
    i2c_write_blocking(MS5611, addr, &val, 1, true);
    sleep_us(9100);
    i2c_write_blocking(MS5611, addr, &adc, 1, true);
    i2c_read_blocking(MS5611, addr, buf, 3, false);
    raw_pres = (uint32_t)((buf[0] << 16) | (buf[1] << 8) | buf[2]);

    for(int i = 0; i < 3; i++) buf[i] = 0;
    val = 0x58;
    i2c_write_blocking(MS5611, addr, &val, 1, true);
    sleep_us(9100);
    i2c_write_blocking(MS5611, addr, &adc, 1, true);
    i2c_read_blocking(MS5611, addr, buf, 3, false);
    raw_temp = (uint32_t)((buf[0] << 16) | (buf[1] << 8) | buf[2]);
}

// converts raw datas to human readable datas
void ms5611_convert_datas() {
    dT = raw_temp - calib_datas[5] * pow(2, 8);
    temperature = 2000 + dT * calib_datas[6] / pow(2,23);

    off = calib_datas[2] * pow(2, 16) + (calib_datas[4] * dT ) / pow(2, 7);
    sens = calib_datas[1] * pow(2, 15) + (calib_datas[3] * dT) / pow(2, 8);

    if (temperature < 2000) {   
        t2 = pow(dT, 2) / pow(2, 31);
        off2 = 5 * pow((temperature-2000), 2) / 2;
        sens2 = 5 * pow((temperature-2000), 2) / 4;

        if(temperature < -1500) {
            off2 += 7 * pow((temperature + 1500), 2);
            sens2 += 11 * pow((temperature + 1500), 2) / 2;
        }
    } else {
        t2 = 0;
        off2 = 0;
        sens2 = 0;
    }
    temperature -= t2;
    off -= off2;
    sens -= sens2;

    pressure = (raw_pres * sens / pow(2, 21) - off) / pow(2, 15);
}

// calculates altitude by using defined sea level pressure
double calc_altitude() {
    return (((temperature/100.f)+273.15)/0.0065)*(1-pow((pressure/100.f)/P0, 0.190263));
}

int main() {
    // initializes usb serial port
    stdio_init_all();
    printf("Wait a second please!\n");

    // initializes and setting i2c for ms5611
    i2c_init(MS5611, 400000);
    gpio_set_function(MS5611_SDA, GPIO_FUNC_I2C);
    gpio_set_function(MS5611_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(MS5611_SDA);
    gpio_pull_up(MS5611_SCL);

    // ms5611 starts working
    ms5611_reset();
    sleep_ms(500);

    // reading calibiration datas
    printf("Wait a second please!\n");
    ms5611_read_calib_data();

    // reading and converting pressure and temperature datas
    ms5611_read_raw();
    ms5611_convert_datas();

    sleep_ms(500);
    
    // defining variables
    double pres[100] = {0}, p;
    uint8_t i = 0;

    // main loop
    while(1) {
        // reads and converts datas again and again
        ms5611_read_raw();
        ms5611_convert_datas();
        
        // filters pressure data
        i %= 100;
        pres[i] = pressure;
        for(int a = 0, p = 0; a < 100; a++) {
            p += pres[a];
        }
        pressure = p/100.f;

        // prints the debug outputs
        printf("Raw datas; T: %lu, P: %lu\n", raw_temp, raw_pres);
        printf("Temperature: %.2f°C\n", ((float)temperature / 100.f));
        printf("Pressure: %.2f mb\n", ((float)pressure / 100.f));
        printf("Altitude: %.2fm\n", calc_altitude());

        i++;
        sleep_ms(25); // waits for 25ms meanwhile does nothing
    }

    // the function for standby mode
    // during this function process nothing actually happens
    tight_loop_contents();

    return 0;
}
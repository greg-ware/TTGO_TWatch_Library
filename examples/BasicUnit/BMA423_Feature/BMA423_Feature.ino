#include "config.h"

TTGOClass *watch;
TFT_eSPI *tft;
BMA *sensor;
bool irq = false;
uint32_t tiltCount = 0;
uint32_t clickCount = 0;
uint32_t stepCount = 0;
int16_t xoffset = 30;


void setup()
{
    Serial.begin(115200);

    // Get TTGOClass instance
    watch = TTGOClass::getWatch();

    // Initialize the hardware, the BMA423 sensor has been initialized internally
    watch->begin();

    // Turn on the backlight
    watch->openBL();

    //Receive objects for easy writing
    tft = watch->tft;
    sensor = watch->bma;

    // Accel parameter structure
    Acfg cfg;
    /*!
        Output data rate in Hz, Optional parameters:
            - BMA4_OUTPUT_DATA_RATE_0_78HZ
            - BMA4_OUTPUT_DATA_RATE_1_56HZ
            - BMA4_OUTPUT_DATA_RATE_3_12HZ
            - BMA4_OUTPUT_DATA_RATE_6_25HZ
            - BMA4_OUTPUT_DATA_RATE_12_5HZ
            - BMA4_OUTPUT_DATA_RATE_25HZ
            - BMA4_OUTPUT_DATA_RATE_50HZ
            - BMA4_OUTPUT_DATA_RATE_100HZ
            - BMA4_OUTPUT_DATA_RATE_200HZ
            - BMA4_OUTPUT_DATA_RATE_400HZ
            - BMA4_OUTPUT_DATA_RATE_800HZ
            - BMA4_OUTPUT_DATA_RATE_1600HZ
    */
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    /*!
        G-range, Optional parameters:
            - BMA4_ACCEL_RANGE_2G
            - BMA4_ACCEL_RANGE_4G
            - BMA4_ACCEL_RANGE_8G
            - BMA4_ACCEL_RANGE_16G
    */
    cfg.range = BMA4_ACCEL_RANGE_2G;
    /*!
        Bandwidth parameter, determines filter configuration, Optional parameters:
            - BMA4_ACCEL_OSR4_AVG1
            - BMA4_ACCEL_OSR2_AVG2
            - BMA4_ACCEL_NORMAL_AVG4
            - BMA4_ACCEL_CIC_AVG8
            - BMA4_ACCEL_RES_AVG16
            - BMA4_ACCEL_RES_AVG32
            - BMA4_ACCEL_RES_AVG64
            - BMA4_ACCEL_RES_AVG128
    */
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;

    /*! Filter performance mode , Optional parameters:
        - BMA4_CIC_AVG_MODE
        - BMA4_CONTINUOUS_MODE
    */
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;

    // Configure the BMA423 accelerometer
    sensor->accelConfig(cfg);

    // Enable BMA423 accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    sensor->enableAccel();

    pinMode(BMA423_INT1, INPUT);
    attachInterrupt(BMA423_INT1, [] {
        // Set interrupt to set irq value to 1
        irq = 1;
    }, RISING); //It must be a rising edge

    // Enable BMA423 isStepCounter feature
    sensor->enableFeature(BMA423_STEP_CNTR, true);
    // Enable BMA423 isTilt feature
    sensor->enableFeature(BMA423_TILT, true);
    // Enable BMA423 isDoubleClick feature
    sensor->enableFeature(BMA423_WAKEUP, true);

    // Reset steps
    sensor->resetStepCounter();

    // Turn on feature interrupt
    sensor->enableStepCountInterrupt();
    sensor->enableTiltInterrupt();
    // It corresponds to isDoubleClick interrupt
    sensor->enableWakeupInterrupt();

    // Some display settings
    tft->setTextColor(random(0xFFFF));
    tft->drawString("BMA423 StepCount", 3, 50, 4);
    tft->setTextFont(4);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);

    tft->setTextColor(random(0xFFFF), TFT_BLACK);
    tft->setCursor(xoffset, 118);
    tft->print("StepCount:");
    tft->print(stepCount);
    tft->setTextColor(random(0xFFFF), TFT_BLACK);
    tft->setCursor(xoffset, 160);
    tft->print("isTilt:");
    tft->print(tiltCount);
    tft->setTextColor(random(0xFFFF), TFT_BLACK);
    tft->setCursor(xoffset, 202);
    tft->print("isDoubleClick:");
    tft->print(clickCount);
}

void loop()
{
    if (irq) {
        irq = 0;
        bool  rlst;
        do {
            // Read the BMA423 interrupt status,
            // need to wait for it to return to true before continuing
            rlst =  sensor->readInterrupt();
        } while (!rlst);

        // Check if it is a step interrupt
        if (sensor->isStepCounter()) {
            // Get step data from register
            stepCount = sensor->getCounter();
            tft->setTextColor(random(0xFFFF), TFT_BLACK);
            tft->setCursor(xoffset, 118);
            tft->print("StepCount:");
            tft->print(stepCount);
        }
        // The wrist must be worn correctly, otherwise the data will not come out
        if (sensor->isTilt()) {
            Serial.println("isTilt");
            tft->setTextColor(random(0xFFFF), TFT_BLACK);
            tft->setCursor(xoffset, 160);
            tft->print("isTilt:");
            tft->print(++tiltCount);
        }
        // Double-click interrupt
        if (sensor->isDoubleClick()) {
            Serial.println("isDoubleClick");
            tft->setTextColor(random(0xFFFF), TFT_BLACK);
            tft->setCursor(xoffset, 202);
            tft->print("isDoubleClick:");
            tft->print(++clickCount);
        }
    }
    delay(20);
}

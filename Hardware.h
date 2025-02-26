#define NOTE_SF 274.4 // 1V/Octave
#define VEL_SF 256.0

//#define LED_PIN 23    // Define the pin for the built-in RGB LED YD-RP2040
#define LED_PIN 16    // Define the pin for the built-in RGB LED ZERO

#define NUM_PIXELS 1  // Number of WS2812 LEDs

#define FORMAT_LITTLEFS_IF_FAILED true

// Voices available
#define NO_OF_VOICES 1
#define trigTimeout 30

#define FM_INPUT 26  // ADC0

//Note DACS
#define DAC_NOTE1 5

//Switch LFO Wave
#define LFO_ALT 7

//Switch Up
#define SWITCH_UP 8

//Switch down
#define SWITCH_DOWN 9

//Switch select
#define SWITCH_SELECT 10

#define GATE_LED 11
#define TRIG_LED 28

//Gate outputs
#define GATE_NOTE1 14

//Trig output
#define TRIG_NOTE1 15

void setupHardware() {

  analogReadResolution(12);

  pinMode(SWITCH_UP, INPUT_PULLUP);
  pinMode(SWITCH_DOWN, INPUT_PULLUP);
  pinMode(SWITCH_SELECT, INPUT_PULLUP);

  pinMode(GATE_NOTE1, OUTPUT);
  digitalWrite(GATE_NOTE1, LOW);

  pinMode(GATE_LED, OUTPUT);
  digitalWrite(GATE_LED, HIGH);
  pinMode(TRIG_LED, OUTPUT);
  digitalWrite(TRIG_LED, HIGH);

  pinMode(TRIG_NOTE1, OUTPUT);
  digitalWrite(TRIG_NOTE1, LOW);

  pinMode(DAC_NOTE1, OUTPUT);
  digitalWrite(DAC_NOTE1, HIGH);

  pinMode(LFO_ALT, OUTPUT);
  digitalWrite(LFO_ALT, HIGH);

  SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE1));
  digitalWrite(DAC_NOTE1, LOW);
  SPI.transfer16(int_ref_on_flexible_mode >> 16);
  SPI.transfer16(int_ref_on_flexible_mode);
  delayMicroseconds(1);
  digitalWrite(DAC_NOTE1, HIGH);
  SPI.endTransaction();

}
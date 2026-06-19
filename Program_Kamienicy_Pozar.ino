#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 0

#include <FastLED.h>
#include <Servo.h>

// ================== PRZYCISK ==================
#define BTN_PIN 2  // D2 do GND (INPUT_PULLUP)

// ================== LED (WS2812B) - pasek 1 ==================
#define LED_PIN      12
#define NUM_LEDS     28
#define LED_TYPE     WS2812B
#define COLOR_ORDER  GRB
#define BRIGHTNESS   120

// ================== LED (WS2812B) - pasek 2 ==================
#define LED2_PIN     10
#define NUM_LEDS2    6

// ================== SILNIK PWM ==================
#define MOTOR_PWM_PIN 6

// ================== WYJŚCIA (tranzystory) ==================
#define OUT_5V   8      // 5V_OUT
#define OUT12_1  5      // 12V_OUT1
#define OUT12_2  7      // 12V_OUT2
#define OUT12_3  14     // 12V_OUT3 = A0
#define OUT12_4  15     // 12V_OUT4 = A1

// 0 = HIGH włącza, 1 = LOW włącza
#define ACTIVE_LOW 0

// ================== KOGUTY ==================
#define KOGUT_PIN   11

const uint16_t PULSE_LOW   = 1000;
const uint16_t PULSE_HIGH  = 2200;
const uint16_t PERIOD_MS   = 100;

const uint8_t FIRST_MODE   = 1;
const uint8_t LAST_MODE    = 7;

const uint8_t FIRE_MODE    = 1;
const uint8_t OFF_MODE     = 7;

Servo sig;
uint8_t currentMode = LAST_MODE;

// Jeden "klik": krótko 2200us, potem baza 1000us
void clickOnce() {
  sig.writeMicroseconds(PULSE_HIGH);
  delay(PERIOD_MS * 2);

  sig.writeMicroseconds(PULSE_LOW);
  delay(PERIOD_MS * 2);

  if (currentMode < LAST_MODE) {
    currentMode++;
  } else {
    currentMode = FIRST_MODE;
  }
}

uint8_t clicksForward(uint8_t from, uint8_t to) {
  if (from == to) return 0;

  uint8_t c = 0;
  uint8_t m = from;

  do {
    if (m < LAST_MODE) {
      m++;
    } else {
      m = FIRST_MODE;
    }

    c++;
  } while (m != to);

  return c;
}

void goToMode(uint8_t target) {
  if (target < FIRST_MODE || target > LAST_MODE) return;
  if (target == currentMode) return;

  uint8_t clicks = clicksForward(currentMode, target);

  for (uint8_t i = 0; i < clicks; i++) {
    clickOnce();
  }
}

// ================== EFEKT LED ==================
#define FPS          60
#define FIXED_BPM    24

const CRGB COLOR_RED    = CRGB(255, 0, 0);
const CRGB COLOR_ORANGE = CRGB(255, 80, 0);

CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS2];

// ================== GRUPY ==================
const uint8_t GROUPS[][7] = {
  {  0,  1,  2,  3, 27, 26, 25 },
  {  4,  5,  6,  7, 24, 23, 22 },
  {  8,  9, 10, 11, 21, 20, 19 },
  { 12, 13, 14, 15, 18, 17, 16 }
};

const uint8_t GROUP_COUNT = sizeof(GROUPS) / sizeof(GROUPS[0]);
const uint8_t GROUP_SIZE  = sizeof(GROUPS[0]) / sizeof(GROUPS[0][0]);

static inline void renderLeds(uint8_t bpm) {
  uint8_t t = beatsin8(bpm, 0, 255);

  CRGB cNorm = blend(COLOR_RED, COLOR_ORANGE, t);
  CRGB cInv  = blend(COLOR_RED, COLOR_ORANGE, 255 - t);

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  for (uint8_t g = 0; g < GROUP_COUNT; g++) {
    CRGB col = (g % 2 == 1) ? cInv : cNorm;

    for (uint8_t i = 0; i < GROUP_SIZE; i++) {
      leds[GROUPS[g][i]] = col;
    }
  }

  fill_solid(leds2, NUM_LEDS2, cNorm);

  FastLED.show();
}

// ================== TRYB ==================
// 0 = POŻAR
// 1 = OFF MODE
byte mode = 0;
byte prevMode = 0;

// ================== DEBOUNCE ==================
unsigned long lastBtnChangeMs = 0;
byte lastBtnReading = HIGH;
byte stableBtn = HIGH;

#define DEBOUNCE_MS 30

void setup() {
  delay(300);

  pinMode(BTN_PIN, INPUT_PULLUP);

  pinMode(OUT_5V, OUTPUT);
  pinMode(OUT12_1, OUTPUT);
  pinMode(OUT12_2, OUTPUT);
  pinMode(OUT12_3, OUTPUT);
  pinMode(OUT12_4, OUTPUT);

  pinMode(MOTOR_PWM_PIN, OUTPUT);

  // LED init
  FastLED.addLeds<LED_TYPE, LED_PIN,  COLOR_ORDER>(leds,  NUM_LEDS);
  FastLED.addLeds<LED_TYPE, LED2_PIN, COLOR_ORDER>(leds2, NUM_LEDS2);
  FastLED.setBrightness(BRIGHTNESS);

  // Koguty init
  sig.attach(KOGUT_PIN);
  sig.writeMicroseconds(PULSE_LOW);
  delay(PERIOD_MS * 3);

  // Zakładamy, że po starcie kogut jest w OFF
  currentMode = LAST_MODE;

  // Odczyt początkowego stanu przycisku
  stableBtn = digitalRead(BTN_PIN);
  lastBtnReading = stableBtn;

  // HIGH = POŻAR, LOW = OFF
  if (stableBtn == HIGH) {
    mode = 0;
    goToMode(FIRE_MODE);
  } else {
    mode = 1;
    goToMode(OFF_MODE);
  }

  prevMode = mode;
}

void loop() {
  // ================== ODCZYT PRZYCISKU Z DEBOUNCE ==================
  byte reading = digitalRead(BTN_PIN);

  if (reading != lastBtnReading) {
    lastBtnChangeMs = millis();
    lastBtnReading = reading;
  }

  if (millis() - lastBtnChangeMs > DEBOUNCE_MS) {
    if (reading != stableBtn) {
      stableBtn = reading;

      // HIGH = POŻAR
      // LOW  = OFF MODE
      if (stableBtn == HIGH) {
        mode = 0;
      } else {
        mode = 1;
      }
    }
  }

  // ================== ZMIANA TRYBU KOGUTÓW ==================
  if (mode != prevMode) {
    if (mode == 0) {
      goToMode(FIRE_MODE);
    } else {
      goToMode(OFF_MODE);
    }

    prevMode = mode;
  }

  // ================== TRYB 0: POŻAR ==================
  if (mode == 0) {
#if ACTIVE_LOW
    digitalWrite(OUT_5V, LOW);
    digitalWrite(OUT12_1, LOW);
    digitalWrite(OUT12_2, HIGH);
    digitalWrite(OUT12_3, HIGH);
    digitalWrite(OUT12_4, HIGH);
#else
    digitalWrite(OUT_5V, HIGH);
    digitalWrite(OUT12_1, HIGH);
    digitalWrite(OUT12_2, LOW);
    digitalWrite(OUT12_3, LOW);
    digitalWrite(OUT12_4, LOW);
#endif

    analogWrite(MOTOR_PWM_PIN, 255);

    renderLeds(FIXED_BPM);
    FastLED.delay(1000 / FPS);
  }

  // ================== TRYB 1: OFF MODE ==================
  else {
#if ACTIVE_LOW
    digitalWrite(OUT_5V, HIGH);
    digitalWrite(OUT12_1, HIGH);
    digitalWrite(OUT12_2, LOW);
    digitalWrite(OUT12_3, LOW);
    digitalWrite(OUT12_4, LOW);
#else
    digitalWrite(OUT_5V, LOW);
    digitalWrite(OUT12_1, LOW);
    digitalWrite(OUT12_2, HIGH);
    digitalWrite(OUT12_3, HIGH);
    digitalWrite(OUT12_4, HIGH);
#endif

    analogWrite(MOTOR_PWM_PIN, 0);

    FastLED.clear(true);
    delay(30);
  }
}
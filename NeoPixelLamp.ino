#include <Adafruit_NeoPixel.h>

// The inspiration for this code came from the
// Adafruit buttoncycler example - https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/buttoncycler/buttoncycler.ino
// I wanted something that would cycle through
// each mode without the mode needing to finish so
// I started researching how to do that using the 
// standard Arduino loop flow state and found the "millis()"
// function.  It's common to use a timer to update things
// instead of using the "delay()" function, which is blocking

// Once I got the basic functions working, I used AI to generate other "tick"
// functions similar to the candleTick I wrote and then edited them to work 
// the way I wanted them to

#define LED_PIN     0
#define NUMPIXELS   7

#define BUTTON_PIN  7    // button to GND, uses INPUT_PULLUP

// See the above linked buttoncycler code for a bit more
// explanation on setup for your NeoPixel.  I had an old
// one laying around and had to adjust it to work with that one
// verses just using these settings for my pixel ring
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

enum Mode : uint8_t { MODE_CANDLE, MODE_SOLID, MODE_RAINBOW, MODE_LIGHTENING, MODE_RED, MODE_GREEN, MODE_BLUE, MODE_PURPLE, MODE_WHITE, MODE_PARTY, MODE_OFF };
Mode mode = MODE_CANDLE;

// Candle parameters 
static const uint8_t baseRed   = 255;
static const uint8_t baseGreen = 140;
static const uint8_t baseBlue  = 20;

// Call all the "tick" functions in the main loop
// these functions don't block and use the millis function
// to determine when to update, which is pretty common
// in arduino sketches

void candleTick() 
{
  static uint32_t nextUpdateMs = 0;

  uint32_t now = millis();
  if (now < nextUpdateMs) return;

  // schedule next update
  nextUpdateMs = now + (uint32_t)random(30, 80);

  // brightness flicker range (tune this)
  int flicker = random(120, 255);

  // slight color variation with flicker
  uint8_t r = map(flicker, 0, 255, baseRed - 20, baseRed);
  uint8_t g = map(flicker, 0, 255, baseGreen - 30, baseGreen);
  uint8_t b = map(flicker, 0, 255, baseBlue - 10, baseBlue);

  for (int i = 0; i < NUMPIXELS; i++) 
  {
    pixels.setPixelColor(i, r, g, b);
  }
  pixels.show();
}

void rainbowFadeTick() 
{
  static uint16_t hue = 0;
  static uint32_t lastMs = 0;

  uint32_t now = millis();
  if (lastMs == 0) lastMs = now;
  uint32_t dt = now - lastMs;
  lastMs = now;

  hue += (uint16_t)(dt * 4);  // fade speed

  uint32_t color = pixels.ColorHSV(hue, 255, 255);
  color = pixels.gamma32(color);

  for (int i = 0; i < NUMPIXELS; i++) 
  {
    pixels.setPixelColor(i, color);
  }
  pixels.show();
}

void partyModeTick() 
{
  static uint32_t nextStepMs = 0;
  static uint32_t nextSparkleMs = 0;

  static uint16_t hue = 0; // base hue that moves
  static uint8_t  pulse = 180; // brightness pulse
  static int8_t   pulseDir = 1;

  uint32_t now = millis();
  if (now < nextStepMs) return;

  // Main update rate (fast enough to feel lively)
  nextStepMs = now + 12;  // ~50 FPS

  // Move hue quickly for energetic color changes
  hue += 900;  // increase for faster color cycling

  // Small triangle-wave pulse (not a harsh blink)
  pulse += pulseDir * 3;
  if (pulse > 245) { pulse = 245; pulseDir = -1; }
  if (pulse < 120) { pulse = 120; pulseDir =  1; }

  // Paint a rotating rainbow around the ring
  for (int i = 0; i < NUMPIXELS; i++) 
  {
    uint16_t localHue = hue + (uint16_t)(i * (65535UL / NUMPIXELS));
    uint32_t c = pixels.ColorHSV(localHue, 255, pulse);
    pixels.setPixelColor(i, pixels.gamma32(c));
  }

  // Add occasional sparkles (random pixels flash white)
  if (now >= nextSparkleMs) 
  {
    nextSparkleMs = now + (uint32_t)random(60, 180);

    int count = random(1, 3); // 1–2 sparkles
    for (int s = 0; s < count; s++) 
    {
      int p = random(NUMPIXELS);
      pixels.setPixelColor(p, 255, 255, 255);  // sparkle white
    }
  }

  pixels.show();
}

void lightningStormTick() 
{
  // Types of storm behavior
  enum Phase : uint8_t { IDLE, PRE_FLASH, STRIKE_ON, STRIKE_OFF };
  static Phase phase = IDLE;

  static uint32_t nextMs = 0;
  static uint8_t strikesLeft = 0;

  uint32_t now = millis();
  if (now < nextMs) return;

  switch (phase) 
  {
    case IDLE: 
    {
      // Long random calm/dark period between storms
      // (If you want a dim "cloud glow", you can set a very low blue here.)
      for (int i = 0; i < NUMPIXELS; i++) pixels.setPixelColor(i, 0, 0, 0);
      pixels.show();

      // Wait 1–6 seconds before next activity
      nextMs = now + (uint32_t)random(1000, 6000);

      // Sometimes do a subtle pre-flash before a strike sequence
      if (random(0, 100) < 40) 
      {
        phase = PRE_FLASH;
      } else 
      {
        phase = STRIKE_ON;
        strikesLeft = random(2, 7); // 2–6 strikes in a burst
      }
      break;
    }

    case PRE_FLASH: 
    {
      // Dim bluish-white pre-flash (far-away lightning)
      uint8_t v = (uint8_t)random(10, 60);
      for (int i = 0; i < NUMPIXELS; i++) pixels.setPixelColor(i, v, v, (uint8_t)(v + 10));
      pixels.show();

      // Brief pre-flash
      nextMs = now + (uint32_t)random(30, 120);

      // Turn it back off next
      phase = STRIKE_OFF;
      strikesLeft = random(2, 7);
      break;
    }

    case STRIKE_ON: 
    {
      // Bright lightning flash: white with a hint of blue
      uint8_t intensity = (uint8_t)random(180, 255);

      // Slight per-pixel variation makes it feel less uniform
      for (int i = 0; i < NUMPIXELS; i++) 
      {
        uint8_t jitter = (uint8_t)random(0, 40);
        uint8_t w = (uint8_t)max(0, intensity - jitter);
        pixels.setPixelColor(i, w, w, (uint8_t)min(255, w + 20));
      }
      pixels.show();

      // Flash duration (very short, lightning-like)
      nextMs = now + (uint32_t)random(20, 80);
      phase = STRIKE_OFF;
      break;
    }

    case STRIKE_OFF: 
    {
      // Darkness between strikes
      for (int i = 0; i < NUMPIXELS; i++) pixels.setPixelColor(i, 0, 0, 0);
      pixels.show();

      // Short gap between strikes in a burst
      nextMs = now + (uint32_t)random(40, 200);

      if (strikesLeft > 0) 
      {
        strikesLeft--;
        phase = STRIKE_ON;
      } else 
      {
        phase = IDLE; // back to calm
      }
      break;
    }
  }
}

void setAll(uint8_t r, uint8_t g, uint8_t b) 
{
  for (int i = 0; i < NUMPIXELS; i++) pixels.setPixelColor(i, r, g, b);
  pixels.show();
}

bool buttonPressedEdge() 
{
  // simple debounce + edge detect
  static bool lastReading = HIGH;
  static bool debouncedState = HIGH;
  static uint32_t lastChangeMs = 0;

  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading) 
  {
    lastChangeMs = millis();
    lastReading = reading;
  }

  if ((millis() - lastChangeMs) > 25) 
  { // debounce time
    if (reading != debouncedState) 
    {
      debouncedState = reading;
      if (debouncedState == LOW) return true; // pressed (to GND)
    }
  }
  return false;
}

void setup() 
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pixels.begin();
  pixels.setBrightness(255);  // master brightness cap (tune) TODO: make this controlable via a double click or something
  randomSeed(analogRead(0));

  setAll(0, 0, 0);
}

void loop()
{
  // Mode toggle
  if (buttonPressedEdge())
    mode = (Mode)((mode + 1) % 11);

  // Run current mode
  switch (mode) 
  {
    case MODE_CANDLE:
      candleTick();
      break;

    case MODE_SOLID:
      setAll(255, 140, 20);
      break;

    case MODE_RAINBOW:
      rainbowFadeTick();
      break;
    
    case MODE_PURPLE:
      setAll(191, 0, 255);
      break;

    case MODE_GREEN:
      setAll(0,255,0);
      break;
    
    case MODE_RED:
      setAll(255,0,0);
      break;

    case MODE_BLUE:
      setAll(0, 0, 255);
      break;

    case MODE_WHITE:
      setAll(255,200,120);
      break;

    case MODE_PARTY:
      partyModeTick();
      break;

    case MODE_LIGHTENING:
      lightningStormTick();
      break;

    case MODE_OFF:
      setAll(0, 0, 0);
      break;
  }
}
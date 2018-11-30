#include <Adafruit_NeoPixel.h>
#define NEOPIXEL_PIN 0
#define BUTTON_PIN 2
#define LED_COUNT 16
#define BRIGHTNESS_DEFAULT 60
#define COUNTDOWN_MAX 360
#define MODE_MAX 3

Adafruit_NeoPixel ring = Adafruit_NeoPixel(LED_COUNT, NEOPIXEL_PIN);

bool     isBtnHeldDown, // Is the button currently pressed
         isManuallySwitchingMode,
         isFadeIn; // Direction of ColourFade() animation (fade in | fade out)
uint8_t  brightness = BRIGHTNESS_DEFAULT, // Current brightness of LEDS
         mode = 0,  // The current animation effect
         colourCountdown = 0, // Seconds until next colour change
         modeCountdown = 0; // Seconds until next mode change         
int16_t offset = 0; // Current offset used by SpinnyWheels() and Rainbow() animation
uint32_t color  = 0xffff00; // Starting colour is yellow
uint32_t currentTime, prevTime; // Used to calculate ticks

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  ring.begin();
  ring.setBrightness(brightness);  
  colourCountdown = NextCountdownValue();
  modeCountdown = NextCountdownValue();
  prevTime = millis();
}

void loop() {
  currentTime = millis();

    // Do single frame of animation depending on current mode
    switch(mode) {
      case 0: Mode_ColourFade(); break;
      case 1: Mode_RandomSparks(); break;
      case 2: Mode_SpinnyWheels(); break;
      case 3: Mode_Rainbow(); break;
    }

    if(Tick()){
      // Run each action every time its countdown timer resets
      Countdown(SwitchMode, &modeCountdown);
      Countdown(SwitchColour, &colourCountdown);
      prevTime = millis();
    }

    // Allow manual changes in mode and colour
    //Delays here are to make button presses a bit more bearable
    if(IsBtnPressed()) {
      // Give 6 minutes of a user chosen colour
      SwitchColour();
      colourCountdown = COUNTDOWN_MAX;
      delay(100);
    }
    else if(isBtnHeldDown && !isManuallySwitchingMode){ 
      // Give 6 minutes of a user chosen mode
      isManuallySwitchingMode = true;
      SwitchMode();
      modeCountdown = COUNTDOWN_MAX;
      delay(100);
    }
    else if(!isBtnHeldDown && isManuallySwitchingMode) {
      // Reset manual mode swich after button release
      isManuallySwitchingMode = false;
      delay(100);
    }
}

/* Basic Funcs */

bool Tick() {
  // Around one second has passed this will return true
  bool shouldTick = (currentTime - prevTime) > 1000;

  // Slow down the animation a little bit
  if(!shouldTick) delay(100);
  
  return shouldTick;
}

bool IsBtnPressed(){
  // Returns true when the button is initially pressed
  bool wasBtnPressed = isBtnHeldDown;
  isBtnHeldDown = digitalRead(BUTTON_PIN) == LOW;
  return isBtnHeldDown && !wasBtnPressed;
}

uint8_t NextCountdownValue() {
  // Pick a new time between 5 seconds and 6 minutes
  return random(COUNTDOWN_MAX) + 5;
}

void Countdown(void (*action)(), uint8_t* value)
{
  if((*value) <= 0) {
    // Run action when currentValue reaches zero, then reset the countdown
    (*action)();
    (*value) = NextCountdownValue();
  }
  else { 
    // Reduce currentValue by 1 
    (*value)--;
  }
}

void SwitchMode() {
  // Iterate LED animation mode
  mode++;
  if(mode > MODE_MAX) mode = 0;

  // Reset neopixel ring
  offset = 0;
  brightness = BRIGHTNESS_DEFAULT;
  ring.setBrightness(brightness);
  for(uint8_t i = 0; i < LED_COUNT; i++) {
    ring.setPixelColor(i, 0);
  }
}

void SwitchColour() {
  color = Wheel(random(255));
}

/* LED Display Funcs */

void Mode_ColourFade() {
  for(uint8_t i = 0; i < LED_COUNT; i++) {
    ring.setPixelColor(i, color);
  }

  // Fade in or out depending on the current fade direction
  brightness += isFadeIn ? 5 : -5;
   
  if(brightness < 0 || brightness > 60) {    
    isFadeIn = !isFadeIn;
  } else {
    ring.setBrightness(brightness);
    ring.show();
  }
}

// Random sparks - just one LED on at a time!
void Mode_RandomSparks() {
  uint8_t i = random(LED_COUNT);
  ring.setPixelColor(i, color);
  ring.show();
  ring.setPixelColor(i, 0);
}

// Spinny wheels (4 LEDs on at a time)
void Mode_SpinnyWheels() {
  for(uint8_t i = 0; i < LED_COUNT; i++) {
    uint32_t c = 0;
    if(((offset + i) & 7) < 2) c = color;
    ring.setPixelColor(i, c);
  }
  ring.show();

  // Iterate or clear if it gets near int16_t.MAX(32,767) but thats unlikely
  offset = (offset > 32000) ? 0 : offset + 1;
}

// Slightly different, this makes the rainbow equally distributed throughout
void Mode_Rainbow() {
  // 5 cycles of all colors on wheel
  offset = (offset >= 256 * 5) ? 0 : offset + 5;
  
  // Draw current animation frame
  for(uint8_t i = 0; i < LED_COUNT; i++) {
    ring.setPixelColor(i, Wheel(((i * 256 / LED_COUNT) + offset) & 255));
  }  
  ring.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return ring.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return ring.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return ring.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}


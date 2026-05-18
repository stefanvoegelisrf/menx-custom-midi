#include "Adafruit_seesaw.h"
#include <seesaw_neopixel.h>

#define I2C_SDA 14
#define I2C_SCL 13

#define ROTARY_SWITCH_PIN 24

#define SLIDER_ANALOGIN 18

#define SLIDER1_ADDRESS 0x30
#define SLIDER2_ADDRESS 0x31

#define ROTARY_EYE1_ADDRESS 0x36
#define ROTARY_EYE2_ADDRESS 0x37
#define ROTARY_NOSE1_ADDRESS 0x38
#define ROTARY_NOSE2_ADDRESS 0x39
#define ROTARY_EAR1_ADDRESS 0x3A
#define ROTARY_EAR2_ADDRESS 0x3B

#define BUTTONS1_ADDRESS 0x32
#define BUTTONS2_ADDRESS 0x33

const uint8_t ENCODER_ADDRESSES[6] = {
  ROTARY_EYE1_ADDRESS,
  ROTARY_EYE2_ADDRESS,
  ROTARY_NOSE1_ADDRESS,
  ROTARY_NOSE2_ADDRESS,
  ROTARY_EAR1_ADDRESS,
  ROTARY_EAR2_ADDRESS
};

const char* ENCODER_NAMES[6] = {
  "Eye 1", "Eye 2", "Nose 1", "Nose 2", "Ear 1", "Ear 2"
};

Adafruit_seesaw rotary_encoders[6] = {
  Adafruit_seesaw(&Wire1),
  Adafruit_seesaw(&Wire1),
  Adafruit_seesaw(&Wire1),
  Adafruit_seesaw(&Wire1),
  Adafruit_seesaw(&Wire1),
  Adafruit_seesaw(&Wire1)
};

int32_t encoder_positions[] = { 0, 0, 0, 0, 0, 0 };

void setupEncoders() {
  for (int i = 0; i < 6; i++) {
    Serial.print("Initializing ");
    Serial.print(ENCODER_NAMES[i]);
    Serial.print(" at address 0x");
    Serial.println(ENCODER_ADDRESSES[i], HEX);

    // Initialize the encoder with its specific address
    if (!rotary_encoders[i].begin(ENCODER_ADDRESSES[i])) {
      Serial.print("Error: Couldn't find encoder: ");
      Serial.println(ENCODER_NAMES[i]);
      while (1) delay(10);
    }

    // Check product firmware version
    uint32_t version = ((rotary_encoders[i].getVersion() >> 16) & 0xFFFF);
    if (version != 4991) {
      Serial.print("Error: Wrong firmware loaded on ");
      Serial.print(ENCODER_NAMES[i]);
      Serial.print("? Version: ");
      Serial.println(version);
      while (1) delay(10);
    }

    // Configure the built-in switch (using standard seesaw SS_SWITCH pin)
    rotary_encoders[i].pinMode(ROTARY_SWITCH_PIN, INPUT_PULLUP);

    // Get starting position
    encoder_positions[i] = rotary_encoders[i].getEncoderPosition();

    // Enable interrupts for the encoder
    rotary_encoders[i].setGPIOInterrupts((uint32_t)1 << ROTARY_SWITCH_PIN, 1);
    rotary_encoders[i].enableEncoderInterrupt();
  }

  Serial.println("All encoders started successfully!\n");
}

void setupSliders() {
}

void setupButtons() {
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  // Set ESP32 pins manually, as they are not on the default pins
  Wire1.setPins(I2C_SDA, I2C_SCL);

  setupEncoders();
  setupSliders();
  setupButtons();
}

void loop() {
  // Check all the encoders
  for (int i = 0; i < 6; i++) {
    // Read current position
    int32_t new_position = rotary_encoders[i].getEncoderPosition();

    // If position changed, print it out
    if (new_position != encoder_positions[i]) {
      Serial.print(ENCODER_NAMES[i]);
      Serial.print(" position: ");
      Serial.println(new_position);
      encoder_positions[i] = new_position;
    }

    // Check if the encoder button is pressed
    // Note: The switch is active LOW (0 when pressed)
    if (!rotary_encoders[i].digitalRead(ROTARY_SWITCH_PIN)) {
      Serial.print(ENCODER_NAMES[i]);
      Serial.println(" BUTTON PRESSED!");
      delay(100);  // Simple debounce for testing
    }
  }

  // don't overwhelm serial port
  delay(10);
}
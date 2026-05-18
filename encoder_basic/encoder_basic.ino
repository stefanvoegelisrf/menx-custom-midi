#include "Adafruit_seesaw.h"
#include "Adafruit_NeoKey_1x4.h"

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

#define KEY_ROWS 2
#define KEY_COLUMNS 4

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

const uint8_t ENCODER_ADDRESSES[] = {
  ROTARY_EYE1_ADDRESS,
  ROTARY_EYE2_ADDRESS,
  ROTARY_NOSE1_ADDRESS,
  ROTARY_NOSE2_ADDRESS,
  ROTARY_EAR1_ADDRESS,
  ROTARY_EAR2_ADDRESS
};

const char* ENCODER_NAMES[] = {
  "Eye 1", "Eye 2", "Nose 1", "Nose 2", "Ear 1", "Ear 2"
};

Adafruit_seesaw rotary_encoders[6];

int32_t encoder_positions[] = { 0, 0, 0, 0, 0, 0 };

const uint8_t SLIDER_ADDRESSES[] = {
  SLIDER1_ADDRESS,
  SLIDER2_ADDRESS
};

const char* SLIDER_NAMES[] = {
  "Eye Brow Slider 1", "Eye Brow Slider 2"
};

Adafruit_seesaw sliders[2];

uint16_t slider_values[] = { 0, 0 };

Adafruit_NeoKey_1x4 neokey_buttons[KEY_ROWS][KEY_COLUMNS / 4] = {
  { Adafruit_NeoKey_1x4(BUTTONS1_ADDRESS) },
  { Adafruit_NeoKey_1x4(BUTTONS2_ADDRESS) },
};

Adafruit_MultiNeoKey1x4 neokeys((Adafruit_NeoKey_1x4*)neokey_buttons, KEY_ROWS, KEY_COLUMNS / 4);

void setupEncoders() {
  for (int i = 0; i < ARRAY_SIZE(rotary_encoders); i++) {
    Serial.print("Initializing ");
    Serial.print(ENCODER_NAMES[i]);
    Serial.print(" at address 0x");
    Serial.println(ENCODER_ADDRESSES[i], HEX);

    rotary_encoders[i] = Adafruit_seesaw(&Wire1);

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
  for (int i = 0; i < ARRAY_SIZE(sliders); i++) {
    Serial.print("Initializing ");
    Serial.print(SLIDER_NAMES[i]);
    Serial.print(" at address 0x");
    Serial.println(SLIDER_ADDRESSES[i], HEX);

    sliders[i] = Adafruit_seesaw(&Wire1);

    if (!sliders[i].begin(SLIDER_ADDRESSES[i])) {
      Serial.println(F("Slider not found!"));
      while (1) delay(10);
    }

    uint16_t pid;
    uint8_t year, mon, day;

    sliders[i].getProdDatecode(&pid, &year, &mon, &day);
    Serial.print("Slider found PID: ");
    Serial.print(pid);
    Serial.print(" datecode: ");
    Serial.print(2000 + year);
    Serial.print("/");
    Serial.print(mon);
    Serial.print("/");
    Serial.println(day);

    if (pid != 5295) {
      Serial.println(F("Wrong slider PID"));
      while (1) delay(10);
    }
  }

  Serial.println("All sliders started successfully!\n");
}

void setupButtons() {
  if (!neokeys.begin()) {  // start matrix
    Serial.println("Could not start NeoKeys, check wiring?");
    while (1) delay(10);
  }

  Serial.println("NeoKeys started!");

  // activate all keys and set callbacks
  for (int y = 0; y < KEY_ROWS; y++) {
    for (int x = 0; x < KEY_COLUMNS; x++) {
      neokeys.registerCallback(x, y, buttonPressed);
    }
  }
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
  for (int i = 0; i < ARRAY_SIZE(rotary_encoders); i++) {
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

  for (int i = 0; i < ARRAY_SIZE(sliders); i++) {
    uint16_t slider_value = sliders[i].analogRead(SLIDER_ANALOGIN);
    if (slider_value != slider_values[i]) {
      Serial.print(SLIDER_NAMES[i]);
      Serial.print(" position: ");
      Serial.println(slider_value);
      slider_values[i] = slider_value;
    }
  }

  neokeys.read();

  // don't overwhelm serial port
  delay(10);
}

NeoKey1x4Callback buttonPressed(keyEvent e) {
  uint8_t key = e.bit.NUM;
  if (e.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
    Serial.print("Key press ");
    Serial.println(key);

  } else if (e.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
    Serial.print("Key release ");
    Serial.println(key);
  }

  return 0;
}
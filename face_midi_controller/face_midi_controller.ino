#include "Adafruit_seesaw.h"
#include "Adafruit_NeoKey_1x4.h"
#include <Control_Surface.h>

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

const uint8_t FIXED_TRIGGER_VELOCITY = 127;

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

Adafruit_seesaw RotaryEncoders[6] = {
  Adafruit_seesaw(&Wire1),
  Adafruit_seesaw(&Wire1),
  Adafruit_seesaw(&Wire1),
  Adafruit_seesaw(&Wire1),
  Adafruit_seesaw(&Wire1),
  Adafruit_seesaw(&Wire1)
};

int32_t EncoderPositions[] = { 0, 0, 0, 0, 0, 0 };

bool LastEncoderSwitchStates[] = { true, true, true, true, true, true };

const uint8_t SLIDER_ADDRESSES[] = {
  SLIDER1_ADDRESS,
  SLIDER2_ADDRESS
};

const char* SLIDER_NAMES[] = {
  "Eye Brow Slider 1", "Eye Brow Slider 2"
};

Adafruit_seesaw Sliders[2] = {
  Adafruit_seesaw(&Wire1),
  Adafruit_seesaw(&Wire1)
};

uint16_t SliderValues[] = { 0, 0 };

Adafruit_NeoKey_1x4 NeoKeyButtons[KEY_ROWS][KEY_COLUMNS / 4] = {
  { Adafruit_NeoKey_1x4(BUTTONS1_ADDRESS, &Wire1) },
  { Adafruit_NeoKey_1x4(BUTTONS2_ADDRESS, &Wire1) },
};

Adafruit_MultiNeoKey1x4 NeoKeys((Adafruit_NeoKey_1x4*)NeoKeyButtons, KEY_ROWS, KEY_COLUMNS / 4);

USBMIDI_Interface midi;

NeoKey1x4Callback buttonPressed(keyEvent e);

void setupEncoders() {
  for (int i = 0; i < ARRAY_SIZE(RotaryEncoders); i++) {
    Serial.print("Initializing ");
    Serial.print(ENCODER_NAMES[i]);
    Serial.print(" at address 0x");
    Serial.println(ENCODER_ADDRESSES[i], HEX);

    if (!RotaryEncoders[i].begin(ENCODER_ADDRESSES[i])) {
      Serial.print("Error: Couldn't find encoder: ");
      Serial.println(ENCODER_NAMES[i]);
      while (1) delay(10);
    }

    // Check product firmware version
    uint32_t version = ((RotaryEncoders[i].getVersion() >> 16) & 0xFFFF);
    if (version != 4991) {
      Serial.print("Error: Wrong firmware loaded on ");
      Serial.print(ENCODER_NAMES[i]);
      Serial.print("? Version: ");
      Serial.println(version);
      while (1) delay(10);
    }

    // Configure the built-in switch
    RotaryEncoders[i].pinMode(ROTARY_SWITCH_PIN, INPUT_PULLUP);

    // Get starting position
    EncoderPositions[i] = RotaryEncoders[i].getEncoderPosition();

    // Enable interrupts for the encoder
    RotaryEncoders[i].setGPIOInterrupts((uint32_t)1 << ROTARY_SWITCH_PIN, 1);
    RotaryEncoders[i].enableEncoderInterrupt();
  }

  Serial.println("All encoders started successfully!\n");
}

void setupSliders() {
  for (int i = 0; i < ARRAY_SIZE(Sliders); i++) {
    Serial.print("Initializing ");
    Serial.print(SLIDER_NAMES[i]);
    Serial.print(" at address 0x");
    Serial.println(SLIDER_ADDRESSES[i], HEX);

    if (!Sliders[i].begin(SLIDER_ADDRESSES[i])) {
      Serial.println(F("Slider not found!"));
      while (1) delay(10);
    }

    uint16_t pid;
    uint8_t year, mon, day;
    Sliders[i].getProdDatecode(&pid, &year, &mon, &day);
    if (pid != 5295) {
      Serial.println(F("Wrong slider PID"));
      while (1) delay(10);
    }
  }

  Serial.println("All sliders started successfully!\n");
}

void setupKeys() {
  if (!NeoKeys.begin()) {
    Serial.println("Could not start NeoKeys, check wiring?");
    while (1) delay(10);
  }

  Serial.println("NeoKeys started!");

  for (int y = 0; y < KEY_ROWS; y++) {
    for (int x = 0; x < KEY_COLUMNS; x++) {
      NeoKeys.registerCallback(x, y, buttonPressed);
    }
  }
}

void checkEncoders() {
  for (int i = 0; i < ARRAY_SIZE(RotaryEncoders); i++) {
    // Read current position
    int32_t newPosition = RotaryEncoders[i].getEncoderPosition();

    Channel midiChannel = (i % 2 == 0) ? Channel_1 : Channel_2;

    if (newPosition != EncoderPositions[i]) {
      Serial.print(ENCODER_NAMES[i]);
      Serial.print(" position: ");
      Serial.println(newPosition);
      // Note: C cuts off the decimal places in this division, so we get the correct CC number
      // e.g. Rotary Encoder Eye 1 has i=0 -> i/2=0 -> 0*2=0, Rotary Encoder Eye 2 has i=1 -> i/2=0.5 -> C cuts off decimal -> 0*2=0
      MIDIAddress ccAddress = MIDIAddress(20 + (i / 2) * 2, midiChannel);
      int32_t difference = newPosition - EncoderPositions[i];
      midi.sendCC(ccAddress, difference > 0 ? 1 : 127);

      EncoderPositions[i] = newPosition;
    }

    // Check if the encoder button is pressed
    // Note: The switch is active LOW (0 when pressed)
    bool currentSwitchState = RotaryEncoders[i].digitalRead(ROTARY_SWITCH_PIN);
    if (currentSwitchState != LastEncoderSwitchStates[i]) {
      Serial.print(ENCODER_NAMES[i]);
      Serial.println(" BUTTON PRESSED!");
      MIDIAddress ccSwitchAddress = MIDIAddress(21 + (i / 2) * 2, midiChannel);
      MIDIAddress noteAddress = MIDIAddress(36 + (i / 2), midiChannel);

      if (!currentSwitchState) {  // Pressed
        midi.sendCC(ccSwitchAddress, FIXED_TRIGGER_VELOCITY);
        midi.sendNoteOn(noteAddress, FIXED_TRIGGER_VELOCITY);
      } else {
        midi.sendCC(ccSwitchAddress, 0);
        midi.sendNoteOff(noteAddress, 0);
      }
      LastEncoderSwitchStates[i] = currentSwitchState;
    }
  }
}

void checkSliders() {
  for (int i = 0; i < ARRAY_SIZE(Sliders); i++) {
    uint16_t sliderValue = Sliders[i].analogRead(SLIDER_ANALOGIN);
    if (sliderValue != SliderValues[i]) {
      Serial.print(SLIDER_NAMES[i]);
      Serial.print(" position: ");
      Serial.println(sliderValue);
      Channel midiChannel = (i == 0) ? Channel_1 : Channel_2;
      MIDIAddress ccAddress = MIDIAddress(10, midiChannel);
      uint8_t midiValue = map(sliderValue, 0, 1023, 0, 127);
      midi.sendCC(ccAddress, midiValue);
      SliderValues[i] = sliderValue;
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  // Set ESP32 pins manually, as they are not on the default pins
  Wire1.setPins(I2C_SDA, I2C_SCL);

  setupSliders();
  setupEncoders();
  setupKeys();
  midi.begin();
}

void loop() {
  checkEncoders();
  checkSliders();
  NeoKeys.read();
  midi.update();
  // don't overwhelm serial port
  delay(10);
}

NeoKey1x4Callback buttonPressed(keyEvent e) {
  uint8_t keyIndex = e.bit.NUM;

  Channel midiChannel = (keyIndex < 4) ? Channel_1 : Channel_2;
  uint8_t offset = (keyIndex < 4) ? 0 : 4;

  uint8_t controlChangeNumber = 26 + (keyIndex - offset);
  uint8_t noteNumber = 40 + (keyIndex - offset);

  MIDIAddress ccAddress = MIDIAddress(controlChangeNumber, midiChannel);
  MIDIAddress noteAddress = MIDIAddress(noteNumber, midiChannel);

  if (e.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
    Serial.print("Key press ");
    Serial.println(keyIndex);
    midi.sendNoteOn(noteAddress, FIXED_TRIGGER_VELOCITY);
    midi.sendCC(ccAddress, FIXED_TRIGGER_VELOCITY);

  } else if (e.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
    Serial.print("Key release ");
    Serial.println(keyIndex);
    midi.sendNoteOff(noteAddress, 0);
    midi.sendCC(ccAddress, 0);
  }

  return 0;
}
/*
       MIDI to CV for reSEMble synth

       For RP2040

       Filesystem 1.5Mb/0.5Mb

      Version 1.2

      Copyright (C) 2020 Craig Barnes

      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 3 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License <http://www.gnu.org/licenses/> for more details.
*/
#include <Arduino.h>
#include <iostream>
#include <cfloat>  // Include for DBL_MAX
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MIDI.h>
#include "Parameters.h"
#include "Hardware.h"
#include <Adafruit_NeoPixel.h>
#include <FS.h>
#include <LittleFS.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

struct VoiceAndNote {
  int note;
  int velocity;
  long timeOn;
  bool sustained;  // Sustain flag
  bool keyDown;
  double noteFreq;  // Note frequency
  int position;
  bool noteOn;
};

// Struct to store note data with derivatives
struct NoteData {
  int value;
  double derivative;
};

struct VoiceAndNote voices[NO_OF_VOICES] = {
  { -1, -1, 0, false, false, 0, -1, false },
};

boolean voiceOn[NO_OF_VOICES] = { false };
bool notes[128] = { 0 }, initial_loop = 1;
int8_t noteOrder[40] = { 0 }, orderIndx = { 0 };
bool S1, S2;

// MIDI setup

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {

  Serial.begin(115200);

  Wire.setSDA(12);
  Wire.setSCL(13);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
    return;
  }

  Serial.println("LittleFS mounted successfully");

  SPI.setSCK(2);
  SPI.setRX(4);
  SPI.setTX(3);
  SPI.begin();

  delay(10);

  setupHardware();

  //MIDI 5 Pin DIN
  MIDI.begin(0);
  MIDI.setHandleNoteOn(myNoteOn);
  MIDI.setHandleNoteOff(myNoteOff);
  MIDI.setHandlePitchBend(myPitchBend);
  MIDI.setHandleControlChange(myControlChange);
  MIDI.setHandleAfterTouchChannel(myAfterTouch);
  Serial.println("MIDI In DIN Listening");

  pixels.begin();
  delay(500);
  pixels.setPixelColor(0, pixels.Color(0, 255, 0));  // GREEN
  pixels.show();

  loadSettingsFromSD();

  lastMenuInteraction = millis();
  settingsSaved = true;

}

void saveSettingsToSD() {
  if (LittleFS.exists("settings.txt")) {
    LittleFS.remove("settings.txt");
  }

  File file = LittleFS.open("settings.txt", "w");
  if (file) {
    file.print("LFORATE,");
    file.println(LFORATE);
    file.print("BEND_WHEEL,");
    file.println(BEND_WHEEL);
    file.print("LFOWAVE,");
    file.println(LFOWAVE);
    file.print("FM_MOD_WHEEL,");
    file.println(FM_MOD_WHEEL);
    file.print("FM_AT_WHEEL,");
    file.println(FM_AT_WHEEL);
    file.print("portamentoOn,");
    file.println(portamentoOn);
    file.print("portamentoTime,");
    file.println(portamentoTime);
    file.print("POLYMODE,");
    file.println(POLYMODE);
    file.print("keyboardMode,");
    file.println(keyboardMode);
    file.print("DETUNE,");
    file.println(DETUNE);
    file.print("LFOMULT,");
    file.println(LFOMULT);
    file.print("LFOALT,");
    file.println(LFOALT);
    file.print("masterChan,");
    file.println(masterChan);
    file.print("portamentoTimestr,");
    file.println(portamentoTimestr);
    file.print("FM_MOD_WHEEL_STR,");
    file.println(FM_MOD_WHEEL_STR);
    file.print("FM_AT_WHEEL_STR,");
    file.println(FM_AT_WHEEL_STR);
    file.close();
    Serial.println("Settings saved to LittleFS.");
    // Trigger the "Settings Saved" message
    showSaveMessage = true;
    saveMessageStartTime = millis();
  } else {
    Serial.println("Error opening file for writing.");
  }
}

void loadSettingsFromSD() {
  File file = LittleFS.open("settings.txt", "r");
  if (file) {
    while (file.available()) {
      String line = file.readStringUntil('\n');
      int delimiterIndex = line.indexOf(',');
      if (delimiterIndex != -1) {
        String key = line.substring(0, delimiterIndex);
        String valueStr = line.substring(delimiterIndex + 1);
        int value = valueStr.toInt();

        if (key == "LFORATE") LFORATE = value;
        else if (key == "BEND_WHEEL") BEND_WHEEL = value;
        else if (key == "LFOWAVE") LFOWAVE = value;
        else if (key == "FM_MOD_WHEEL") FM_MOD_WHEEL = value;
        else if (key == "FM_AT_WHEEL") FM_AT_WHEEL = value;
        else if (key == "portamentoOn") portamentoOn = value;
        else if (key == "portamentoTime") portamentoTime = value;
        else if (key == "POLYMODE") POLYMODE = value;
        else if (key == "keyboardMode") keyboardMode = value;
        else if (key == "DETUNE") DETUNE = value;
        else if (key == "LFOMULT") LFOMULT = value;
        else if (key == "LFOALT") LFOALT = value;
        else if (key == "masterChan") masterChan = value;
        else if (key == "portamentoTimestr") portamentoTimestr = value;
        else if (key == "FM_MOD_WHEEL_STR") FM_MOD_WHEEL_STR = value;
        else if (key == "FM_AT_WHEEL_STR") FM_AT_WHEEL_STR = value;
      }
    }
    file.close();
    Serial.println("Settings loaded from LittleFS.");
  } else {
    Serial.println("Failed to open settings.txt on LittleFS.");
  }
}

void lfoalt_task() {
  if (LFOALT != lfoalt_old) {
    if (LFOALT) {
      digitalWrite(LFO_ALT, LOW);
    } else {
      digitalWrite(LFO_ALT, HIGH);
    }
    lfoalt_old = LFOALT;
  }
}

void loop() {
  updateMenu();

    // Handle the display of "Settings Saved" message
  if (showSaveMessage) {
    if (millis() - saveMessageStartTime < saveMessageDuration) {
      displaySaveMessage();
    } else {
      showSaveMessage = false;  // Stop showing the message after the duration
      displayMenu();            // Return to normal menu display
    }
  } else {
    displayMenu();
  }

  if (!settingsSaved && (millis() - lastMenuInteraction > 5000)) {
    saveSettingsToSD();
    settingsSaved = true;
  }
  
  updateTimers();
  MIDI.read(0);  //MIDI 5 Pin DIN
  mod_task();
  lfoalt_task();
  updateVoice1();
}

void updateMenu() {
  static bool lastUpState = HIGH, lastDownState = HIGH, lastSelectState = HIGH;
  static unsigned long upPressTime = 0;
  static unsigned long downPressTime = 0;
  static const unsigned long initialDelay = 300;  // Delay before auto-repeat starts (ms)
  static const unsigned long repeatRate = 50;    // Interval between repeats (ms)

  bool upState = digitalRead(SWITCH_UP);
  bool downState = digitalRead(SWITCH_DOWN);
  bool selectState = digitalRead(SWITCH_SELECT);

  unsigned long currentMillis = millis();

  // UP button logic
  if (upState == LOW && lastUpState == HIGH) {
    upPressTime = currentMillis;
    lastMenuInteraction = currentMillis;
    settingsSaved = false;

    if (!editing) {
      currentMenu = (MenuState)((currentMenu - 1 + MENU_TOTAL_ITEMS) % MENU_TOTAL_ITEMS);
    } else {
      incrementMenuValue(1);
    }
  } else if (upState == LOW && (currentMillis - upPressTime > initialDelay)) {
    // Auto-repeat after initial delay
    if ((currentMillis - upPressTime) % repeatRate < 10) {  // Adjust for repeat interval
      lastMenuInteraction = currentMillis;
      settingsSaved = false;
      if (editing) {
        incrementMenuValue(1);
      } else {
        currentMenu = (MenuState)((currentMenu - 1 + MENU_TOTAL_ITEMS) % MENU_TOTAL_ITEMS);
      }
    }
  }

  // DOWN button logic
  if (downState == LOW && lastDownState == HIGH) {
    downPressTime = currentMillis;
    lastMenuInteraction = currentMillis;
    settingsSaved = false;

    if (!editing) {
      currentMenu = (MenuState)((currentMenu + 1) % MENU_TOTAL_ITEMS);
    } else {
      incrementMenuValue(-1);
    }
  } else if (downState == LOW && (currentMillis - downPressTime > initialDelay)) {
    // Auto-repeat after initial delay
    if ((currentMillis - downPressTime) % repeatRate < 10) {
      lastMenuInteraction = currentMillis;
      settingsSaved = false;
      if (editing) {
        incrementMenuValue(-1);
      } else {
        currentMenu = (MenuState)((currentMenu + 1) % MENU_TOTAL_ITEMS);
      }
    }
  }

  // SELECT button logic (toggles editing mode)
  if (selectState == LOW && lastSelectState == HIGH) {
    editing = !editing;
    lastMenuInteraction = currentMillis;
    settingsSaved = false;
  }

  lastUpState = upState;
  lastDownState = downState;
  lastSelectState = selectState;
}

void incrementMenuValue(int delta) {
  switch (currentMenu) {
    case MENU_LFORATE:
      LFORATE = constrain(LFORATE + delta, 0, 127);
      break;
    case MENU_LFOWAVE:
      LFOWAVE = constrain(LFOWAVE + delta, 0, 7);
      break;
    case MENU_LFOMULT:
      LFOMULT = constrain(LFOMULT + delta, 0, 4);
      break;
    case MENU_LFOALT:
      LFOALT = !LFOALT;
      break;
    case MENU_BEND_WHEEL:
      BEND_WHEEL = constrain(BEND_WHEEL + delta, 0, 12);
      break;
    case MENU_FM_MOD_WHEEL:
      FM_MOD_WHEEL_STR = constrain(FM_MOD_WHEEL_STR + delta, 0, 127);
      FM_MOD_WHEEL = map(FM_MOD_WHEEL_STR, 0, 127, 0, 16.2);
      break;
    case MENU_FM_AT_WHEEL:
      FM_AT_WHEEL_STR = constrain(FM_AT_WHEEL_STR + delta, 0, 127);
      FM_AT_WHEEL = map(FM_AT_WHEEL_STR, 0, 127, 0, 16.2);
      break;
    case MENU_DETUNE:
      DETUNE = constrain(DETUNE + delta, 0, 127);
      break;
    case MENU_GLIDE_SW:
      portamentoOn = !portamentoOn;
      break;
    case MENU_GLIDE_TIME:
      portamentoTimestr = constrain(portamentoTimestr + delta, 0, 127);
      portamentoTime = map(portamentoTimestr, 0, 127, 0, 10000);
      break;
    case MENU_POLYMODE:
      POLYMODE = !POLYMODE;
      break;
    case MENU_NOTEPRIORITY:
      keyboardMode = constrain(keyboardMode + delta, 0, 2);
      break;
    case MENU_MIDICHANNEL:
      masterChan = constrain(masterChan + delta, 0, 16);
      break;
    default:
      break;
  }
}

void displayMenu() {
  display.clearDisplay();
  display.setCursor(0, 0);

  const int visibleItems = 4;
  int startItem = currentMenu - (currentMenu % visibleItems);

  for (int i = 0; i < visibleItems && (startItem + i) < MENU_TOTAL_ITEMS; i++) {
    int menuIndex = startItem + i;

    // Highlight logic
    if (menuIndex == currentMenu) {
      if (editing) {
        // Highlight the parameter being edited (inverse colors)
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);  // Inverse
        display.print("> ");
      } else {
        // Normal selection indicator
        display.setTextColor(SSD1306_WHITE);
        display.print("> ");
      }
    } else {
      display.setTextColor(SSD1306_WHITE);
      display.print("  ");
    }

    switch ((MenuState)menuIndex) {
      case MENU_LFORATE:
        display.print("LFO Rate: ");
        display.println(LFORATE);
        break;
      case MENU_LFOWAVE:
        display.print("LFO Wave: ");
        if (LFOALT == 1) {
          switch (LFOWAVE) {
            case 0:
              display.println("Saw+Oct");
              break;
            case 1:
              display.println("QuadSaw");
              break;
            case 2:
              display.println("QuadPulse");
              break;
            case 3:
              display.println("TriStep");
              break;
            case 4:
              display.println("Sine+Oct");
              break;
            case 5:
              display.println("Sine+3rd");
              break;
            case 6:
              display.println("Sine+4th");
              break;
            case 7:
              display.println("Slopes");
              break;
          }
        } else {
          switch (LFOWAVE) {
            case 0:
              display.println("Ramp Up");
              break;
            case 1:
              display.println("Ramp Down");
              break;
            case 2:
              display.println("Square");
              break;
            case 3:
              display.println("Triangle");
              break;
            case 4:
              display.println("Sine");
              break;
            case 5:
              display.println("Sweeps");
              break;
            case 6:
              display.println("Lumps");
              break;
            case 7:
              display.println("Random");
              break;
          }
        }
        break;
      case MENU_LFOMULT:
        display.print("LFO Mult: ");
        switch (LFOMULT) {
          case 0:
            display.println("x 0.5");
            break;
          case 1:
            display.println("x 1.0");
            break;
          case 2:
            display.println("x 1.5");
            break;
          case 3:
            display.println("x 2.0");
            break;
          case 4:
            display.println("x 3.0");
            break;
        }
        break;
      case MENU_LFOALT:
        display.print("LFO Alt:  ");
        display.println(LFOALT ? "On" : "Off");
        break;
      case MENU_BEND_WHEEL:
        display.print("Bend Depth: ");
        display.println(BEND_WHEEL);
        break;
      case MENU_FM_MOD_WHEEL:
        display.print("FM Depth:   ");
        display.println(FM_MOD_WHEEL_STR);
        break;
      case MENU_FM_AT_WHEEL:
        display.print("AT Depth:   ");
        display.println(FM_AT_WHEEL_STR);
        break;
      case MENU_DETUNE:
        display.print("Detune:     ");
        display.println(DETUNE);
        break;
      case MENU_GLIDE_SW:
        display.print("Glide SW:   ");
        display.println(portamentoOn ? "On" : "Off");
        break;
      case MENU_GLIDE_TIME:
        display.print("Glide Time: ");
        display.println(portamentoTimestr);
        break;
      case MENU_POLYMODE:
        display.print("PolyMode:   ");
        display.println(POLYMODE ? "On" : "Off");
        break;
      case MENU_NOTEPRIORITY:
        display.print("Priority:   ");
        switch (keyboardMode) {
          case 0:
            display.println("Bottom");
            break;
          case 1:
            display.println("Top");
            break;
          case 2:
            display.println("Last");
            break;
        }
        break;
      case MENU_MIDICHANNEL:
        display.print("Channel:    ");
        display.println(masterChan);
        break;
    }

    // Reset text color for the next line
    display.setTextColor(SSD1306_WHITE);

  }

  display.display();
}

void displaySaveMessage() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor((SCREEN_WIDTH - (6 * 12)) / 2, SCREEN_HEIGHT / 2 - 8);  // Center the message
  display.println("Saved");
  display.display();
  display.setTextSize(1);
}

void myPitchBend(byte channel, int bend) {
  if (channel == masterChan) {
    switch (BEND_WHEEL) {
      case 12:
        bend_data = map(bend, -8192, 8191, -3235, 3235);
        break;

      case 11:
        bend_data = map(bend, -8192, 8191, -2970, 2969);
        break;

      case 10:
        bend_data = map(bend, -8192, 8191, -2700, 2699);
        break;

      case 9:
        bend_data = map(bend, -8192, 8191, -2430, 2429);
        break;

      case 8:
        bend_data = map(bend, -8192, 8191, -2160, 2159);
        break;

      case 7:
        bend_data = map(bend, -8192, 8191, -1890, 1889);
        break;

      case 6:
        bend_data = map(bend, -8192, 8191, -1620, 1619);
        break;

      case 5:
        bend_data = map(bend, -8192, 8191, -1350, 1349);
        break;

      case 4:
        bend_data = map(bend, -8192, 8191, -1080, 1079);
        break;

      case 3:
        bend_data = map(bend, -8192, 8191, -810, 809);
        break;

      case 2:
        bend_data = map(bend, -8192, 8191, -540, 539);
        break;

      case 1:
        bend_data = map(bend, -8192, 8191, -270, 270);
        break;

      case 0:
        bend_data = 0;
        break;
    }
  }
}

// Reads MIDI CC messages
void myControlChange(byte channel, byte number, byte value) {
  if (channel == masterChan) {
    switch (number) {

      case 1:
        FM_RANGE_UPPER = int(value * FM_MOD_WHEEL);
        FM_RANGE_LOWER = (FM_RANGE_UPPER - FM_RANGE_UPPER - FM_RANGE_UPPER);
        break;

      case 5:                                           // Portamento time
        portamentoTimestr = value;
        portamentoTime = map(value, 0, 127, 0, 10000);  // Map to a max of 2 seconds
        break;

      case 15:
        DETUNE = value;
        break;

      case 16:
        BEND_WHEEL = map(value, 0, 127, 0, 12);
        break;

      case 17:
        FM_MOD_WHEEL = map(value, 0, 127, 0, 16.2);
        break;

      case 19:
        FM_AT_WHEEL = map(value, 0, 127, 0, 16.2);
        break;

      case 65:  // Portamento on/off
        switch (value) {
          case 127:
            portamentoOn = true;
            Serial.print("Portamento On: ");
            Serial.println(portamentoOn);
            break;
          case 0:
            portamentoOn = false;
            Serial.print("Portamento Off: ");
            Serial.println(portamentoOn);
            break;
        }
        break;

      case 123:
        switch (value) {
          case 127:
            allNotesOff();
            break;
        }
        break;

      case 127:
        keyboardMode = map(value, 0, 127, 0, 2);
        if (keyboardMode > 0 && keyboardMode < 3) {
          allNotesOff();
        }
        break;
    }
  }
}

void myAfterTouch(byte channel, byte value) {
  if (channel == masterChan) {
    FM_AT_RANGE_UPPER = int(value * FM_AT_WHEEL);
    FM_AT_RANGE_LOWER = (FM_AT_RANGE_UPPER - FM_AT_RANGE_UPPER - FM_AT_RANGE_UPPER);
  }
}

// Get the LFO input on the FM_INPUT
void mod_task() {

  MOD_VALUE = analogRead(FM_INPUT);
  FM_VALUE = map(MOD_VALUE, 0, 4095, FM_RANGE_LOWER, FM_RANGE_UPPER);
  FM_AT_VALUE = map(MOD_VALUE, 0, 4095, FM_AT_RANGE_LOWER, FM_AT_RANGE_UPPER);
}

void commandTopNote() {
  int topNote = 0;
  bool noteActive = false;

  for (int i = 0; i < 128; i++) {
    if (notes[i]) {
      topNote = i;
      noteActive = true;
    }
  }

  if (noteActive) {
    commandNote(topNote);
  } else {  // All notes are off, turn off gate
    digitalWrite(GATE_NOTE1, LOW);
  }
}

void commandBottomNote() {
  int bottomNote = 0;
  bool noteActive = false;

  for (int i = 87; i >= 0; i--) {
    if (notes[i]) {
      bottomNote = i;
      noteActive = true;
    }
  }

  if (noteActive) {
    commandNote(bottomNote);
  } else {  // All notes are off, turn off gate
    digitalWrite(GATE_NOTE1, LOW);
  }
}

void commandLastNote() {

  int8_t noteIndx;

  for (int i = 0; i < 40; i++) {
    noteIndx = noteOrder[mod(orderIndx - i, 40)];
    if (notes[noteIndx]) {
      commandNote(noteIndx);
      return;
    }
  }
  digitalWrite(GATE_NOTE1, LOW);  // All notes are off
}

void commandNote(int noteMsg) {
  note1 = noteMsg;
  digitalWrite(GATE_NOTE1, HIGH);
  digitalWrite(TRIG_NOTE1, HIGH);
  noteTrig[0] = millis();
}

// MIDI note on callback
void myNoteOn(byte channel, byte note, byte velocity) {
  if (channel == masterChan) {

    //Check for out of range notes
    if (note < 0 || note > 127) return;

    // Calculate new target mV values including all real-time corrections
    targetMV_a = (unsigned int)(((float)(note + transpose + realoctave) * NOTE_SF + 0.5));

    targetMV_b = (unsigned int)(((float)(note + transpose + realoctave) * NOTE_SF + 0.5));

    targetMV_c = (unsigned int)(((float)(note + transpose + realoctave) * NOTE_SF + 0.5));

    if (portamentoOn) {
      // If portamento is enabled, start glide from the **current** value
      initialMV_a = currentMV_a;
      initialMV_b = currentMV_b;
      initialMV_c = currentMV_c;
      glideStartTime = millis();  // Start the glide transition
      portamentoActive = true;
    } else {
      // If portamento is OFF, instantly set frequency
      currentMV_a = targetMV_a;
      currentMV_b = targetMV_b;
      currentMV_c = targetMV_c;
      portamentoActive = false;
    }

    switch (keyboardMode) {

      case 0:
      case 1:
      case 2:
        if (keyboardMode == 0) {
          S1 = 1;
          S2 = 1;
        }
        if (keyboardMode == 1) {
          S1 = 0;
          S2 = 1;
        }
        if (keyboardMode == 2) {
          S1 = 0;
          S2 = 0;
        }
        noteMsg = note;

        if (velocity == 0) {
          notes[noteMsg] = false;
        } else {
          notes[noteMsg] = true;
        }
        voices[0].velocity = velocity;

        if (S1 && S2) {  // Highest note priority
          commandTopNote();
        } else if (!S1 && S2) {  // Lowest note priority
          commandBottomNote();
        } else {                 // Last note priority
          if (notes[noteMsg]) {  // If note is on and using last note priority, add to ordered list
            orderIndx = (orderIndx + 1) % 40;
            noteOrder[orderIndx] = noteMsg;
          }
          commandLastNote();
        }
        break;
    }
  }
}

// MIDI Note Off callback
void myNoteOff(byte channel, byte note, byte velocity) {
  if (channel == masterChan) {

    switch (keyboardMode) {
      case 0:
      case 1:
      case 2:
        if (keyboardMode == 0) {
          S1 = 1;
          S2 = 1;
        }
        if (keyboardMode == 1) {
          S1 = 0;
          S2 = 1;
        }
        if (keyboardMode == 2) {
          S1 = 0;
          S2 = 0;
        }
        noteMsg = note;

        notes[noteMsg] = false;

        if (S1 && S2) {  // Highest note priority
          commandTopNote();
        } else if (!S1 && S2) {  // Lowest note priority
          commandBottomNote();
        } else {                 // Last note priority
          if (notes[noteMsg]) {  // If note is on and using last note priority, add to ordered list
            orderIndx = (orderIndx + 1) % 40;
            noteOrder[orderIndx] = noteMsg;
          }
          commandLastNote();
        }
        break;
    }
  }
}

// Kills the trigger after 20 mS
void updateTimers() {

  if (millis() > noteTrig[0] + trigTimeout) {
    digitalWrite(TRIG_NOTE1, LOW);  // Set trigger low after 20 msec
  }
}

// Updates the DAC with the note1 data and adds in all offsets, bend, modulation, aftertouch, octave range
void updateVoice1() {
  static unsigned long lastUpdateTime = 0;  // Tracks the last time this function was called
  unsigned long currentTime = millis();     // Get the current time in milliseconds
  unsigned long deltaTime = currentTime - lastUpdateTime;

  if (portamentoActive) {
    // Calculate the progress as a fraction of portamentoTime
    float progress = (float)(millis() - glideStartTime) / (float)portamentoTime;

    // Clamp progress between 0.0 and 1.0
    if (progress > 1.0) progress = 1.0;

    // Apply an **adjusted** exponential scaling for smooth glide
    float rate = 3.5;  // Tweak this value for a more natural feel
    float exponentialProgress = (1.0 - exp(-rate * progress)) / (1.0 - exp(-rate));

    // Interpolate currentMV values for an **exponential glide**
    currentMV_a = initialMV_a + (targetMV_a - initialMV_a) * exponentialProgress;
    currentMV_b = initialMV_b + (targetMV_b - initialMV_b) * exponentialProgress;
    currentMV_c = initialMV_c + (targetMV_c - initialMV_c) * exponentialProgress;

    // If the glide is **complete**, snap to final values and disable glide
    if (progress >= 1.0) {
      currentMV_a = targetMV_a;
      currentMV_b = targetMV_b;
      currentMV_c = targetMV_c;
      portamentoActive = false;
    }
  }

  // Now continuously apply parameters to the frequency calculation

  // Calculate mV for channel_a (Oscillator A)
  oscanote1 = note1 + OCTAVE_A;  // Apply the correct octave adjustment for this oscillator
  additionalmV = (unsigned int)(((float)(OCTAVE_A)*NOTE_SF + 0.5) + (VOLTOFFSET + bend_data + FM_VALUE + FM_AT_VALUE));
  finalmV = (currentMV_a + additionalmV);
  sample_data1 = (channel_a & 0xFFF0000F) | (((int(finalmV)) & 0xFFFF) << 4);
  outputDAC(DAC_NOTE1, sample_data1);

  // Calculate mV for channel_b (Oscillator B)
  oscbnote1 = note1 + OCTAVE_B;  // Apply octave B adjustments
  additionalmV = (unsigned int)(((float)(OCTAVE_B)*NOTE_SF + 0.5) + (VOLTOFFSET + bend_data + FM_VALUE + FM_AT_VALUE + DETUNE));
  finalmV = (currentMV_b + additionalmV);
  sample_data1 = (channel_b & 0xFFF0000F) | (((int(finalmV)) & 0xFFFF) << 4);
  outputDAC(DAC_NOTE1, sample_data1);

  // Calculate mV for channel_c (Oscillator C)
  osccnote1 = note1 + OCTAVE_C;  // Apply octave C adjustments
  additionalmV = (unsigned int)(((float)(OCTAVE_C)*NOTE_SF + 0.5) + (VOLTOFFSET + bend_data + FM_VALUE + FM_AT_VALUE + (DETUNE * 2)));
  finalmV = (currentMV_c + additionalmV);
  sample_data1 = (channel_c & 0xFFF0000F) | (((int(finalmV)) & 0xFFFF) << 4);
  outputDAC(DAC_NOTE1, sample_data1);

  sample_data1 = (channel_e & 0xFFF0000F) | (((int(map(LFORATE, 0, 127, 0, 21940))) & 0xFFFF) << 4);
  outputDAC(DAC_NOTE1, sample_data1);

  sample_data1 = (channel_f & 0xFFF0000F) | (((int(map(LFOWAVE, 0, 7, 0, 21500))) & 0xFFFF) << 4);
  outputDAC(DAC_NOTE1, sample_data1);

  sample_data1 = (channel_g & 0xFFF0000F) | (((int(map(LFOMULT, 0, 4, 0, 12000))) & 0xFFFF) << 4);
  outputDAC(DAC_NOTE1, sample_data1);

  // Update last update time
  lastUpdateTime = currentTime;
}

// Resets all the notes to off
void allNotesOff() {
  digitalWrite(GATE_NOTE1, LOW);

  voices[0].note = -1;

  voiceOn[0] = false;
}

void outputDAC(int CHIP_SELECT, uint32_t sample_data) {
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE1));
  digitalWrite(CHIP_SELECT, LOW);
  delayMicroseconds(1);
  SPI.transfer16(sample_data >> 16);
  SPI.transfer16(sample_data);
  delayMicroseconds(3);  // Settling time delay
  digitalWrite(CHIP_SELECT, HIGH);
  SPI.endTransaction();
}

int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Thomas Kocher
 *
 * DESCRIPTION
 * Fan speed actuator for the elco AIRSYSTEM+ 355.
 * The fan speed ist controlled by closing a contact (relay) for a certain period of time.
 *   
 * Level  Time  Action 
 * -----  ----  -----------------
 *     1  0.5s  level 1
 *     2  1.0s  level 2
 *     3  1.5s  level 3
 *     4  2.0s  auto
 *     5  2.5s  level 3 for 15min
 *     6  3.0s  level 3 for 60min
 * 
 * This sketch will remember relay state after power failure. 
 * The communication of this actuator is RS485 based.
 */
  
// Enable debug prints to serial monitor
#define MY_DEBUG

/**
 * Configure RS485 transport
 */
// Enable RS485 transport layer
#define MY_RS485

// Incontrast to radio based transport, we have to explicitely assigen a MY_NODE_ID whe using RS485 transport.
#define MY_NODE_ID 45

// Define this to enables DE-pin management on defined pin
#define MY_RS485_DE_PIN 2

// Set RS485 baud rate to use
#define MY_RS485_BAUD_RATE 9600


/**
 * Configure actuator
 */
#define ACTUATOR_NAME "AIRSYSTEM+ 355 Actuator"
#define ACTUATOR_VERSION "1.0"

#define RELAY_PIN 12 // Digital I/O pin number for relay
#define RELAY_ON 1   // GPIO value to write to turn on attached relay
#define RELAY_OFF 0  // GPIO value to write to turn off attached relay

#define STATUS_PIN 13 // Digital I/O pin number for status led
#define STATUS_BLINK_INTERVAL 200

#include <MySensors.h>


/**
 * Initialize actuator.
 */
void before() {
  Serial.print("Initializing "); Serial.print(ACTUATOR_NAME); Serial.print(" v"); Serial.print(ACTUATOR_VERSION); Serial.println("...");

  pinMode(STATUS_PIN, OUTPUT);
  digitalWrite(STATUS_PIN, LOW);

  // Then set relay pins in output mode
  pinMode(RELAY_PIN, OUTPUT);
  // Set relay to last known state (using eeprom storage)
  //  digitalWrite(RELAY_PIN, RELAY_OFF);
  setFanLevel(loadState(1));
}


/**
 * Present the actuator to the controller.
 */
void presentation() {
  // Send the sketch version information to the gateway and Controller and enable ACK messages
  sendSketchInfo(ACTUATOR_NAME, ACTUATOR_VERSION, true);

  // Register actuator to gw
  present(1, S_HVAC);
}


/**
 * Execute a command on incoming command message.
 */
void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type == V_HVAC_SPEED) {
    Serial.println("Incoming change for fan...");
    uint8_t level = message.getByte();
    setFanLevel(level);
  }
}


/**
 * Set fan level accordingly ...
 * 
 * Level  Time  Action 
 * -----  ----  -----------------
 *     1  0.5s  level 1
 *     2  1.0s  level 2
 *     3  1.5s  level 3
 *     4  2.0s  auto
 *     5  2.5s  level 3 for 15min
 *     6  3.0s  level 3 for 60min
 */
void setFanLevel(uint8_t level) {
  if (level >= 1 && level <= 6) {
    Serial.print("Setting fan speed level: "); Serial.println(level);

    saveState(1, level);

    digitalWrite(RELAY_PIN, RELAY_ON);
    wait(level * 500);
    digitalWrite(RELAY_PIN, RELAY_OFF);

    displayStatus(level);
  } else {
    Serial.print("ERROR: Invalid fan speed level: "); Serial.println(level);
    displayError();
  }

}


/**
 * Bink status led to display fan level. 
 * 1 = blink once
 * 2 = blink twice
 * etc.
 */
void displayStatus(uint8_t level) {
  digitalWrite(STATUS_PIN, LOW);
  wait(STATUS_BLINK_INTERVAL);
  for (uint8_t i = 0; i < level; i++) {
    digitalWrite(STATUS_PIN, HIGH);
    wait(STATUS_BLINK_INTERVAL);
    digitalWrite(STATUS_PIN, LOW);
    wait(STATUS_BLINK_INTERVAL);
  }
}


/**
 * Light status led on error.
 */
void displayError() {
  digitalWrite(STATUS_PIN, HIGH);
}

void setup() {
}

void loop() {
}


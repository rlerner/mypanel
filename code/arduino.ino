#include <SPI.h>
#include <Ethernet.h>
#define ZONE_DOOR 1
#define ZONE_WINDOW 2
/* (c)2023 Robert Lerner, All Rights Reserved
 *
 * Home Security Monitor
 * github.com/rlerner/mypanel/
 * See Also: robert-lerner.com
*/

// SETUP BEFORE ROLLING OUT
byte mac[] = { 0xA8, 0x61, 0xFF, 0xFF, 0xFF, 0xFF }; // Set MAC from your model (on sticker bottom of v2) or invent for other models
IPAddress RELAY_IP(10,11,12,13); // Set IP Address to send alerts to
String PATH_NAME = "/homesec/?state="; // Set query string path to send data to

// How often should the heartbeat be sent, in cycle count. Default cycle count is roughly one second
#define HEARTBEAT_CYCLES 30

//pin 29 is reserved for the speaker, but is configured by tone.cpp
const int SPKR = 29;
// Zones will start from pin 30 = Zone 1, pin 31 = Zone 2, etc
const int zoneCount = 15;
// Specifies alarm type when opened, you'll probably install the system and find that you're wrong on a few, so make your serial cable/USB accessible
int zoneTypes[] = {
  ZONE_DOOR, ZONE_DOOR, ZONE_DOOR, ZONE_DOOR, ZONE_DOOR, ZONE_DOOR, ZONE_DOOR, ZONE_DOOR, ZONE_DOOR, ZONE_DOOR, ZONE_DOOR, ZONE_DOOR, ZONE_DOOR, ZONE_DOOR, ZONE_DOOR  
};


// End of options

EthernetClient client;
int hbCounter = 0;
// For storing last state of each zone for "rising edge" insecurity detection
int zoneLast[zoneCount];

void setup() {
  // Configure zone pins, grab current state, and set
  for (int pin=30;pin<=30+zoneCount;pin++) {
    pinMode(pin,INPUT_PULLUP);
    zoneLast[pin-30] = digitalRead(pin);
  }
  // Play "Boot Logo"
  alarm(4);

  if (Ethernet.begin(mac) == 0) {
    alarm(5);
    while(true);
  }
}

void loop() {
  String webBody = "";
  String heartbeat = "HB";
  bool alarmSounded = false;

  for (int pin=30;pin<=30+zoneCount;pin++) {
    int curState = digitalRead(pin);
    heartbeat = heartbeat + "Z" + String(curState);

    // Detect pin transitioning from low to high
    if (curState == HIGH && zoneLast[pin-30] == LOW) {
      if (!alarmSounded) { // Several alarms may play back to back, which is ideal for security but sounds like ass. Plus, the chances of two items opening during the same cycle are low. They will all send notifications to the users.
        alarm(zoneTypes[pin-30]);
        alarmSounded = true;
      }
      webBody += "ZONE" + String(pin-29); // -29 to offset the pin, and offset arrays starting at 0
    }
    zoneLast[pin-30] = curState;
  }

  // Iterate for heartbeat and send request. If it fails to send, trigger alarm 5 to signify internet issues. This can get annoying, so think about if you want it.
  hbCounter++;
  if (hbCounter>=HEARTBEAT_CYCLES) {
    hbCounter = 0;
    if (!webRequest(RELAY_IP,PATH_NAME+heartbeat)) {
      alarm(5);
    }
  }

  // For Zone Insecure notices (door open, etc) send off a request.
  if (webBody!="") {
    webRequest(RELAY_IP,PATH_NAME+webBody);
  }
  delay(1000);
}

bool webRequest(IPAddress ip_addr,String query_string) {
  if (client.connect(ip_addr, 80)) { // Sure be great if I didn't have to use plain HTTP, amirite
    client.println("GET " + query_string + " HTTP/1.1"); // Stretch goal, make POST instead
    client.println("Host: " + String(ip_addr[0]) + "." + String(ip_addr[1]) + "." + String(ip_addr[2]) + "." + String(ip_addr[3]) + ":80");
    client.println("Connection: close");
    client.println();
    client.stop();
    return true;
  } else {
    return false;
  }
}

// Various sounds we'll send. Not all are used right now
void alarm(int type) {
  if (type == 1) {
    // Door Alarm
    sqWave(SPKR,4000,400);
    sqWave(SPKR,2000,500);
  } else if (type == 2) {
    // Window Alarm
    sqWave(SPKR,2000,200);
    sqWave(SPKR,2000,100);
    sqWave(SPKR,3000,500);
  } else if (type == 3) {
    // General Panic
    sqWave(SPKR,1000,200);
    sqWave(SPKR,2000,200);
    sqWave(SPKR,3000,200);
    sqWave(SPKR,4000,200);
  } else if (type == 4) {
    // Boot "logo"
    sqWave(SPKR,1000,200);
    sqWave(SPKR,3000,200);
    sqWave(SPKR,2000,200);
    sqWave(SPKR,4000,200);
    sqWave(SPKR,1000,200);
  } else if (type == 5) {
    // Internet Connectivity Issue
    sqWave(SPKR,500,25);
  } else {
    // Unknown Alarm Call
    sqWave(SPKR,2000,200);
    delay(200);
    sqWave(SPKR,2000,200);
    delay(200);
  }
}

// Used the same as tone, but I threw a delay in here as well since it saves typing both for the sounds.
void sqWave(int speaker_pin, int freq, int duration_ms) {
  tone(speaker_pin,freq,duration_ms);
  delay(duration_ms);
}

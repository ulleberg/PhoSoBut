#include "application.h"

byte sonosip[] = { 192, 168, 1, 7 };

int debug = 0;

#define SONOS_PAUSE "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:Pause xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID></u:Pause></s:Body></s:Envelope>"
#define SONOS_PLAY  "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:Play xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID><Speed>1</Speed></u:Play></s:Body></s:Envelope>"
#define SONOS_P3    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:SetAVTransportURI xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID><CurrentURI>x-sonosapi-stream:s26044?sid=254&amp;flags=32</CurrentURI><CurrentURIMetaData>&lt;DIDL-Lite xmlns:dc=&quot;http://purl.org/dc/elements/1.1/&quot; xmlns:upnp=&quot;urn:schemas-upnp-org:metadata-1-0/upnp/&quot; xmlns:r=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot; xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/&quot;&gt;&lt;item id=&quot;R:0/0/3&quot; parentID=&quot;R:0/0&quot; restricted=&quot;true&quot;&gt;&lt;dc:title&gt;NRK P3&lt;/dc:title&gt;&lt;upnp:class&gt;object.item.audioItem.audioBroadcast&lt;/upnp:class&gt;&lt;desc id=&quot;cdudn&quot; nameSpace=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot;&gt;SA_RINCON65031_&lt;/desc&gt;&lt;/item&gt;&lt;/DIDL-Lite&gt;</CurrentURIMetaData></u:SetAVTransportURI></s:Body></s:Envelope>"
#define SONOS_P1    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:SetAVTransportURI xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID><CurrentURI>x-sonosapi-stream:s47450?sid=254&amp;flags=32</CurrentURI><CurrentURIMetaData>&lt;DIDL-Lite xmlns:dc=&quot;http://purl.org/dc/elements/1.1/&quot; xmlns:upnp=&quot;urn:schemas-upnp-org:metadata-1-0/upnp/&quot; xmlns:r=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot; xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/&quot;&gt;&lt;item id=&quot;R:0/0/1&quot; parentID=&quot;R:0/0&quot; restricted=&quot;true&quot;&gt;&lt;dc:title&gt;NRK P1 Trndelag&lt;/dc:title&gt;&lt;upnp:class&gt;object.item.audioItem.audioBroadcast&lt;/upnp:class&gt;&lt;desc id=&quot;cdudn&quot; nameSpace=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot;&gt;SA_RINCON65031_&lt;/desc&gt;&lt;/item&gt;&lt;/DIDL-Lite&gt;</CurrentURIMetaData></u:SetAVTransportURI></s:Body></s:Envelope>"

#define PAUSE 0
#define PLAY 1
#define P3 2
#define P1 3

const int rt_buttonPin = D5;     // the number of the pause/play pushbutton pin
const int pp_buttonPin = D6;     // the number of the pause/play pushbutton pin
const int ledPin =  D7;      // the number of the LED pin

int pp_state = LOW;      // the current state of the play/pause intput pin:
int reading_pp;           // the current reading from the input play/pause pin
int pp_previous = LOW;    // the previous reading from the play/pause input pin:

int rt_state = LOW;      // the current state of the toggle P1/P3 intput pin:
int reading_rt;           // the current reading from the P1/P3 input play/pause pin
int rt_previous = LOW;    // the previous reading from the P1/P3 input pin:


long last = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers

TCPClient client;

void setup()
{
  pinMode(pp_buttonPin, INPUT_PULLUP);
  pinMode(rt_buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  if (debug) {
    Serial.begin(9600);
    while(!Serial.available()); // wait here for user to press ENTER in Serial Terminal
  }
}

void loop()
{

  reading_pp = digitalRead(pp_buttonPin);
  reading_rt = digitalRead(rt_buttonPin);

  // if the input just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin and remember
  // the time
  if (reading_pp == HIGH && pp_previous == LOW && millis() - last > debounce) {
    if (pp_state == HIGH) {
      pp_state = LOW;
      if (debug) {
        Serial.println("play");
      }
      sonos(PLAY);
    }
    else {
      pp_state = HIGH;
      if (debug) {
        Serial.println("pause");
      }
      sonos(PAUSE);
    }

    last = millis();
  }

  digitalWrite(ledPin, pp_state);

  pp_previous = reading_pp;

  if (reading_rt == HIGH && rt_previous == LOW && millis() - last > debounce) {
    if (rt_state == HIGH) {
      rt_state = LOW;
      if (debug) {
        Serial.println("p3");
      }
      sonos(P3);
      sonos(PLAY);
    }
    else {
      rt_state = HIGH;
      if (debug) {
        Serial.println("p1");
      }
      sonos(P1);
      sonos(PLAY);
    }

    last = millis();
    pp_state = LOW;

  }

  rt_previous = reading_rt;
}

void out(const char *s)
{
  client.println(s);
  if (debug) {
    Serial.println(s);
  }
}

void sonos(int cmd)
{
  char buf[2048];

  if (client.connect(sonosip, 1400)) {
    if (debug) {
      Serial.println("connected");
      Serial.println(cmd);
    }

    out("POST /MediaRenderer/AVTransport/Control HTTP/1.1");
    out("Connection: close");
    sprintf(buf, "Host: %d.%d.%d.%d:1400", sonosip[0], sonosip[1], sonosip[2], sonosip[3]);
    out(buf);

    if (cmd == PLAY | cmd == PAUSE){
    sprintf(buf, "Content-Length: %d", (cmd == PLAY) ? strlen(SONOS_PLAY) : strlen(SONOS_PAUSE));
    out(buf);
    }

    if (cmd == P3 | cmd == P1){
    sprintf(buf, "Content-Length: %d", (cmd == P3) ? strlen(SONOS_P3) : strlen(SONOS_P1));
    out(buf);
    }

    out("Content-Type: text/xml; charset=\"utf-8\"");


    if (cmd == PLAY | cmd ==PAUSE){
    sprintf(buf, "Soapaction: \"urn:schemas-upnp-org:service:AVTransport:1#%s\"", (cmd == PLAY) ? "Play" : "Pause");
    out(buf);
    }

    if (cmd == P3 | cmd ==P1){
    sprintf(buf, "Soapaction: \"urn:schemas-upnp-org:service:AVTransport:1#%s\"", (cmd == P3) ? "SetAVTransportURI" : "SetAVTransportURI");
    out(buf);
    }

    if (cmd == PLAY | cmd ==PAUSE){
    out("");
    strcpy(buf, (cmd == PLAY) ? SONOS_PLAY : SONOS_PAUSE);
    out(buf);
    }

    if (cmd == P3 | cmd ==P1){
    out("");
    strcpy(buf, (cmd == P3) ? SONOS_P3 : SONOS_P1);
    out(buf);
    }

    unsigned int count = 0;
    unsigned long lastTime = millis();
    while( client.available()==0 && millis()-lastTime<5000) { //5 second timeout for reply
    }  //do nothing
    lastTime = millis();
    while( client.available() && millis()-lastTime<500 ) {  //500 milliseconds timeout after last data
      char c = client.read();
      if (debug) {
        Serial.print(c);
      }
      lastTime = millis();
      count++;
    }
    client.flush();  //for safety

    //client.flush();
    delay(400);
    if (debug) {
    Serial.println();
    Serial.print("Done, Total bytes returned: ");
    Serial.println(count);
    }

  } else {
    if (debug) {
      Serial.println("connection failed");
    }
  }
  client.stop();
}

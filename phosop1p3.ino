#include "application.h"

byte sonosip[] = { 192, 168, 1, 9 }; //Kitchen
 
int debug = 0;
 
#define SONOS_PLAY  "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:Play xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID><Speed>1</Speed></u:Play></s:Body></s:Envelope>"
#define SONOS_P3    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:SetAVTransportURI xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID><CurrentURI>x-sonosapi-stream:s26044?sid=254&amp;flags=32</CurrentURI><CurrentURIMetaData>&lt;DIDL-Lite xmlns:dc=&quot;http://purl.org/dc/elements/1.1/&quot; xmlns:upnp=&quot;urn:schemas-upnp-org:metadata-1-0/upnp/&quot; xmlns:r=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot; xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/&quot;&gt;&lt;item id=&quot;R:0/0/3&quot; parentID=&quot;R:0/0&quot; restricted=&quot;true&quot;&gt;&lt;dc:title&gt;NRK P3&lt;/dc:title&gt;&lt;upnp:class&gt;object.item.audioItem.audioBroadcast&lt;/upnp:class&gt;&lt;desc id=&quot;cdudn&quot; nameSpace=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot;&gt;SA_RINCON65031_&lt;/desc&gt;&lt;/item&gt;&lt;/DIDL-Lite&gt;</CurrentURIMetaData></u:SetAVTransportURI></s:Body></s:Envelope>"
#define SONOS_P1    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:SetAVTransportURI xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID><CurrentURI>x-sonosapi-stream:s47450?sid=254&amp;flags=32</CurrentURI><CurrentURIMetaData>&lt;DIDL-Lite xmlns:dc=&quot;http://purl.org/dc/elements/1.1/&quot; xmlns:upnp=&quot;urn:schemas-upnp-org:metadata-1-0/upnp/&quot; xmlns:r=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot; xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/&quot;&gt;&lt;item id=&quot;R:0/0/1&quot; parentID=&quot;R:0/0&quot; restricted=&quot;true&quot;&gt;&lt;dc:title&gt;NRK P1 Trndelag&lt;/dc:title&gt;&lt;upnp:class&gt;object.item.audioItem.audioBroadcast&lt;/upnp:class&gt;&lt;desc id=&quot;cdudn&quot; nameSpace=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot;&gt;SA_RINCON65031_&lt;/desc&gt;&lt;/item&gt;&lt;/DIDL-Lite&gt;</CurrentURIMetaData></u:SetAVTransportURI></s:Body></s:Envelope>"


#define PLAY 1
#define P3 2
#define P1 3
 
const int p1_buttonPin = D1;     // the number of the pause/play pushbutton pin
const int p3_buttonPin = D0;     // the number of the pause/play pushbutton pin


int reading_p1;           // the current reading from the input play/pause pin
int reading_p3;           // the current reading from the P1/P3 input play/pause pin

 
TCPClient client;
 
void setup()
{
  pinMode(p1_buttonPin, INPUT_PULLUP);
  pinMode(p3_buttonPin, INPUT_PULLUP);

  if (debug) {
    Serial.begin(9600);
    Serial.println("Ready");
    while(!Serial.available()); // wait here for user to press ENTER in Serial Terminal
    Serial.println("Start");
  }
}
 
void loop()
{
 
  reading_p1 = digitalRead(p1_buttonPin);
  reading_p3 = digitalRead(p3_buttonPin);
 
  // if the input just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin and remember
  // the time
  if (reading_p3 == LOW) {
    if (debug) {
      Serial.println("nrk P3");
    }
      sonos(P3);    
      sonos(PLAY);
      delay(300);
  }

  if (reading_p1 == LOW) {
    if (debug) {
      Serial.println("nrk P1");
    }
    sonos(P1);    
    sonos(PLAY);
    delay(300);
  }
    
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
    
    if (cmd == PLAY) {
    sprintf(buf, "Content-Length: %d", strlen(SONOS_PLAY));
    out(buf);
    }
    
    if (cmd == P3 | cmd == P1){
    sprintf(buf, "Content-Length: %d", (cmd == P3) ? strlen(SONOS_P3) : strlen(SONOS_P1));
    out(buf);
    } 
 
    out("Content-Type: text/xml; charset=\"utf-8\"");
    
  
    if (cmd == PLAY){
    out("Soapaction: \"urn:schemas-upnp-org:service:AVTransport:1#Play\"");
    }
    
    if (cmd == P3 | cmd ==P1){
    out("Soapaction: \"urn:schemas-upnp-org:service:AVTransport:1#SetAVTransportURI\"");
    }
 
    if (cmd == PLAY){
    out("");
    strcpy(buf, SONOS_PLAY);
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

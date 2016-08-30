#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <TimerOne.h>
#include <util/atomic.h>

#define timerLed 10

const int SER     = 7; //datapin
const int RCLK    = 6; //clockPin
const int SRLCLK  = 3; //latchpin

byte mac[]  = {0x5e, 0xa4, 0x18, 0xf0, 0x8a, 0xf6};
IPAddress localIp(192,168,1,67);
IPAddress gateway(192,168,1,254);
IPAddress subnet(255,255,255,0);
IPAddress dnsIp(192,168,1,254);
unsigned int port = 27534;
byte destIp[]  = {51, 254, 123, 208};

char message[]  = "Hello";
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged";
  EthernetUDP Udp;

char response[2];

char programs[] = "dovecot,postfix,postgresql-9.4,httpd,iptables,network,null,null";

void setup() {
  Serial.begin(9600);

  Serial.println("Setup");

  pinMode(SER, OUTPUT);
  pinMode(RCLK, OUTPUT); //latch
  pinMode(SRLCLK, OUTPUT); //clock
  pinMode(timerLed, OUTPUT);

  // initialize Timer1
  Timer1.initialize(10000000);
  Timer1.attachInterrupt(sendRequest);


  Ethernet.begin(mac, localIp, dnsIp, gateway, subnet);
  Udp.begin(port);
}

void sendRequest() {
//  digitalWrite(timerLed, LOW);
  if(Udp.beginPacket(destIp, port) == 0) { Serial.println("Begin Packet fail !"); }
  Udp.write(programs);
  if(Udp.endPacket() == 0) { Serial.println("End Packet fail !"); }
  Serial.println("Request sent");
}




void loop() {
    int packetSize = Udp.parsePacket();
    if (packetSize) {
      Serial.print("Received packet of size ");
      Serial.println(packetSize);
      Serial.print("  From ");
      IPAddress remote = Udp.remoteIP();
      for (int i = 0; i < 4; i++) {
        Serial.print(remote[i], DEC);
        if (i < 3) {
          Serial.print(".");
        }
      }
      Serial.print(", port ");
      Serial.println(Udp.remotePort());

      Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      response[0]  = packetBuffer[0];
      response[1]  = packetBuffer[1];
      updateShiftRegister();
    }
delay(1000);
}



void updateShiftRegister()
{
  digitalWrite(RCLK, LOW);
  shiftOut(SER, SRLCLK, LSBFIRST, response[1]);
  shiftOut(SER, SRLCLK, MSBFIRST, response[0]);
  digitalWrite(RCLK, HIGH);
}

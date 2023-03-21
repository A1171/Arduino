#include <ESP8266WiFi.h>
#include <UniversalTelegramBot.h>
#define WIFI_SSID1 "rtk29"
#define WIFI_PASSWORD1 "3843747171"
#define WIFI_SSID2 "Keenetic-4392"
#define WIFI_PASSWORD2 "Password"
WiFiClientSecure secured_client;
//BotName ESP8266_Remote_01   ESP8266_Remote_01Bot
#define BOT_TOKEN "5244841499:AAGPYkn9Dn6EcnVDjtsM6-8Losmka0PmoYU"
//https://api.telegram.org/bot5244841499:AAGPYkn9Dn6EcnVDjtsM6-8Losmka0PmoYU/getUpdates
#define CHAT_ID "2096068766"
const unsigned long RespondceDelay=1000;
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;
#define RelayOn LOW
#define RelayOff HIGH
#define R0 4
#define R1 5
#define R2 13
#define R3 15
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  configTime(0,0,"pool.ntp.org");
  secured_client.setTrustAnchors(&cert);
  Serial.print("Connecting to WiFi SSID");
  //Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID2,WIFI_PASSWORD2);
  while(WiFi.status() != WL_CONNECTED)
    {
    Serial.print(".");
    delay(300);
    }
  int SelectedWIFI=2;
  /*int SelectedWIFI=-1;
  while(WiFi.status() != WL_CONNECTED)
    {
    Serial.print(".");
    delay(2000);
    if(WiFi.status() != WL_CONNECTED and SelectedWIFI==-1){WiFi.disconnect();WiFi.begin(WIFI_SSID2,WIFI_PASSWORD2);}
    else if(WiFi.status() != WL_CONNECTED)SelectedWIFI=1;
    Serial.print(".");
    delay(2000);
    if(WiFi.status() != WL_CONNECTED and SelectedWIFI==-1){WiFi.disconnect();WiFi.begin(WIFI_SSID1,WIFI_PASSWORD1);}
    else if(WiFi.status() != WL_CONNECTED)SelectedWIFI=2;
    }
  if(SelectedWIFI==1)Serial.print("\nWiFi 1 connected. IP adress: ");
  if(SelectedWIFI==2)Serial.print("\nWiFi 2 connected. IP adress: ");
  Serial.print("\nWiFi 2 connected. IP adress: ");
  Serial.println(WiFi.localIP());*/
  /*
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  */
  pinMode(R0, OUTPUT);
  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(R3, OUTPUT);
  /*
  digitalWrite(D0, RelayOn); // Turn OFF modem
  digitalWrite(D1, RelayOn); // Turn OFF modem
  digitalWrite(D2, RelayOn); // Turn OFF modem
  digitalWrite(D3, RelayOn); // Turn OFF modem
  */
  digitalWrite(R0, RelayOn); // Turn OFF modem
  digitalWrite(R1, RelayOn); // Turn OFF modem
  digitalWrite(R2, RelayOn); // Turn OFF modem
  digitalWrite(R3, RelayOn); // Turn OFF modem
  String Message="";
  
  if(SelectedWIFI==1)Message="Hi! bot online "+String(WIFI_SSID1)+" wifi connected";
  if(SelectedWIFI==2)Message="Hi! bot online "+String(WIFI_SSID2)+" wifi connected";

  int str_len = Message.length() + 1; 
  char Message_char[str_len];
  Message.toCharArray(Message_char, str_len);
  bot.sendMessage(CHAT_ID,Message_char,"");
  }



void loop() {
  // put your main code here, to run repeatedly:
int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages)
    {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    
}


void handleNewMessages(int numNewMessages)
{
  for(int i = 0; i < numNewMessages; i++)
  {
    if(bot.messages[i].chat_id == CHAT_ID)
    {
      String text = bot.messages[i].text;
      Serial.println(text);
      if(text == "/start" || text == "/help")
      {
        String welcome = "Hi i'm ESP8266.\n";
        //welcome += "/temp - get the current temperature value\n";
        welcome += "/relay - current relay state\n";
        welcome += "/relay0on - turn on relay 0\n";
        welcome += "/relay0off - turn off relay 0\n";
        welcome += "same for relay 1-3";
        bot.sendMessage(CHAT_ID, welcome, "");
      }
      else if(text == "/relay")
        {
        
        bot.sendMessage(CHAT_ID, FormRelayStates(), "");
        }
      else if(text == "/relay0on" or text == "/relay0On")
        {
        digitalWrite(R0, RelayOn);
        bot.sendMessage(CHAT_ID, FormRelayStates(), "");
        }
      else if(text == "/relay0off" or text == "/relay0Off")
        {
        digitalWrite(R0, RelayOff);
        bot.sendMessage(CHAT_ID, FormRelayStates(), "");
        }
      else if(text == "/relay1on" or text == "/relay1On")
        {
        digitalWrite(R1, RelayOn);
        bot.sendMessage(CHAT_ID, FormRelayStates(), "");
        }
      else if(text == "/relay1off" or text == "/relay1Off")
        {
        digitalWrite(R1, RelayOff);
        bot.sendMessage(CHAT_ID, FormRelayStates(), "");
        }
      else if(text == "/relay2on" or text == "/relay2On")
        {
        digitalWrite(R2, RelayOn);
        bot.sendMessage(CHAT_ID, FormRelayStates(), "");
        }
      else if(text == "/relay2off" or text == "/relay2Off")
        {
        digitalWrite(R2, RelayOff);
        bot.sendMessage(CHAT_ID, FormRelayStates(), "");
        }
      else if(text == "/relay3on" or text == "/relay3On")
        {
        digitalWrite(R3, RelayOn);
        bot.sendMessage(CHAT_ID, FormRelayStates(), "");
        }
      else if(text == "/relay3off" or text == "/relay3Off")
        {
        digitalWrite(R3, RelayOff);
        bot.sendMessage(CHAT_ID, FormRelayStates(), "");
        }
      
    }
  }
}
String FormRelayStates()
{
String RelayStates="RelayStates\n";
        String StateX="On";
        if(digitalRead(R0)==RelayOff)StateX="Off";
        RelayStates+= "Relay1:"+StateX+"\n";
        StateX="On";
        if(digitalRead(R1)==RelayOff)StateX="Off";
        RelayStates+= "Relay1:"+StateX+"\n";
        StateX="On";
        if(digitalRead(R2)==RelayOff)StateX="Off";
        RelayStates+= "Relay1:"+StateX+"\n";
        StateX="On";
        if(digitalRead(R3)==RelayOff)StateX="Off";
        RelayStates+= "Relay1:"+StateX+"\n";
return RelayStates;
}

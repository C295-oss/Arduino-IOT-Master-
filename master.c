#include <WiFiS3.h>
#include "thingProperties.h"


char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

unsigned int localPort = 8888;
WiFiServer server(localPort);

// Use to keep track of the clients
#define MAX_CLIENTS 20
WiFiClient *clients[MAX_CLIENTS] = {NULL};


void setup() {
  Serial.begin(9600);
  while (!Serial)
  { 
    // Wait for serial port to connect
    ;
  }
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  printWifiStatus();

  // Init cloud variables.
  initProperties();

  // Connect to the Arduino IOT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
}


void printWifiStatus()
{
  // Print the SSID of the network
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print the IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}


void Message(char msg)
{
  for (int i = 0; i < MAX_CLIENTS; i++) 
  {  
    if (clients[i] != NULL && clients[i]->connected())
      clients[i]->print(msg); 
  }
}


void loop() 
{
  ArduinoCloud.update();
  
  WiFiClient newClient = server.available();
  if (newClient)
  {
    Serial.println("New Client...");
    // Add client to array to keep track of each client
    bool clientAdded = false;
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (clients[i] == NULL) {
        clients[i] = new WiFiClient(newClient);
        Serial.print("Client added to slot ");
        Serial.println(i);
        clientAdded = true;
        break;  
      }
    }

    if (!clientAdded) {
      Serial.println("No room for new client");
      newClient.stop();  // Close the connection if no slots available
    }
  }

  // Process existing clients
  for (int i = 0; i < MAX_CLIENTS; i++)
  {
    auto* client = clients[i];
    if (client != NULL)
    {
      if (!client->connected())
      {
        // Client disconnected, remove from list
        Serial.print("Client disconnected from slot ");
        Serial.println(i);
        delete client;  // Free the memory
        clients[i] = NULL;
        continue;
      }

      // Check if there's data available to read
      if (client->available() > 0) 
      {
        
        // char msg = client->read();
        char msg = client->read();

        if (msg != '\0' || msg != '\r' || msg != '\n')
        {
          Serial.print("Client ");
          Serial.print(i);
          Serial.print(" sent: ");
          Serial.println(msg);
          
          // Handle Alert!
          //    Buzzer goes off + LEDs turn red
          if (msg == 'L')
          {
            Message('L');
          }
          if (msg == 'D')
          {
            doorbell_rang = !doorbell_rang;
          }  
        }
        
        client->println("Message received");
      }
    }
  }
}

void onDoorbellRangChange() {
  // do something
}


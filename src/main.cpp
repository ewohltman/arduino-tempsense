#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Ethernet.h>

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDA, 0x02};
IPAddress ip(192,168,1,3);
EthernetServer server(80);

LiquidCrystal lcd(9, 8, 5, 4, 3, 2);

const int pinTempSensor = A0;
const int pinBlueLED = 6;
const int pinRedLED = 7;

float currentTemp = 0.0;

void webServer(float tempF);
float sampleSensorVoltage(int numSamples);

void webServer (float tempF) {
	EthernetClient client = server.available();

	if (client) {
	    boolean currentLineIsBlank = true; // An HTTP request ends with a blank line

	    while (client.connected()) {
	    	if (client.available()) {
	    		char c = client.read();

	            // If you've gotten a newline character and the line is blank,
                // then the HTTP request has completed and we can craft a
                // response
                if (c == '\n' && currentLineIsBlank) {
                    // Send a standard HTTP 200 OK response
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");
                    client.println("Refresh: 15");
                    client.println();
                    client.print("<html><head>");
                    client.print("<title>Arduino Temperature</title>");
                    client.print("<script>");
                    client.print("function refresh() {");
                    client.print("document.getElementById(\"outputText\").innerHTML = \"");
                    client.print("<br><br>");
                    client.print("<u>Current Temperature:</u>");
                    client.print("<br><br>");
                    client.print(tempF);
                    client.print(" F;");
                    client.print("setTimeout('refresh()', 1000);}\"");
                    client.print("</script>");
                    client.print("</head>");
                    client.print("<body onload=\"refresh()\" style=\"background-color:#62AEB2\">");
                    client.print("<p id=\"outputText\" style=\"text-align:center;font-size:72px;font-weight:bold\">");
                    client.print("<br><br>");
                    client.print("<u>Current Temperature:</u>");
                    client.print("<br><br>");
                    client.print(tempF);
                    client.print(" F");
                    client.print("</p>");
                    client.println("</body></html>");

                    break;
                }
            
                // If we're here, the request has not completed yet
                if (c == '\r') {
                    // We just read in probably the beginning of a \r\n end-of-line
                    // Do nothing here since it's not the newline we're looking for
                } else if (c == '\n') {
                    // End of current request line, begin reading next line
                    currentLineIsBlank = true;
                } else {
                    // We just read in a non-newline so there is more to come on this line
                    currentLineIsBlank = false;
                }
	        }
	    }

        // Give the client time to receive the response
        delay(1);

        // Close the connection
        client.stop();
	}
}

float sampleSensorVoltage(int numSamples) {
    int sampleSet = 0;

    for (int i = 0; i < numSamples; i++) {
        sampleSet += analogRead(pinTempSensor);
    }

    int sampleVal = sampleSet / numSamples;
    float tempSensorVoltage = (sampleVal / 1024.0) * 5.0;

    return tempSensorVoltage;
}

void setup() {
    pinMode(pinRedLED, OUTPUT);
    pinMode(pinBlueLED, OUTPUT);

    digitalWrite(pinRedLED, LOW);
    digitalWrite(pinBlueLED, LOW);

    // Serial.begin(9600);

    lcd.begin(16, 2);

    Ethernet.begin(mac, ip);
	server.begin();
}

void loop() {
    float tempSensorVoltage = sampleSensorVoltage(50);

    float tempC = (tempSensorVoltage - 0.5) * 100.0;
    float tempF = ((tempC * 9.0) / 5.0) + 32.0;

    // Serial.print("Temperature in C: ");
    // Serial.print(tempC);
    // Serial.print("\n");

    // Serial.print("Temperature in F: ");
    // Serial.print(tempF);
    // Serial.print("\n\n");

    if (currentTemp != tempF) {
        currentTemp = tempF;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temperature:");
        lcd.setCursor(0, 1);
	    lcd.print(String(currentTemp) + " F");

        if (currentTemp >= 74.0) {
            digitalWrite(pinRedLED, HIGH);
            digitalWrite(pinBlueLED, LOW);
        } else {
            digitalWrite(pinBlueLED, HIGH);
            digitalWrite(pinRedLED, LOW);
        }
    }

    webServer(tempF);

    delay(100);
}

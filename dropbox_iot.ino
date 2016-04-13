#include <Bridge.h>
#include <Console.h>
#include <Temboo.h>

#include "TembooAccount.h" // contains Temboo account information
#include "DropBoxAccount.h" // contains DropbBox account information

#define ITERATION  12 // Iterations of temperature sensor data
#define DEBUG 1

int numRuns = 1;   // Execution count, so this doesn't run forever
int maxRuns = 10;   // Maximum number of times the Choreo should be executed

int sensorPin = 0;
int ledPin = 13;

void setup() {
  Bridge.begin();
  pinMode (ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
#if DEBUG
  // Wait for Console port to connect
  // Example for connecting to Console port 
  // ssh "user name"@"IP ADDRESS" 'telnet localhost 6571'
  // Be careful, till the ssh-telnet connection is done the code is stuck here
  while (!Console);
#endif
}

void loop() {
  Process time;
  
  //Indicate through LED that we have passed the setup() 
  digitalWrite(ledPin, HIGH);

  if (numRuns <= maxRuns) {
    String timeString = "";
    String e_timeString = "";
    
    time.runShellCommand("date");
    
    while(time.available()) {
      char c = time.read();
      timeString += c;
    }
    timeString += sensor_action();
    
    TembooChoreo Base64EncodeChoreo;

    // Invoke the Temboo client
    Base64EncodeChoreo.begin();

    // Set Temboo account credentials
    Base64EncodeChoreo.setAccountName(TEMBOO_ACCOUNT);
    Base64EncodeChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    Base64EncodeChoreo.setAppKey(TEMBOO_APP_KEY);
    
    // Set Choreo inputs
    Base64EncodeChoreo.addInput("Text", timeString);
    
    // Identify the Choreo to run
    Base64EncodeChoreo.setChoreo("/Library/Utilities/Encoding/Base64Encode");
    
    // Run the Choreo; when results are available, print them to serial
    Base64EncodeChoreo.run();

    //Encoding API of TEMBOO returns response with mutiple lines
    //Hack to extract the second line which is encoded string which 
    //is the data to be transferred
    bool first_line = 0;
    while(Base64EncodeChoreo.available()) {
      char c = Base64EncodeChoreo.read();
      if (c != '\n')
        e_timeString += c;
      else {
         if (! first_line) {
            first_line = 1;
            e_timeString = "";
         }
         else break;
      }   
    }

    Base64EncodeChoreo.close();

    TembooChoreo UploadFileChoreo;
    // Invoke the Temboo client
    UploadFileChoreo.begin();

    // Set Temboo account credentials
    UploadFileChoreo.setAccountName(TEMBOO_ACCOUNT);
    UploadFileChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    UploadFileChoreo.setAppKey(TEMBOO_APP_KEY);
    
    // Set Choreo inputs
    UploadFileChoreo.addInput("FileContents",e_timeString);
    UploadFileChoreo.addInput("AccessToken", ACESSTOKEN);
    UploadFileChoreo.addInput("FileName", "iot_sensor_data.txt");
    UploadFileChoreo.addInput("AppKey", APPKEY);
    UploadFileChoreo.addInput("AccessTokenSecret", ACESSTOKENSECRET);
    UploadFileChoreo.addInput("AppSecret", APPSECRET);
    
    // Identify the Choreo to run
    UploadFileChoreo.setChoreo("/Library/Dropbox/FilesAndMetadata/UploadFile");
    
    // Run the Choreo; when results are available, print them to serial
    UploadFileChoreo.run();
    
    while(UploadFileChoreo.available()) {
      char c = UploadFileChoreo.read();
    }
    UploadFileChoreo.close();
  }
  numRuns++;
  delay(30000); // wait 30 seconds between UploadFile calls
}

String sensor_action() {
  int reading;
  float voltage;
  float temperatureC;
  float temperatureF;
  float average_tempF = 0;
  String tempF = "";
  int i;
    
  for (i = 0; i < ITERATION; i++) {
    // ADC calculations
    reading = analogRead(sensorPin);
    voltage = reading * 5.0;
    voltage /= 1024.0;     
    temperatureC = (voltage - 0.5) * 100 ; 
    temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
    average_tempF += temperatureF;
#ifdef DEBUG
    Console.print(temperatureF);
    Console.print("\n");
#endif
    delay(5000); //wait 5 seconds between consecutive sensor read
  }
  average_tempF = average_tempF / ITERATION;
#ifdef DEBUG
  Console.print("Average \t");
  Console.print(average_tempF);
  Console.print("\n");
#endif
  tempF = String (average_tempF);
  return tempF;
}

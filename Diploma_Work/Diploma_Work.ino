#include <DHT.h>

// Soil Moisture Sensors
#define soilSensor1 A0
#define soilSensor2 A1
#define soilSensor3 A2
#define soilSensor4 A3
#define soilSensor5 A4
#define soilSensor6 A5
#define soilSensor7 A6
#define soilSensor8 A7
#define soilSensor9 A8
#define soilSensor10 A9

// Irrigation relay
#define irrigationRelay 2

// Humidifier relay
#define humidifierRelay 3

// Window limit switches and relay
#define windowRelayOpen 4
#define windowRelayClose 5
#define windowLimitSwitchClose 6
#define windowLimitSwitchOpen 7

// DHT11 Sensor
#define TemperatureAndHumiditySensor 8
#define dhtType DHT11
DHT dht(TemperatureAndHumiditySensor, dhtType);

// Fields for air temperature and humidity sensor data calculation and storing
const float ALPHA = 0.3;
float hAvg = 0.0, tAvg = 0.0;

// Constants for calculation of soil moisture sensors reading into percentage
const int DRY_ADC = 820;
const int WET_ADC = 360;

// Global variables containing the values at which the automation process starts / stops the respective systems
float startIrrigationAtSoilMoisture = 35.0;
float stopIrrigationAtSoilMoisture = 60.0;
float openWindowsAtTemperature = 35;
float closeWindowsAtTemperature = 20;
float startHumidifierAtHumidityLevel = 15.0;
float stopHumidifierAtHumidityLevel = 80.0;


// System automation toggle flags
int isIrrigationAutomated = true;
int isHumidifierAutomated = true;
int isTemperatureControlAutomated = true;

// Global variables containing the time in milliseconds when respective systems have been toggled on or off
unsigned long irrigationStartedAt = 0;
unsigned long irrigationStoppedAt = 0;
unsigned long humidifierStartedAt = 0;
unsigned long humidifierStoppedAt = 0;

// Constants containing the minutes for the start / stop of respective systems during the automation process
const int SOAK_TIME = 15;
const int IRRIGATION_TIME = 5;
const int AIR_HUMIDIFYING_TIME = 300;
const int HUMIDIFYING_TIME = 5;

// Fields containing data for incoming messages through the serial port communication with the server
const byte BUFFER_SIZE = 64;
char inputBuffer[BUFFER_SIZE];
byte bufferIndex = 0;

float readHumidity() {
  float h = dht.readHumidity();
  return isnan(h) ? hAvg : h;
}

float readTemperature() {
  float t = dht.readTemperature();
  return isnan(t) ? tAvg : t;
}

void updateAverages(float h, float t) {
  hAvg = (1 - ALPHA) * hAvg + ALPHA * h;
  tAvg = (1 - ALPHA) * tAvg + ALPHA * t;
}

float convertToMoisturePercent(int adcValue)
{
  float moisture = 100.0 * (DRY_ADC - adcValue) / float(DRY_ADC - WET_ADC);

  if (moisture < 0)
    moisture = 0;
  if (moisture > 100)
    moisture = 100;
  return moisture;
}

int *GetSoilMoistureSensorsData()
{
  static int data[10];
  data[0] = analogRead(soilSensor1);
  data[1] = analogRead(soilSensor2);
  data[2] = analogRead(soilSensor3);
  data[3] = analogRead(soilSensor4);
  data[4] = analogRead(soilSensor5);
  data[5] = analogRead(soilSensor6);
  data[6] = analogRead(soilSensor7);
  data[7] = analogRead(soilSensor8);
  data[8] = analogRead(soilSensor9);
  data[9] = analogRead(soilSensor10);
  return data;
}
int *GetSoilMoistureSensorsDataAsPercentage()
{
  static int data[10];
  data[0] = convertToMoisturePercent(analogRead(soilSensor1));
  data[1] = convertToMoisturePercent(analogRead(soilSensor2));
  data[2] = convertToMoisturePercent(analogRead(soilSensor3));
  data[3] = convertToMoisturePercent(analogRead(soilSensor4));
  data[4] = convertToMoisturePercent(analogRead(soilSensor5));
  data[5] = convertToMoisturePercent(analogRead(soilSensor6));
  data[6] = convertToMoisturePercent(analogRead(soilSensor7));
  data[7] = convertToMoisturePercent(analogRead(soilSensor8));
  data[8] = convertToMoisturePercent(analogRead(soilSensor9));
  data[9] = convertToMoisturePercent(analogRead(soilSensor10));
  return data;
}
String GetSoilMoistureSensorsDataAsString()
{
  int *sensorsData = GetSoilMoistureSensorsData();
  String resultString = String(sensorsData[0]);
  for (int i = 1; i < 10; i++)
  {
    resultString = resultString + "," + sensorsData[i];
  }
  return resultString;
}
String GetSoilMoistureSensorsDataPercentageAsString()
{
  int *sensorsData = GetSoilMoistureSensorsDataAsPercentage();
  String resultString = String(sensorsData[0]);
  for (int i = 1; i < 10; i++)
  {
    resultString = resultString + "," + sensorsData[i];
  }
  return resultString;
}
int GetAverageSoilMoistureSensorsData()
{
  int *sensorsData = GetSoilMoistureSensorsData();
  int totalSensorsMeasurement = 0;
  for (int i = 0; i < 10; i++)
  {
    totalSensorsMeasurement += sensorsData[i];
  }
  int averageSensorData = totalSensorsMeasurement / 10;
  return averageSensorData;
}
int GetAverageSoilMoistureSensorsDataAsPercentage()
{
  int *sensorsData = GetSoilMoistureSensorsDataAsPercentage();
  int totalSensorsMeasurement = 0;
  for (int i = 0; i < 10; i++)
  {
    totalSensorsMeasurement += sensorsData[i];
  }
  int averageSensorData = totalSensorsMeasurement / 10;
  return averageSensorData;
}
void ToggleIrrigationAutomation()
{
  isIrrigationAutomated = !isIrrigationAutomated;
}
void ToggleHumidifierAutomation()
{
  isHumidifierAutomated = !isHumidifierAutomated;
}
void ToggleTemperatureControlAutomation()
{
  isTemperatureControlAutomated = !isTemperatureControlAutomated;
}
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  if (found > index)
  {
    return data.substring(strIndex[0], strIndex[1]);
  }
  else
  {
    return "";
  }
}
int splitString(String data, char delimiter, String parts[], int maxParts)
{
  int numParts = 0;
  int startIndex = 0;
  int delimIndex = data.indexOf(delimiter);

  while (delimIndex >= 0 && numParts < maxParts)
  {
    parts[numParts++] = data.substring(startIndex, delimIndex);
    startIndex = delimIndex + 1;
    delimIndex = data.indexOf(delimiter, startIndex);
  }

  // Add last part
  if (numParts < maxParts)
  {
    parts[numParts++] = data.substring(startIndex);
  }

  return numParts;
}
int CalculateMinutesInTimeDifference(unsigned long timeNow, unsigned long control)
{
  return (control - timeNow) / 60000;
}
void OpenWindow()
{
  digitalWrite(windowRelayOpen, HIGH);
  digitalWrite(windowRelayClose, LOW);
  while (digitalRead(windowLimitSwitchOpen) == LOW)
  {
    delay(10);
  }
  StopMotor();
}
void CloseWindow()
{
  digitalWrite(windowRelayClose, HIGH);
  digitalWrite(windowRelayOpen, LOW);
  while (digitalRead(windowLimitSwitchClose) == LOW)
  {
    delay(10);
  }
  StopMotor();
}
void StopMotor()
{
  digitalWrite(windowRelayOpen, LOW);
  digitalWrite(windowRelayClose, LOW);
}

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);

  pinMode(irrigationRelay, OUTPUT);
  pinMode(humidifierRelay, OUTPUT);
  pinMode(windowRelayOpen, OUTPUT);
  pinMode(windowRelayClose, OUTPUT);
  pinMode(windowLimitSwitchClose, INPUT);
  pinMode(windowLimitSwitchOpen, INPUT);

  pinMode(TemperatureAndHumiditySensor, INPUT);

  // Configuring soil moisture level sensor pins as input
  pinMode(soilSensor1, INPUT);
  pinMode(soilSensor2, INPUT);
  pinMode(soilSensor3, INPUT);
  pinMode(soilSensor4, INPUT);
  pinMode(soilSensor5, INPUT);
  pinMode(soilSensor6, INPUT);
  pinMode(soilSensor7, INPUT);
  pinMode(soilSensor8, INPUT);
  pinMode(soilSensor9, INPUT);
  pinMode(soilSensor10, INPUT);

  digitalWrite(irrigationRelay, 0);
}

void handleCommand(char *command)
{
  String commandParts[2];
  splitString(String(command), '|', commandParts, 2);
  String cmd = commandParts[0];
  String commandId = commandParts[1];

  if (cmd.length() > 0)
  {
    // Logic to check if command has parameter
    String commandBody[2];
    splitString(cmd, ':', commandBody, 2);
    String parameter = commandBody[1];
        // Serial.println(commandBody);
    if(parameter.length() == 0)
    {
      if (cmd == "windowStatus")
      {
        int status = digitalRead(windowLimitSwitchOpen);
        bool result = status == 0 ? true : false;
        Serial.println(String(result) + "|" + commandId);
      }
      else if (cmd == "openWindows")
      {
        OpenWindow();
        Serial.println("Window(s) opened.|" + commandId);
      }
      else if (cmd == "closeWindows")
      {
        CloseWindow();
        Serial.println("Window(s) closed.|" + commandId);
      }
      else if (cmd == "irrigationStatus")
      {
        Serial.println(String(digitalRead(irrigationRelay)) + "|" + commandId);
      }
      else if (cmd == "humidifierStatus")
      {
        Serial.println(String(digitalRead(humidifierRelay)) + "|" + commandId);
      }
      else if (cmd == "startIrrigation")
      {
        digitalWrite(irrigationRelay, 1);
        Serial.println("Irrigation started.|" + commandId);
      }
      else if (cmd == "stopIrrigation")
      {
        digitalWrite(irrigationRelay, 0);
        Serial.println("Irrigation stopped.|" + commandId);
      }
      else if (cmd == "startHumidifier")
      {
        digitalWrite(humidifierRelay, 1);
        Serial.println("Humidifier started.|" + commandId);
      }
      else if (cmd == "stopHumidifier")
      {
        digitalWrite(humidifierRelay, 0);
        Serial.println("Humidifier stopped.|" + commandId);
      }
      else if (cmd == "soilMoistureSensors")
      {
        int sensor0 = analogRead(soilSensor1);
        int sensor1 = analogRead(soilSensor2);
        int sensor2 = analogRead(soilSensor3);
        int sensor3 = analogRead(soilSensor4);
        int sensor4 = analogRead(soilSensor5);
        int sensor5 = analogRead(soilSensor6);
        int sensor6 = analogRead(soilSensor7);
        int sensor7 = analogRead(soilSensor8);
        int sensor8 = analogRead(soilSensor9);
        int sensor9 = analogRead(soilSensor10);
        String result = String(sensor0) + "," + sensor1 + "," + sensor2 + "," + sensor3 + "," + sensor4 + "," + sensor5 + "," + sensor6 + "," + sensor7 + "," + sensor8 + "," + sensor9;
        Serial.println(result + "|" + commandId);
      }
      else if (cmd == "dht11Sensor") 
      {
        Serial.println(String(tAvg) + "," + String(hAvg) + "|" + commandId);
      }
      else if (cmd == "airHumidity")
      {
        Serial.println(String(hAvg) + "|" + commandId);
      }
      else if (cmd == "airTemperature")
      {
        Serial.println(String(tAvg) + "|" + commandId);
      }
      else if (cmd == "airHumidityAutomationStatus")
      {
        Serial.println(String(isHumidifierAutomated) + "|" + commandId);
      }
      else if (cmd == "airTemperatureAutomationStatus")
      {
        Serial.println(String(isTemperatureControlAutomated) + "|" + commandId);
      }
      else if (cmd == "irrigationAutomationStatus")
      {
        Serial.println(String(isIrrigationAutomated) + "|" + commandId);
      }
      else if (cmd == "toggleAirHumidityAutomation")
      {
        isHumidifierAutomated = !isHumidifierAutomated;
        Serial.println(String(isHumidifierAutomated) + "|" + commandId);
      }
      else if (cmd == "toggleAirTemperatureAutomation")
      {
        isTemperatureControlAutomated = !isTemperatureControlAutomated;
        Serial.println(String(isTemperatureControlAutomated) + "|" + commandId);
      }
      else if (cmd == "toggleIrrigationAutomation")
      {
        isIrrigationAutomated = !isIrrigationAutomated;
        Serial.println(String(isIrrigationAutomated) + "|" + commandId);
      }
      else if (cmd == "irrigationAutomationConfiguration")
      {
        String minSoilMoisture = String(startIrrigationAtSoilMoisture);
        String maxSoilMoisture = String(stopIrrigationAtSoilMoisture);
        Serial.println(minSoilMoisture + "," + maxSoilMoisture + "|" + commandId);
      }
      else if (cmd == "temperatureAndHumidityAutomationConfiguration")
      {
        String minAirTemp = String(openWindowsAtTemperature);
        String maxAirTemp = String(closeWindowsAtTemperature);
        String minAirHumidity = String(startHumidifierAtHumidityLevel);
        String maxAirHumidity = String(stopHumidifierAtHumidityLevel);
        Serial.println(minAirTemp + "," + maxAirTemp + minAirHumidity + "," + maxAirHumidity + "|" + commandId);
      }
    }
    else
    {
      cmd = commandBody[0];
      if(cmd == "setStartIrrigationAt")
      {
        startIrrigationAtSoilMoisture = parameter.toFloat();
        Serial.println("Irrigation start at parameter set|" + commandId);
      }
      else if (cmd == "setStopirrigationAt")
      {
        stopIrrigationAtSoilMoisture = parameter.toFloat();
        Serial.println("Irrigation stop at parameter set|" + commandId);
      }
      else if (cmd == "setStartHumidifierAt")
      {
        startHumidifierAtHumidityLevel = parameter.toFloat();
        Serial.println("Humidifier start at parameter set|" + commandId);
      }
      else if (cmd == "setStopHumidifierAt")
      {
        stopHumidifierAtHumidityLevel = parameter.toFloat();
        Serial.println("Humidifier stop at parameter set|" + commandId);
      }
      else if (cmd == "setOpenWindowsAt")
      {
        openWindowsAtTemperature = parameter.toFloat();
        Serial.println("Windows open at parameter set|" + commandId);
      }
      else if (cmd == "setCloseWindowsAt")
      {
        closeWindowsAtTemperature = parameter.toFloat();
        Serial.println("Windows close at parameter set|" + commandId);
      }
    }
  }
}

void loop()
{
  float h = readHumidity();
  float t = readTemperature();
  updateAverages(h, t);

  if(isIrrigationAutomated)
  {
    unsigned long timeNowSoakedCheck = millis();
    unsigned long timeDifferenceForSoak = CalculateMinutesInTimeDifference(timeNowSoakedCheck, irrigationStoppedAt);
    if(timeDifferenceForSoak >= SOAK_TIME)
    {
      if(digitalRead(irrigationRelay) == LOW) 
      {
        int moistureCheckBeforeStart = GetAverageSoilMoistureSensorsDataAsPercentage();
        if(moistureCheckBeforeStart <= startIrrigationAtSoilMoisture)
        {
          digitalWrite(irrigationRelay, HIGH);
          irrigationStartedAt = millis();
        }
      }
      else
      {
        unsigned long timeNowForIrrigationStopCheck = millis();
        unsigned long timeDifferenceForStop = CalculateMinutesInTimeDifference(timeNowForIrrigationStopCheck, irrigationStartedAt);
        if(timeDifferenceForStop >= IRRIGATION_TIME)
        {
          int moistureCheckBeforeStop = GetAverageSoilMoistureSensorsDataAsPercentage();
          if(moistureCheckBeforeStop >= stopIrrigationAtSoilMoisture)
          {
            digitalWrite(irrigationRelay, LOW);
            irrigationStoppedAt = millis();
          }
        }
      }
    }
  }

  if(isHumidifierAutomated)
  {
    unsigned long timeNowHumidifiedCheck = millis();
    unsigned long timeDifferenceForHumidifying = CalculateMinutesInTimeDifference(timeNowHumidifiedCheck, humidifierStoppedAt);
    if(timeDifferenceForHumidifying >= AIR_HUMIDIFYING_TIME)
    {
      if(digitalRead(humidifierRelay) == LOW) 
      {
        int humidityCheckBeforeStart = readHumidity();
        if(humidityCheckBeforeStart <= startHumidifierAtHumidityLevel)
        {
          digitalWrite(humidifierRelay, HIGH);
          humidifierStartedAt = millis();
        }
      }
      else
      {
        unsigned long timeNowForHumidifierStopCheck = millis();
        unsigned long timeDifferenceForStop = CalculateMinutesInTimeDifference(timeNowForHumidifierStopCheck, humidifierStartedAt);
        if(timeDifferenceForStop >= HUMIDIFYING_TIME)
        {
          int moistureCheckBeforeStop = readHumidity();
          if(moistureCheckBeforeStop >= stopHumidifierAtHumidityLevel)
          {
            digitalWrite(humidifierRelay, LOW);
            humidifierStoppedAt = millis();
          }
        }
      }
    }
  }



  while (Serial.available() > 0)
  {
    char incoming = Serial.read();

    if (incoming == '\n' || incoming == '\r')
    {
      if (bufferIndex > 0)
      {
        inputBuffer[bufferIndex] = '\0';
        handleCommand(inputBuffer);
        bufferIndex = 0;
      }
    }
    else
    {
      if (bufferIndex < BUFFER_SIZE - 1)
      {
        inputBuffer[bufferIndex++] = incoming;
      }
    }
  }
}
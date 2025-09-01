// #include <array>

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

const int SAMPLE_CNT = 8; // moving average samples

const float OPEN_WINDOW_AT_TEMPERATURE = 30;
const float CLOSE_WINDOW_AT_TEMPERATURE = 20;

const int DRY_ADC = 820; // Dry soil/air reading
const int WET_ADC = 360; // Fully saturated soil/water reading
const float START_IRRIGATION_AT_SOIL_MOISTURE = 35.0;
const float STOP_IRRIGATION_AT_SOIL_MOISTURE = 60.0;

// System automation toggle flags
int isIrrigationAutomated = true;
int isHumidifierAutomated = true;
int isTemperatureControlAutomated = true;

int irrigationStoppedAt = 0;
int humidifierStoppedAt = 0;

float samples[SAMPLE_CNT];
int sampleIndex = 0;

float convertToMoisturePercent(int adcValue)
{
  float moisture = 100.0 * (DRY_ADC - adcValue) / float(DRY_ADC - WET_ADC);

  if (moisture < 0)
    moisture = 0;
  if (moisture > 100)
    moisture = 100;
  return moisture;
}

float readSmoothedPercent(int pin)
{
  samples[sampleIndex] = convertToMoisturePercent(analogRead(pin));
  sampleIndex = (sampleIndex + 1) % SAMPLE_CNT;
  float sum = 0.0;
  for (int i = 0; i < SAMPLE_CNT; i++)
    sum += samples[i];
  return sum / SAMPLE_CNT;
}

int *GetSoilMoistureSensorsData()
{
  int data[10];
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
  int data[10];
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

void OpenWindow()
{
  digitalWrite(windowRelayOpen, 1);
  while (true)
  {
    int limitSwitchTriggered = digitalRead(windowLimitSwitchOpen);
    if (limitSwitchTriggered == 1)
    {
      break;
    }
  }
  digitalWrite(windowRelayOpen, 0);
}

void CloseWindow()
{
  digitalWrite(windowRelayClose, 1);
  while (true)
  {
    int limitSwitchTriggered = digitalRead(windowLimitSwitchClose);
    if (limitSwitchTriggered == 1)
    {
      break;
    }
  }
  digitalWrite(windowRelayClose, 0);
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

void setup()
{
  Serial.begin(9600);

  pinMode(irrigationRelay, OUTPUT);
  pinMode(humidifierRelay, OUTPUT);
  pinMode(windowRelayOpen, OUTPUT);
  pinMode(windowRelayClose, OUTPUT);
  pinMode(windowLimitSwitchClose, INPUT);
  pinMode(windowLimitSwitchOpen, INPUT);

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

const byte BUFFER_SIZE = 64;
char inputBuffer[BUFFER_SIZE];
byte bufferIndex = 0;

void handleCommand(char *command)
{
  String commandParts[2];
  splitString(String(*command), ',', commandParts, 2);
  String msg = commandParts[1];

  if (msg.length() > 0)
  {

    if (msg == "windowStatus\r")
    {
      int status = digitalRead(windowLimitSwitchOpen);
      bool result = status == 0 ? true : false;
      Serial.println(String(result) + "|" + commandParts[0]);
    }
    else if (msg == "irrigationStatus\r")
    {
      Serial.println(String(digitalRead(irrigationRelay)) + "|" + commandParts[0]);
    }
    else if (msg == "humidifierStatus\r")
    {
      Serial.println(String(digitalRead(humidifierRelay)) + "|" + commandParts[0]);
    }
    else if (msg == "openWindow\r")
    {
      // digitalWrite(windowRelay, 1);
      OpenWindow();
      Serial.println("Window(s) opened.|" + commandParts[0]);
    }
    else if (msg == "closeWindow\r")
    {
      // digitalWrite(windowRelay, 0);
      CloseWindow();
      Serial.println("Window(s) closed.|" + commandParts[0]);
    }
    else if (msg == "startIrrigation\r")
    {
      digitalWrite(irrigationRelay, 1);
      Serial.println("Irrigation started.|" + commandParts[0]);
    }
    else if (msg == "stopIrrigation\r")
    {
      digitalWrite(irrigationRelay, 0);
      Serial.println("Irrigation stopped.|" + commandParts[0]);
    }
    else if (msg == "startHumidifier\r")
    {
      digitalWrite(humidifierRelay, 1);
      Serial.println("Humidifier started.|" + commandParts[0]);
    }
    else if (msg == "stopHumidifier\r")
    {
      digitalWrite(humidifierRelay, 0);
      Serial.println("Humidifier stopped.|" + commandParts[0]);
    }
    else if (msg == "soilMoistureSensors\r")
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
      Serial.println(result + "|" + commandParts[0]);
    }
  }
}

void loop()
{

  // ADD AUTOMATION LOGIC BELOW:
  // =========================================
  // ....
  // =========================================

  while (Serial.available() > 0)
  {
    char incoming = Serial.read();

    if (incoming == '\n' || incoming == '\r')
    {
      if (bufferIndex > 0)
      {
        inputBuffer[bufferIndex] = '\0'; // null-terminate string
        handleCommand(inputBuffer);      // process command
        bufferIndex = 0;                 // reset buffer
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

#include<SPI.h>
#include <AD7193.h>
void setup() {
 Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  delay(1000);
  AD7193.begin();
  AD7193.AppendStatusValuetoData();  
  AD7193.SetPGAGain(8);
  AD7193.SetAveraging(100);
  AD7193.Calibrate();
  AD7193.ReadRegisterMap();
  Serial.println("\nBegin AD7193 conversion - single conversion (pg 35 of datasheet, figure 25)");
}

void loop() {
  unsigned long ch0Data;
  unsigned long chTempData;
  
  ch0Data = (AD7193.ReadADCChannel(0) >> 8);     // last 8 bits of ch0Data are status bits, and not data bits
  Serial.print("  CH0 data: ");
  Serial.print(ch0Data, HEX);
  chTempData = (AD7193.ReadADCChannel(8) >> 8);   // last 8 bits of ch0Data are status bits, and not data bits
  Serial.print("  CH8 data (temperature): ");
  Serial.println(chTempData, HEX);         
  // Convert AD7193 temperature data to temperature (degC), 
  // and calculate equivalent voltage - used for cold junction compensation
  float ambientTemp = AD7193.TempSensorDataToDegC(chTempData);
  float referenceVoltage = Thermocouple_Ktype_TempToVoltageDegC(ambientTemp);
  // measure thermocouple voltage
  float thermocoupleVoltage = AD7193.DataToVoltage(ch0Data);
  // Cold Junction Compensation
  float compensatedVoltage = thermocoupleVoltage + referenceVoltage;
  float compensatedTemperature = Thermocouple_Ktype_VoltageToTempDegC(compensatedVoltage);
  // Display Summary
  Serial.print("\n\t\tChannel 0 Compensated Thermocouple Voltage Measurement: ");
  Serial.print(compensatedTemperature, 3);  Serial.println(" degC");
  Serial.print("\t\t--------------------------------------------------------");
  Serial.println("\n\t\tThermocouple Measurement Details:");
  Serial.print("\t\t\tThermocouple Voltage: ");  Serial.println(thermocoupleVoltage, 7);
  Serial.print("\t\t\tReference Temp: ");  Serial.println(ambientTemp, 5);
  Serial.print("\t\t\tReference Voltage: ");  Serial.println(referenceVoltage, 5);
  Serial.print("\t\t\tReference Temp Voltage Equivalent: ");  Serial.println(referenceVoltage, 5);
  Serial.println("");

  delay(100);
  
}

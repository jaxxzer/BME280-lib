#include <Wire.h>

#define ADDRESS 0x76
#define COMPENSATION 0x88
#define TEMP_MSB 0xFA
#define TEMP_LSB 0xFB
#define TEMP_XLSB 0xFC
#define CTRL_MEAS 0xF4

void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
Wire.begin(ADDRESS);
}

	uint16_t T1;
	int16_t T2;
	int16_t T3;

int32_t adc_temperature;
int32_t temperature;

void loop() {
	set_normal_mode();
	read_calibration();
  // put your main code here, to run repeatedly:

	adc_temperature = read_temp_adc();
	Serial.print("\n\nadc_t: ");
	Serial.print(adc_temperature);
	Serial.println("\n\n");

	temperature = calculate_temp(adc_temperature);
	
	Serial.print("\n\nTEMP: ");
	Serial.print(temperature);
	Serial.println("\n\n");
	
	delay(1000);

}

int32_t calculate_temp(int32_t adc_T) {
	int32_t var1, var2, t_fine, T;
	var1 = ((((adc_T>>3) - ((int32_t)T1<<1))) * ((int32_t)T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)T1))*((adc_T>>4) - ((int32_t)T1))) >> 12)*((int32_t)T3)) >> 14;
	t_fine = var1 + var2;
	T  = (t_fine * 5 + 128) >> 8;
	return T;
}

void read_calibration() {
	Wire.beginTransmission(ADDRESS);
	Wire.write(COMPENSATION);
	Wire.endTransmission();
	Wire.requestFrom(ADDRESS, 10);
	
	uint8_t buf[6];
	delay(10);
			int i = 0;
	while(Wire.available()) {
		buf[i++] = Wire.read();
	}
	T1 = buf[0] + (buf[1]<<8);
	T2 = buf[2] + (buf[3]<<8);
	T3 = buf[4] + (buf[5]<<8);
//	Serial.print("T1: ");
//	Serial.print(T1);
//	Serial.print("\tT2: ");
//	Serial.print(T2);
//	Serial.print("\tT3: ");
//	Serial.println(T3);
}

int32_t read_temp_adc() {
	Wire.beginTransmission(ADDRESS);
	Wire.write(TEMP_MSB);
	Wire.endTransmission();
	Wire.requestFrom(ADDRESS, 3);
	uint8_t buf[3];
	delay(10);
			int i = 0;
	while(Wire.available()) {

		buf[i++] = Wire.read();
//		Serial.print("adc buf ");
//		Serial.print(i-1);
//		Serial.print(": ");
//		Serial.println((uint8_t)buf[i-1]);
	}

	uint8_t msb, lsb, xlsb;
	uint32_t adc_t = 0;
	adc_t = (buf[0] << 16) + (buf[1] << 8) + buf[2];
	adc_t = adc_t >> 4;
	return adc_t;
}

void set_normal_mode() {
		Wire.beginTransmission(ADDRESS);
	Wire.write(CTRL_MEAS);
	uint8_t mode = 0xFF;
	Wire.write(mode);
	Wire.endTransmission();
}


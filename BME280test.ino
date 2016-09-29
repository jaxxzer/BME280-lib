#include <Wire.h>

// Datasheet p31
// i2c slave address tie SDO to GND for 0x76, to VDDIO for 0x77
#define BME280_ADDRESS 0x76

// Datasheet p22
#define BME280_ADD_COMPENSATION1 0x88
#define BME280_COMP1_LENGTH 25 // 0x88...0xA1
#define BME280_ADD_COMPENSATION2 0xE1
#define BME280_COMP2_LENGTH 7 // 0xE1...0xE7


// Datasheet p28,29
#define BME280_ADD_PRESS 0xF7
#define BME280_ADD_TEMP 0xFA
#define BME280_ADD_HUM 0xFD


#define BME280_ADD_CTRL_MEAS 0xF4


// Datasheet p26
#define BME280_ADD_CTRL_HUM 0xF2


// Datasheet p27
#define BME280_OVERSAMPLE_SKIP 0
#define BME280_OVERSAMPLE_1	1
#define BME280_OVERSAMPLE_2 2
#define BME280_OVERSAMPLE_4 3
#define BME280_OVERSAMPLE_8 4
#define BME280_OVERSAMPLE_16 5

#define BME280_MODE_SLEEP 0
#define BME280_MODE_FORCE 1
#define BME280_MODE_NORMAL 3


void setup() {
	Serial.begin(115200);
	Wire.begin(BME280_ADDRESS);
	set_hum_mode(BME280_OVERSAMPLE_16);
	set_mode(BME280_OVERSAMPLE_1, BME280_OVERSAMPLE_1, BME280_MODE_NORMAL);
}

uint16_t dig_T1;
int16_t dig_T2;
int16_t dig_T3;

int16_t dig_P1;
int16_t dig_P2;
int16_t dig_P3;
int16_t dig_P4;
int16_t dig_P5;
int16_t dig_P6;
int16_t dig_P7;
int16_t dig_P8;
int16_t dig_P9;

int16_t dig_H1;
int16_t dig_H2;
int16_t dig_H3;
int16_t dig_H4;
int16_t dig_H5;
int16_t dig_H6;


int32_t adc_temperature;
int32_t temperature;
int32_t adc_humidity;
int32_t humidity;

void loop() {

	read_calibration();

	adc_temperature = read_temp_adc();
	Serial.print("\n\nadc_t: ");
	Serial.print(adc_temperature);
	Serial.println("\n\n");

	temperature = calculate_temp(adc_temperature);
	
	Serial.print("\n\nTEMP: ");
	Serial.print(temperature);
	Serial.println("\n\n");

	adc_humidity = read_hum_adc();
	Serial.print("\n\nadc_h: ");
	Serial.print(adc_humidity);
	Serial.println("\n\n");
	humidity = calculate_humidity(adc_humidity);
	Serial.print("\n\nhumidity: ");
	Serial.print((float)adc_humidity/1024.0f);
	Serial.println("\n\n");
	
	delay(1000);

}


// Datasheet p23
int32_t t_fine;
int32_t calculate_temp(int32_t adc_T) {
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)dig_T1))*((adc_T>>4) - ((int32_t)dig_T1))) >> 12)*((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T  = (t_fine * 5 + 128) >> 8;
	return T;
}

// Datasheet p23
int32_t calculate_humidity(int32_t adc_H) {
	int32_t v_x1_u32r;
	v_x1_u32r = (t_fine - ((int32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) +
		((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) * (((v_x1_u32r *
		((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
		((int32_t)dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (uint32_t)(v_x1_u32r>>12);
		
	return 0;
}

// Datasheet p22
void read_calibration() {
	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(BME280_ADD_COMPENSATION1);
	Wire.endTransmission();
	Wire.requestFrom(BME280_ADDRESS, BME280_COMP1_LENGTH);
	
	uint8_t buf[BME280_COMP1_LENGTH + BME280_COMP2_LENGTH];
	delay(10);
	int i = 0;
	while(Wire.available()) {
		buf[i++] = Wire.read();
	}

	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(BME280_ADD_COMPENSATION2);
	delay(10);
	Wire.endTransmission();
	Wire.requestFrom(BME280_ADDRESS, BME280_COMP2_LENGTH);
	while(Wire.available()) {
		buf[i++] = Wire.read();
	}

	i = 0;

	// 0x88
	dig_T1 = buf[i++] + (buf[i++]<<8); // unsigned short
	dig_T2 = buf[i++] + (buf[i++]<<8); // signed short
	dig_T3 = buf[i++] + (buf[i++]<<8); // signed short

	// 0x8E
	dig_P1 = buf[i++] + (buf[i++]<<8); // unsigned short
	dig_P2 = buf[i++] + (buf[i++]<<8); // signed short
	dig_P3 = buf[i++] + (buf[i++]<<8); // signed short
	dig_P4 = buf[i++] + (buf[i++]<<8); // signed short
	dig_P5 = buf[i++] + (buf[i++]<<8); // signed short
	dig_P6 = buf[i++] + (buf[i++]<<8); // signed short
	dig_P7 = buf[i++] + (buf[i++]<<8); // signed short
	dig_P8 = buf[i++] + (buf[i++]<<8); // signed short
	dig_P9 = buf[i++] + (buf[i++]<<8); // signed short

	// 0xA1
	dig_H1 = buf[i++]; 									// unsigned char
	dig_H2 = buf[i++] + (buf[i++]<<8); // signed short
	dig_H3 = buf[i++];									// unsigned char

	// wtf?! notice we dont increment i as we will need same byte in next step
	dig_H4 = (buf[i++]<<5) | (buf[i] & 0b00001111);  // signed short
	dig_H5 = (buf[i++] & 0b00001111) + (buf[i++]<<4);
	dig_H6 = buf[i]; // signed char
	
	Serial.println(i); // make sure we kept proper count
//	Serial.print("dig_T1: ");
//	Serial.print(dig_T1);
//	Serial.print("\tdig_T2: ");
//	Serial.print(dig_T2);
//	Serial.print("\tdig_T3: ");
//	Serial.println(dig_T3);
}

int32_t read_temp_adc() {
	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(BME280_ADD_TEMP);
	Wire.endTransmission();
	Wire.requestFrom(BME280_ADDRESS, 3);
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
	uint32_t adc_t = 0;
	// msb, lsb, xlsb
	adc_t = (buf[0] << 16) + (buf[1] << 8) + buf[2];
	adc_t = adc_t >> 4;
	return adc_t;
}

int32_t read_hum_adc() {
	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(BME280_ADD_HUM);
	Wire.endTransmission();
	Wire.requestFrom(BME280_ADDRESS, 2);
	uint8_t buf[2];
	delay(10);
	int i = 0;
	while(Wire.available()) {
		buf[i++] = Wire.read();
	}
	uint32_t adc_h = 0;
	adc_h = (buf[0] << 8) + buf[1];
	return adc_h;
}

// Datasheet p27
void set_mode(uint8_t osrs_t, uint8_t osrs_p, uint8_t mode) {

	uint8_t ctrl_meas = (osrs_t<<5) | (osrs_p<<2) | mode;
	
	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(BME280_ADD_CTRL_MEAS);
	Wire.write(ctrl_meas);
	Wire.endTransmission();
}

// per Datasheet p25 changes to this register only become active after
// write to ctrl_meas register !!
void set_hum_mode(uint8_t osrs_h) {
	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(BME280_ADD_CTRL_HUM);
	Wire.write(osrs_h);
	Wire.endTransmission();
}


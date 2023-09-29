/*
 * Copyright (c) 2011, Shantanu Gupta <shans95g@gmail.com>
 */

#include <Chipset/msm_i2c.h>
#include <Library/DS2746.h>


UINT32 ds2746_voltage(UINT8 addr) {
	UINT32 vol;
	UINT8 s0, s1;

	msm_i2c_read(addr, DS2746_VOLTAGE_MSB, &s0, 1);
	msm_i2c_read(addr, DS2746_VOLTAGE_LSB, &s1, 1);

	vol = s0 << 8;
	vol |= s1;

	return ((vol >> 4) * DS2746_VOLTAGE_RES) / 1000;
}

/*
  //min,	max
	7150,	15500,	// Sony 1300mAh (Formosa)	7.1k ~ 15k
	27500,	49500,	// Sony 1300mAh (HTE)		28k  ~ 49.5k
	15500,	27500,	// Sanyo 1300mAh (HTE)		16k  ~ 27k
	100,	7150,	// Samsung 1230mAh			0.1k ~ 7k
	0,		100,	// HTC Extended 2300mAh		0k   ~ 0.1k
*/
/*
// The resistances in mOHM
const UINT16 rsns[] = {
	25,			// 25 mOHM
	20,			// 20 mOHM
	15,			// 15 mOHM
	10,			// 10 mOHM
	5,			// 5 mOHM
};
*/
INT16 ds2746_current(UINT8 addr, UINT16 resistance) {
	INT16 cur;
	INT8 s0, s1;

	msm_i2c_read(addr, DS2746_CURRENT_MSB, &s0, 1);
	msm_i2c_read(addr, DS2746_CURRENT_LSB, &s1, 1);

	cur = s0 << 8;
	cur |= s1;
	
	return (((cur >> 2) * DS2746_CURRENT_ACCUM_RES) / resistance);
}

INT16 ds2745_temperature(UINT8 addr) {
	INT16 temp;
	INT8 s0, s1;

	msm_i2c_read(addr, DS2745_TEMPERATURE_MSB, &s0, 1);
	msm_i2c_read(addr, DS2745_TEMPERATURE_LSB, &s1, 1);

	temp = s0 << 8;
	temp |= s1;
	
	return ((temp >> 5) * DS2745_TEMPERATURE_RES);
}
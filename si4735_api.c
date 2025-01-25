#include "si4735_api.h"

#include "patch_init.h"

#define _AM_MODE 0
#define _FM_MODE 1
#define _SSB_MODE 2
// #define SYNC_MODE 3

uint16_t MIN_LIMIT=200;
uint16_t  MAX_LIMIT=30000;
//#define IF_FEQ 455

uint16_t encoder=15200;
uint16_t pwm1=750;
uint16_t coef=5;
uint8_t encoder_mode=2;
int16_t bfo=0;
int16_t vol=0x32;  // 0x1a

/*
11 метров, 25.60 — 26.10 МГц (11,72 — 11,49 метра).
13 метров, 21.45 — 21.85 МГц (13,99 — 13,73 метра).
15 метров, 18.90 — 19.02 МГц (15,87 — 15,77 метра).
16 метров, 17.55 — 18.05 МГц (17,16 — 16,76 метра).
19 метров, 15.10 — 15.60 МГц (19,87 — 18,87 метра).
22 метра, 13.50 — 13.87 МГц (22,22 — 21,63 метра).
25 метров 11.60 — 12.10 МГц (25,86 — 24,79 метра).
31 метр, 9.40 — 9.99 МГц (31,91 — 30,03 метра).
41 метр, 7.20 — 7.50 МГц (41,67 — 39,47 метра).
49 метров, 5.85 — 6.35 МГц (52,36 — 47,66 метра).
60 метров, 4.75 — 5.06 МГц (63,16 — 59,29 метра).
75 метров, 3.90 — 4.00 МГц (76,92 — 75 метров).
90 метров, 3.20 — 3.40 МГц (93,75 — 88,24 метров).
120 метров (средние волны), 2.30 — 2.495 МГц (130,43 — 120,24 метра).
*/
uint16_t bands[]={200,1000,3100,3600,5800,7200,9300,11200,13500,14200,15100,17450,21500,27000};
uint8_t steps[]={1,5,10,50};
uint8_t reciver_mode=0;
//0 - am, 1 -fm, 2 - ssb

void delay(uint16_t ms){
	uint64_t temp;
	temp=ms<<10;
	while(temp--)__asm__("nop");
}

void si4734_reset(si4735App* app){
	furi_hal_gpio_write(app->output_pin, false); // SI4734_RST_CLR();
	delay(10); // delay(10); // furi_delay_ms(10);
	furi_hal_gpio_write(app->output_pin, true); // SI4734_RST_SET();
	delay(10); // delay(10); // furi_delay_ms(10);
}

uint8_t si4734_fm_mode(){
	FURI_LOG_E(TAG, "si4734_fm_mode()");
	// ARG1 (1<<4)|0 AN322 p130
	// ARG2 00000101
	uint8_t cmd[3]={POWER_UP,0x10,0x05};
	uint8_t status=0, tray=0;
	uint32_t timeout = 100;

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	// bool status_b = false;
    // status_b = furi_hal_i2c_tx(&furi_hal_i2c_handle_external, SI4734ADR, cmd, 3, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,3,0,0);
	// status_b = furi_hal_i2c_write_reg_8(&furi_hal_i2c_handle_external, SI4734ADR, POWER_UP, 0x10, timeout);
	// FURI_LOG_I(TAG, "status_b: %b", status_b);
	// status_b = furi_hal_i2c_write_reg_8(&furi_hal_i2c_handle_external, SI4734ADR, POWER_UP, 0x05, timeout);
	// FURI_LOG_I(TAG, "status_b: %b", status_b);
	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 3, timeout);
	// FURI_LOG_I(TAG, "status_b: %b", status_b);
	furi_delay_ms(1000); // furi_delay_ms(1000); // delay(1000);
#if 1	
    do{ 
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
		tray++;
		if(tray==255){
			FURI_LOG_E(TAG, "tray==255");
            furi_hal_i2c_release(&furi_hal_i2c_handle_external);
            return 0xff;
        }
		delay(50); // furi_delay_ms(50); // delay(50);
	}while(status!=0x80);
#endif
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return status; // status;
}

uint8_t si4734_am_mode(){
	// ARG1 (1<<4)|1 AN322 p130
	// ARG2 00000101
	uint8_t cmd[3]={POWER_UP,0x11,0x05};
	uint8_t status, tray=0;
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 3, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,3,0,0);
	delay(1000);
	do{ 
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(50);
	}while(status!=0x80);

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return status;
}

uint8_t si4734_set_prop(uint16_t prop, uint16_t val){
	uint8_t cmd[6]={SET_PROPERTY,0,(prop>>8),
									(prop&0xff),(val>>8),(val&0xff)};
	uint8_t status;
	uint32_t timeout = 100;
	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
	
	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 6, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,6,0,0);
	delay(100); // furi_delay_ms(100);
	furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
	
	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return status;
}

void reciver_set_mode(si4735App* app, uint8_t rec_mod){
	static uint16_t amfreq=15200,fmfreq=9920;//запоминаем старое значение // 8910
	
	si4734_powerdown();
								//частоты
	if(reciver_mode==_FM_MODE)fmfreq=app->freq_khz; else amfreq=encoder;
	if(rec_mod==_AM_MODE){
		//o_printf("AM mode\n");
		reciver_mode=_AM_MODE;
		si4734_am_mode();
		si4734_set_prop(AM_CHANNEL_FILTER, 0x0100);
		si4734_set_prop(AM_SOFT_MUTE_MAX_ATTENUATION, 0);//soft mute off
		si4734_set_prop(AM_AUTOMATIC_VOLUME_CONTROL_MAX_GAIN, 0x5000); //60дб
		si4734_set_prop(RX_VOLUME, vol);
		//si4734_set_prop(AM_SEEK_BAND_TOP, 30000);
		MIN_LIMIT=200;
		MAX_LIMIT=30000;
		//encoder=15200;
		encoder=amfreq-bfo/1000;//поправка на bfo
		bfo=bfo%1000;
		si4734_am_set_freq(encoder);
		coef=1;
		app->coef=coef;
		encoder_mode=0;
	} else if(rec_mod==_FM_MODE){
		//oled_clear();
		//o_printf("FM mode\n");
		reciver_mode=_FM_MODE;
		si4734_fm_mode();
		si4734_set_prop(FM_DEEMPHASIS,0x0001);//01 = 50 µs. Used in Europe, Australia, Japan
		si4734_set_prop(RX_VOLUME, vol);
		MIN_LIMIT=6000;
		MAX_LIMIT=11100;
		coef=1; // coef=1;
		app->coef=coef;
		//encoder=8910;
		// encoder=fmfreq;
		app->freq_khz = fmfreq;
		si4734_fm_set_freq(app->freq_khz); // encoder
		encoder_mode=0;
		//-----RDS-----
		// uint8_t status = 0;
		// char buff[14];
		si4735_RDS_set_interrupt();
		// sprintf(buff, "status = %c\r\n", status);
		// usart_transmit(&tx_rb, buff);
		si4735_RDS_set_group();
		// sprintf(buff, "status = %2X\r\n", status);
		// usart_transmit(&tx_rb, buff);
		si4735_Configures_RDS_setting();
		// sprintf(buff, "status = %2X\r\n", status);
		// usart_transmit(&tx_rb, buff);

		// status = si4734_get_int_status();
		// sprintf(buff, "status = %2X\r\n", status);
		// usart_transmit(&tx_rb, buff);
	}else{
		reciver_mode=_SSB_MODE;
		//bfo=0;
		si4734_ssb_patch_mode(ssb_patch_content);
		si4734_set_prop(0x0101,((1<<15)|(1<<12)|(1<<4)|2));//ssb man page 24
		si4734_set_prop(SSB_BFO, bfo);
		si4734_set_prop(AM_SOFT_MUTE_MAX_ATTENUATION, 0);//soft mute off
		si4734_set_prop(AM_AUTOMATIC_VOLUME_CONTROL_MAX_GAIN, 0x7000); //84дб
		si4734_set_prop(RX_VOLUME, vol);
		MIN_LIMIT=200;
		MAX_LIMIT=30000;
		//encoder=7100;
		encoder=amfreq;
		si4734_ssb_set_freq(encoder);
		coef=1;
		app->coef=coef;
		encoder_mode=0;
	}
}

uint8_t si4735_RDS_set_interrupt(){
	uint8_t status=0;
	/*
	  Enable RDSRECV interrupt (set RDSINT bit when RDS has filled the
	  FIFO by the amount set on FM_RDS_INTERRUPT_FIFO_COUNT
	  Reply Status. Clear-to-send high
	*/
	status = si4734_set_prop(0x1500, 0x0001); // FM_RDS_INT_SOURCE
	// usart_transmit(&tx_rb, "RDS_interrupt_setup: COMPLITED\r\n");
	// char buff[30];
	// sprintf(buff, "status = %02X\r\n", status);
	return status;
}

uint8_t si4735_RDS_set_group(){
	uint8_t status=0;
	/*
	  Sets the minimum number of
	  RDS groups stored in the
	  receive FIFO required before
	  RDSRECV is set.
	*/
	status = si4734_set_prop(0x1501, 0x0004); // FM_RDS_INT_FIFO_COUNT
	// usart_transmit(&tx_rb, "RDS_set_group: COMPLITED\r\n");
	// char buff[30];
	// sprintf(buff, "status = %02X\r\n", status);
	return status;
}

uint8_t si4735_Configures_RDS_setting(){
	uint8_t status=0;
	/*
	  Configures RDS setting.
	*/
	status = si4734_set_prop(0x1502, 0xEF01); // FM_RDS_CONFIG
	// usart_transmit(&tx_rb, "Configures_RDS_setting: COMPLITED\r\n");
	// char buff[30];
	// sprintf(buff, "status = %02X\r\n", status);
	return status;
}

uint8_t si4734_ssb_patch_mode(const uint8_t *patch){
	//ARG1 (1<<4)|1 AN322 p130
	//ARG2 00000101
	uint8_t cmd[3]={POWER_UP,0x31,0x05};
	uint8_t status, tray=0;
	uint16_t count,iterate=0;
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 3, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,3,0,0);
	delay(1000);
	do{	
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(50);
		}while(status!=0x80);
	if(status!=0x80) return 0x1;
	//count=(sizeof patch)/8;
	count=0x451;
	while(count--){
		tray=0;
		furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), patch+iterate, 8, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,patch+iterate,8,0,0);
		iterate+=8;
		delay(2);
		do{	
			furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
			tray++;
			if(tray==255){
				furi_hal_i2c_release(&furi_hal_i2c_handle_external);
				return 0x02;
			} 
			delay(1);
		}while(status!=0x80);
	}
	furi_hal_i2c_release(&furi_hal_i2c_handle_external);
	return status;
}

uint8_t si4734_fm_set_freq(uint16_t freq_10khz){
	uint8_t fast,freq_h,freq_l,status,tray=0;
	fast=0;
	freq_h=freq_10khz>>8;
	freq_l=freq_10khz&0xff;
	uint8_t cmd[6]={FM_TUNE_FREQ,fast,freq_h,freq_l,0,0};
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 6, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,6,0,0);
	delay(20);
	//do {status=si4734_get_int_status();
		//delay(50);
		//} while(!status || status&1);
	do{	
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
		tray++;
		if(tray==255){
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(20);
		}while(status!=0x80);

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return status;
}

uint8_t si4734_am_set_freq(uint16_t freq_khz){
	uint8_t fast,freq_h,freq_l,status,tray=0;
	fast=0;
	freq_h=freq_khz>>8;
	freq_l=freq_khz&0xff;
	uint8_t cmd[6]={AM_TUNE_FREQ,fast,freq_h,freq_l,0,1};
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 6, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,6,0,0);
	delay(20);
	//do {status=si4734_get_int_status();
		//delay(50);
		//} while(!status || status&1);
	do{	
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(20);
		}while(status!=0x80);

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return status;
}

uint8_t si4734_ssb_set_freq(uint16_t freq_khz){
	uint8_t mode,freq_h,freq_l,status,tray=0;
	if(freq_khz>10000)mode=0b10000000;else mode=0b01000000;
	freq_h=freq_khz>>8;
	freq_l=freq_khz&0xff;
	uint8_t cmd[6]={AM_TUNE_FREQ,mode,freq_h,freq_l,0,1};
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 6, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,6,0,0);
	delay(20);
	//do {status=si4734_get_int_status();
		//delay(50);
		//} while(!status || status&1);
	do{	
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(20);
		}while(status!=0x80);

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return status;
}

uint8_t si4734_powerdown(){
	uint8_t cmd=POWER_DOWN,status;
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), &cmd, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,&cmd,1,0,0);
	//usart_transmit("test3!\r\n");
	delay(200);
	furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), &status, 1, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
	// usart_transmit(&tx_rb, "si4734_powerdown: COMPLITED\r\n");
	furi_hal_i2c_release(&furi_hal_i2c_handle_external);
	return status;
}

void si4734_volume(int8_t dv){
	int16_t vol=si4734_get_prop(0x4000);//AN332 p170
	vol+=dv;
	if(vol<0)vol=0;
	if(vol>0x3f)vol=0x3f;
	si4734_set_prop(0x4000,vol);
}

uint16_t si4734_get_prop(uint16_t prop){
	uint8_t cmd[4]={GET_PROPERTY,0,(prop>>8),(prop&0xff)};
	uint8_t answer[4];
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 4, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,4,0,0);
	delay(100);
	furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), answer, 4, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,answer,4);

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return (answer[2]<<8)|answer[3];
}

void show_freq(si4735App* app, uint16_t freq, int16_t offset){
	//NB колонки в пиксилях, а строки по 8 пикселей, отсчёт с нуля из
	//верхнего левого угла
	uint16_t offset_hz;
	UNUSED(offset_hz);
	uint16_t freq_khz;
	// char buff[30];
	if(reciver_mode==_FM_MODE){
		// o_printf_at(18*5,1,1,0,"x10");
		app->multiplier_freq = 10;
		// sprintf(buff, "x10\r\n");
		// FURI_LOG_I(TAG, "x10");
	}
	else{ 
		app->multiplier_freq = 1;
		// o_printf_at(18*5,1,1,0,"   ");
		// sprintf(buff, "   \r\n");
	}
	// usart_transmit(&tx_rb, buff);
	//Вся эта канитель от того что bfo работает не как описанно в 
	//в даташите f=f-bfo а f=f-bfo
	if(reciver_mode==_SSB_MODE){
		offset_hz=(1000-offset%1000)%1000;
		freq_khz=freq-offset/1000;
		if(offset%1000>0)freq_khz--;
		// o_printf_at(18*5,3,1,0,"%03d",offset_hz);
		// sprintf(buff, "%03d\r\n", offset_hz);
		// usart_transmit(&tx_rb, buff);
		// o_printf_at(0,1,3,0,"%5d",freq_khz);
		// sprintf(buff, "%5d\r\n", freq_khz);
		// usart_transmit(&tx_rb, buff);
	}
	else{
		// o_printf_at(18*5,3,1,0,"   ");
		// sprintf(buff, "   \r\n");
		// usart_transmit(&tx_rb, buff);
		// o_printf_at(0,1,3,0,"%5d",freq);
		app->freq_khz=freq;
		// sprintf(buff, "%5d\r\n", freq);
		// usart_transmit(&tx_rb, buff);
		// FURI_LOG_I(TAG, "%5d", freq);
		}

	// o_printf_at(18*5,2,1,0,"KHz");
	// sprintf(buff, "KHz\r\n");
	// usart_transmit(&tx_rb, buff);
	
}

uint8_t get_recivier_signal_status(uint8_t *snr,uint8_t *rssi,uint8_t *freq_of){
	uint8_t status,resp1,resp2;
	switch(reciver_mode){
		case _AM_MODE: status=si4734_am_signal_status(&resp1,&resp2,rssi,snr);
			break;
		case _FM_MODE: status=si4734_fm_signal_status(rssi,snr,freq_of);
			break;
		case _SSB_MODE: status=si4734_am_signal_status(&resp1,&resp2,rssi,snr);
			break;
		default:
			status=0xff;
			break;
		}
	return status;
}

uint8_t si4734_fm_signal_status(uint8_t *rssi,uint8_t *snr,uint8_t *freq_of){
	uint8_t cmd[3]={FM_RSQ_STATUS,0x1};
	uint8_t tray=0;
	uint8_t answer[8];
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 2, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,2,0,0);
	delay(50);
	answer[0]=0;
	while(answer[0]==0){
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), answer, 8, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,answer,8);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(50);
	}

	*rssi=answer[4];
	*snr=answer[5];
	*freq_of=answer[7];

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return answer[0];
	
}

uint8_t si4734_am_signal_status(uint8_t *resp1,uint8_t *resp2,uint8_t *rssi,uint8_t *snr){
	uint8_t cmd[3]={AM_RSQ_STATUS,0x1};
	uint8_t tray=0;
	uint8_t answer[6];
	uint32_t timeout = 100;

	furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

	furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 2, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,2,0,0);

	delay(50);
	answer[0]=0;
	while(answer[0]==0){
		furi_hal_i2c_rx(&furi_hal_i2c_handle_external, ((SI4734ADR<<1)|0x1), answer, 6, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,0,0,answer,6);
		tray++;
		if(tray==255) {
			furi_hal_i2c_release(&furi_hal_i2c_handle_external);
			return 0xff;
		}
		delay(50);
		}
	*resp1=answer[1];
	*resp2=answer[2];
	*rssi=answer[4];
	*snr=answer[5];

	furi_hal_i2c_release(&furi_hal_i2c_handle_external);

	return answer[0];
	}

void show_reciver_status(si4735App* app, uint8_t snr, uint8_t rssi, uint8_t status){
	uint8_t n=1;
	//o_printf_at(1,4,1,0,"SNR:%2ddB SI: %2duVdB",snr,rssi);
	app->snr=snr;
	app->rssi=rssi;
	// char buff[30];
	// sprintf(buff, "SNR:%2ddB SI: %2duVdB\r\n",snr,rssi);
	// usart_transmit(&tx_rb, buff);
	//coef - глобальная переменная
	//n поправочный коэфициэнт шага
	if(reciver_mode==_FM_MODE)n=10;
	app->n=n;
	//o_printf_at(1,5,1,0,"status x%x %dKHz   ",status,coef*n);
	app->status=status;
	// sprintf(buff, "status x%x %dKHz   \r\n",status,coef*n);
	// usart_transmit(&tx_rb, buff);
}	

void show_reciver_full_status(si4735App* app, uint16_t freq, int16_t offset,uint8_t snr, uint8_t rssi, uint8_t status){
	show_freq(app, freq, offset);
	show_reciver_status(app, snr,rssi,status);
}
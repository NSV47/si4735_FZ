#include "si4735_api.h"

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

	bool status_b = false;
    // status_b = furi_hal_i2c_tx(&furi_hal_i2c_handle_external, SI4734ADR, cmd, 3, timeout); // i2c_transfer7(SI4734I2C,SI4734ADR,cmd,3,0,0);
	// status_b = furi_hal_i2c_write_reg_8(&furi_hal_i2c_handle_external, SI4734ADR, POWER_UP, 0x10, timeout);
	// FURI_LOG_I(TAG, "status_b: %b", status_b);
	// status_b = furi_hal_i2c_write_reg_8(&furi_hal_i2c_handle_external, SI4734ADR, POWER_UP, 0x05, timeout);
	// FURI_LOG_I(TAG, "status_b: %b", status_b);
	status_b = furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SI4734ADR<<1), cmd, 3, timeout);
	FURI_LOG_I(TAG, "status_b: %b", status_b);
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

#if 0
uint8_t si4734_am_mode(){
	// ARG1 (1<<4)|1 AN322 p130
	// ARG2 00000101
	uint8_t cmd[3]={POWER_UP,0x11,0x05};
	uint8_t status, tray=0;
	i2c_transfer7(SI4734I2C,SI4734ADR,cmd,3,0,0);
	delay(1000);
	do{ 
		i2c_transfer7(SI4734I2C,SI4734ADR,0,0,&status,1);
		tray++;
		if(tray==255) return 0xff;
		delay(50);
	}while(status!=0x80);
	return status;
}
#endif
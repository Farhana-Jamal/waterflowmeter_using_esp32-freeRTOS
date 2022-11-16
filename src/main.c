#include <string.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/task.h"
#include <driver/pcnt.h>
#include <stdio.h>

#include "font8x8_basic.h"

#define PCNT_GPIO 4
#define H_LIM_VALUE 1000
#define L_LIM_VALUE -10





#define OLED_I2C_ADDRESS   0x3C

// Control byte
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20    // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP      0xA1    
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8    
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14


#define SDA_PIN GPIO_NUM_21
#define SCL_PIN GPIO_NUM_22

#define tag "SSD1306"

float callibration_factor = 4.5;
float flowRate = 0.0;
float flowMilllilitres = 0.0;
float totalMillilitres = 0.0; 
float totalQuantity = 0.0;

char FlowRate[100];
char flowrateprnt[50];
char rateOfFlow[50];

char Quantity[100];
char quantityprnt[50];
char totalqty[50];



void floatToString_quantity(void *prmtr)
{
	while (1)
	{
	    float x = totalQuantity;
	
	    sprintf(Quantity, "%f" ,x);
	
	    printf("the quantity : %s \n" ,Quantity);

        strcpy(quantityprnt , Quantity);
        strcpy(totalqty,  "quantity:");

        printf("quantityprint : %s \n" , quantityprnt);
   
        printf("totalqty : %s \n",totalqty);


        strcat(totalqty, quantityprnt);

        printf("cocatenated string qty :  %s \n", totalqty);

	    vTaskDelay(1000/portTICK_PERIOD_MS);
	}


}



void floatToString_flowrate(void *prmtr)
{
	while (1)
	{
	    float x = flowRate;
	
	    sprintf(FlowRate, "%f" ,x);
	
	    printf("the flowrate : %s \n" ,FlowRate);

        strcpy(flowrateprnt , FlowRate);
        strcpy(rateOfFlow,  "flowrate:");

        printf("print : %s \n" , flowrateprnt);
   
        printf("FlowRate : %s \n",rateOfFlow);


        strcat(rateOfFlow, flowrateprnt);

        printf("cocatenated string :  %s \n", rateOfFlow);

	    vTaskDelay(1000/portTICK_PERIOD_MS);
	}


}

int pcnt_unit = PCNT_UNIT_0;

static void PCNT_init(int unit)
{
    pcnt_config_t pcnt_config = 
    {
        .pulse_gpio_num = PCNT_GPIO,
        .channel = PCNT_CHANNEL_0,
        .unit = unit,
        .pos_mode = PCNT_COUNT_INC,
        .neg_mode = PCNT_COUNT_DIS,
        .counter_h_lim = H_LIM_VALUE,
        .counter_l_lim = L_LIM_VALUE

    };
    pcnt_unit_config(&pcnt_config);

    
    pcnt_counter_pause(unit);
    pcnt_counter_clear(unit);
    pcnt_counter_resume(unit);
}


void calcul_task(void *prmtr)
{
   
    static int16_t count = 0;
    while(1)
    {
        pcnt_get_counter_value(pcnt_unit , &count);
        printf("pulsecount : %d \n", count );

        flowRate = (((1000.0 / 1000) * count) / callibration_factor);
        printf("flowrate : %f \n",flowRate);

        flowMilllilitres = (flowRate / 60) * 1000;
        
        totalMillilitres = totalMillilitres + flowMilllilitres;
         
        totalQuantity = totalMillilitres /1000; 

        pcnt_counter_pause(pcnt_unit);
        pcnt_counter_clear( pcnt_unit);
        pcnt_counter_resume(pcnt_unit);

        vTaskDelay(1000/portTICK_PERIOD_MS);

        printf("quantity : %f \n",totalQuantity);
    }


}



void i2c_master_init()
{
    i2c_config_t i2c_config = 
    {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 1000000
    };
    i2c_param_config(I2C_NUM_0, &i2c_config);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

void ssd1306_init() {
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    
    i2c_master_write_byte(cmd,OLED_CONTROL_BYTE_CMD_STREAM,true);
    i2c_master_write_byte(cmd,OLED_CMD_DISPLAY_OFF,true);
    i2c_master_write_byte(cmd,OLED_CMD_SET_MUX_RATIO,true);
    i2c_master_write_byte(cmd,0x3F,true);
    i2c_master_write_byte(cmd,OLED_CMD_SET_DISPLAY_OFFSET,true);
    i2c_master_write_byte(cmd,0x00,true);
    i2c_master_write_byte(cmd,OLED_CMD_SET_DISPLAY_START_LINE,true);
    i2c_master_write_byte(cmd,OLED_CMD_SET_SEGMENT_REMAP,true);
    i2c_master_write_byte(cmd,OLED_CMD_SET_COM_SCAN_MODE,true);
    i2c_master_write_byte(cmd,OLED_CMD_SET_COM_PIN_MAP,true);
    i2c_master_write_byte(cmd,0x12,true);
    i2c_master_write_byte(cmd,OLED_CMD_SET_CONTRAST,true);
    i2c_master_write_byte(cmd,0x7F,true);
    i2c_master_write_byte(cmd,OLED_CMD_DISPLAY_RAM,true);
    i2c_master_write_byte(cmd,OLED_CMD_DISPLAY_NORMAL,true);
    i2c_master_write_byte(cmd,OLED_CMD_SET_DISPLAY_CLK_DIV,true);
    i2c_master_write_byte(cmd,0x80,true);
    i2c_master_write_byte(cmd,OLED_CMD_SET_CHARGE_PUMP,true);
    i2c_master_write_byte(cmd,0x14,true);
    i2c_master_write_byte(cmd,OLED_CMD_SET_PRECHARGE,true);
    i2c_master_write_byte(cmd,0x22,true);
    i2c_master_write_byte(cmd,OLED_CMD_SET_VCOMH_DESELCT,true);
    i2c_master_write_byte(cmd,0x30,true);
    i2c_master_write_byte(cmd,OLED_CMD_SET_MEMORY_ADDR_MODE,true);
    i2c_master_write_byte(cmd,0x00,true);
    i2c_master_write_byte(cmd,OLED_CMD_DISPLAY_ON,true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

void task_displayClear(void *ignore)
{
    

    i2c_cmd_handle_t cmd;

    uint8_t zero[128] = {0};
	while(1)
	{

    for(uint8_t i = 0 ; i<8 ; i++)
    {
        
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE , true);
        i2c_master_write_byte(cmd , OLED_CONTROL_BYTE_CMD_SINGLE ,true);
        i2c_master_write_byte(cmd , 0xB0 |i , true );
        i2c_master_write_byte(cmd , OLED_CONTROL_BYTE_DATA_STREAM , true);
        i2c_master_write(cmd ,zero , 128 , true);
        i2c_master_stop(cmd);
        i2c_master_cmd_begin(I2C_NUM_0 ,cmd ,10/portTICK_PERIOD_MS);
        
        i2c_cmd_link_delete(cmd);   
    }
    vTaskDelete(NULL);

	}
}



void task_displayText(void *argtext)
{   
	while(1)
	{
    char *text = (char *)argtext;
	printf("string : %s \n", rateOfFlow);
    uint8_t text_len = strlen(text);

    uint8_t cur_page = 0;
    
   
    
    i2c_cmd_handle_t cmd;


    cmd  = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd,OLED_CONTROL_BYTE_CMD_STREAM, true);
    i2c_master_write_byte(cmd,0x00, true);
    i2c_master_write_byte(cmd,0x10, true);
    i2c_master_write_byte(cmd,0xB0 | cur_page, true);;
    i2c_master_stop(cmd);
    
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    
    i2c_cmd_link_delete(cmd);

    for(uint16_t i=0; i<text_len; i++)
    {
        if(text[i] == '\n')
        {
            cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
            i2c_master_write_byte(cmd , OLED_CONTROL_BYTE_CMD_STREAM , true) ;
            i2c_master_write_byte(cmd , 0x00 , true);
            i2c_master_write_byte(cmd , 0x10 , true);
            i2c_master_write_byte(cmd , 0xB0 | ++cur_page , true);
            i2c_master_stop(cmd);
            i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
            i2c_cmd_link_delete(cmd);
        }
        else
        {

            cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
            i2c_master_write_byte(cmd,OLED_CONTROL_BYTE_DATA_STREAM, true);
            i2c_master_write(cmd , font8x8_basic_tr[(uint8_t)text[i]] , 8 ,true);
            i2c_master_stop(cmd);  
            i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
            
            i2c_cmd_link_delete(cmd);

     
        }
    } vTaskDelay(1000/portTICK_PERIOD_MS);
	}
    vTaskDelete(NULL);
}
void app_main(void)
{
     PCNT_init(pcnt_unit);

    

    xTaskCreate(calcul_task , "calculation task" ,2048 , NULL , 5 ,NULL );

	xTaskCreate(floatToString_flowrate , "float to string_fr " , 2048 , NULL ,5 , NULL);
	xTaskCreate(floatToString_quantity , "float to string_qty", 2048 , NULL, 5, NULL);

	// concatenate();
    
    i2c_master_init();
    ssd1306_init();

    xTaskCreate(&task_displayClear , "displayClear" , 2048 , NULL  , 5 , NULL);
    vTaskDelay(100/portTICK_PERIOD_MS);

    xTaskCreate(&task_displayText ,"displayText" , 2048 , rateOfFlow , 5 , NULL);
    vTaskDelay(1000/portTICK_PERIOD_MS);
	
}




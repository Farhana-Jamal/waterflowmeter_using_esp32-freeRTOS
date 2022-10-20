#include <freertos/FreeRTOS.h>
#include <driver/pcnt.h>
#include <freertos/task.h>


#define PCNT_GPIO 4
#define H_LIM_VALUE 1000
#define L_LIM_VALUE -10

#define callibration_factor  4.5

float flowRate = 0.0;
float flowMilllilitres = 0.0;
float totalMillilitres = 0.0; 
float totalQuantity = 0.0;


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

void app_main()
{
    PCNT_init(pcnt_unit);


    xTaskCreate(calcul_task , "calculation task" ,2048 , NULL , 1 ,NULL );


  
  
  
  
  
  
  
  
  
  /*  int pcnt_unit = PCNT_UNIT_0;
    PCNT_init(pcnt_unit);
    

    int16_t count = 0;
    while(1)
    {
        pcnt_get_counter_value(pcnt_unit , &count);
        printf("%d \n", count );
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }*/
}


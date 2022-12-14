/*
 * Программа для микроконтроллера Анруино Нано Евери
 * Предназначение - управление работой сенсора на основе полевого транзистора.
 * 
 * Принимает управляющие байты от USB порта для установки напряжения двух ЦАП MCP4725 (затвор и канал), 
 * а также запуска и опроса одного АЦП MCP3421(или MCP3424 - в этом случае можно добавлять еще АЦП с дописыванием кода и добавлением команд).
 * Возвращает в порт результаты измерения при опросе АЦП.
 * Программа не преобразует напряжение в отсчеты и обратно, все это вынесено в управляющую программу на ПК (для облегчения отладки)
 * 
 * автор - Парфенов Петр, qrspeter@gmail.com для Орловой Анны Олеговны (Университет ИТМО)
 * 
 * 
 * 
 * */



#include "Wire.h"


// адреса устройств
const byte DAC_GATE_ADDRESS   = 0x62;
const byte DAC_DRAIN_ADDRESS  = 0x63;
const byte ADC_DRAIN_ADDRESS  = 0x68;


// переменная для режимов АЦП, влияет на длину возвращаемых данных. Прочие изменения не влияют и должны учитываться в управляющей программе на ПК.
enum adc_bit_mode
{
  bit18     = 0,
  bit12_16  = 1
};

// маска очищающая старший полубайт старшего байта, на всякий случай (там 12-битное значение, где игнорируются старшие 4, делаем на всякий случай, тк так там флаги)
const byte dac_mask   = 0b00001111; // ? 010-Sets in Write mode? 0b01000000

// стартовое состояние ЦАП
byte status_ADC_drain = 0b10001100;

// маска для битов, отвечающих на разрядность преобразования
const byte ADC_mode_mask = 0b00001100;

 

// operation codes // or enum/struct? https://radioprog.ru/post/655
const byte sensor_reset   = 0;
const byte setDAC_Gate    = 1;
const byte setDAC_Drain   = 2;
const byte setADC         = 3; // а может и не нужен, если режим задавать в команде опроса АЦП. Хотя проще период считать на компе, снимая отладку с микроконтроллера
const byte getADC         = 4; // а вот тут куча свободных бит для режима работы



struct ADC_result_18
{
    byte meas_2;
    byte meas_1;
    byte meas_0;
    byte meas_status;
} adc18;

struct ADC_result_12_16
{
    byte meas_1;
    byte meas_0;
    byte meas_status;
} adc12_16;



struct DAC_voltage
{
    byte volt_0;
    byte volt_1;
} voltage;


void Sensor_reset()
{

   Wire.beginTransmission(0x00); // The general call addresses all devices on the bus using the I2C address 0.
   Wire.write(0x06); 
   Wire.endTransmission();

        Wire.beginTransmission(DAC_GATE_ADDRESS);
        Wire.write(0x00);  // потом это должен быть не 0, а 4096/2=2048 = x800, а то будет висеть минус ХЗ    
        Wire.write(0x00);
        Wire.endTransmission();  

        Wire.beginTransmission(DAC_DRAIN_ADDRESS);
        Wire.write(0x00);  // потом это должен быть не 0, а 4096/2=2048 = x800, а то будет висеть минус ХЗ    
        Wire.write(0x00);
        Wire.endTransmission();  
   
   
   Wire.beginTransmission(ADC_DRAIN_ADDRESS);
   Wire.write(0b00001100); // первый канал (00), единичная выборка (0), 18 бит (11), единичное усиление (00)
   Wire.endTransmission();
           
}


void setup() {

// ==============
  // TEST initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
// ==============
  
  Serial.begin(9600);
  Wire.begin();

  //Sensor_reset();
 // ADC setup
  Wire.beginTransmission(ADC_DRAIN_ADDRESS);// 
  Wire.write(0b00001100); // Настройка в 12 бит - Wire.write(B10011)... 
  Wire.endTransmission();

/*
  // for initial programming only ============================   потом это должен быть не 0, а 7F, а то будет висеть минус ХЗ    

        Wire.beginTransmission(DAC_GATE_ADDRESS);
        Wire.write(0b01100000);  // c2=0, c1=1, c2=1 (DAC + EEPROM), XX unused, PD1=PD0=0, X unused
        Wire.write(0x00);
        Wire.write(0x00);
        Wire.endTransmission();  

        Wire.beginTransmission(DAC_DRAIN_ADDRESS);
        Wire.write(0b01100000);  // c2=0, c1=1, c2=1 (DAC + EEPROM), XX unused, PD1=PD0=0, X unused
        Wire.write(0x00);
        Wire.write(0x00);
        Wire.endTransmission();  
*/ // ============================       

        Wire.beginTransmission(DAC_GATE_ADDRESS);
        Wire.write(0x00);  // потом это должен быть не 0, а 4096/2=2048 = x800, а то будет висеть минус ХЗ    
        Wire.write(0x00);
        Wire.endTransmission();  

        Wire.beginTransmission(DAC_DRAIN_ADDRESS);
        Wire.write(0x00);  // потом это должен быть не 0, а 4096/2=2048 = x800, а то будет висеть минус ХЗ    
        Wire.write(0x00);
        Wire.endTransmission();    
  
}

void loop() 
{
/*
// ==============
  // TEST wait for a second
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(500);                        
  digitalWrite(LED_BUILTIN, LOW);   
  delay(500);                      
// ==============
  */

  int first_lap_sign = 0;
  delay(100);

    if( Serial.available()) 
    {
  
      switch(Serial.read())
      {
        // или через указатели http://mypractic.ru/urok-15-ukazateli-v-c-dlya-arduino-preobrazovanie-raznyx-tipov-dannyx-v-bajty.html 
	        case sensor_reset:
            Sensor_reset();

            break;
  //===============================================================
        case setDAC_Gate:
        // get 12 bit
     //   dac_voltage.voltage = Serial.read();
        voltage.volt_1 = Serial.read();
        voltage.volt_0 = Serial.read();
        
        Wire.beginTransmission(DAC_GATE_ADDRESS);
//        Wire.write(dac_control);
        voltage.volt_1 = voltage.volt_1 & dac_mask;
        Wire.write(voltage.volt_1);
        Wire.write(voltage.volt_0);  
        Wire.endTransmission();   
        
   //     status_byte = 1;           
    //    Serial.write(status_byte);      
        
        break;
        
  //===============================================================        
        case setDAC_Drain:
        // get 12 bit
        voltage.volt_1 = Serial.read();
        voltage.volt_0 = Serial.read();
  
        Wire.beginTransmission(DAC_DRAIN_ADDRESS);
//        Wire.write(dac_control);
        voltage.volt_1 = voltage.volt_1 & dac_mask;
        Wire.write(voltage.volt_1);
        Wire.write(voltage.volt_0);     
        Wire.endTransmission();           
  
  //      status_byte = 1;           
  //      Serial.write(status_byte);      
        
        break;
  
  //===============================================================
        case setADC:
        status_ADC_drain = Serial.read();
        Wire.beginTransmission(ADC_DRAIN_ADDRESS);
        Wire.write(status_ADC_drain);  //  12-14-16-18 бит это b10000000-b10000100-b10001000-b10001100 (x80/x84/x88/x8C)
        Wire.endTransmission();

   //     status_byte = 1;           
  //      Serial.write(status_byte);      

        break;

  //===============================================================
        case getADC:
  
        Wire.beginTransmission(ADC_DRAIN_ADDRESS);
  
        byte adc_status;
        adc_bit_mode bit_mode;
        adc_status = status_ADC_drain & ADC_mode_mask; // устанавливается при запуске АЦП, то есть опрос не проводить до запуска
        
        if(adc_status == ADC_mode_mask) // status_ADC_drain = 0b10001100, ADC_mode_mask = 0b00001100
        bit_mode = bit18;
        else
        bit_mode = bit12_16;

        
        do
        {
          if(first_lap_sign)
          delay(50); 
          
          if(bit_mode == bit18) 
          {
            Wire.requestFrom(ADC_DRAIN_ADDRESS, sizeof(adc18)); 
  
            adc18.meas_2 = Wire.read();
            adc18.meas_1 = Wire.read();
            adc18.meas_0 = Wire.read();
            adc18.meas_status = Wire.read();
            adc_status = adc18.meas_status;
             
          }
          else
          {
            Wire.requestFrom(ADC_DRAIN_ADDRESS, sizeof(adc12_16)); 
            adc12_16.meas_1 = Wire.read();
            adc12_16.meas_0 = Wire.read();
            adc12_16.meas_status = Wire.read();
            adc_status = adc12_16.meas_status;
           
          }
          first_lap_sign = 1;
          
         }
         while( (adc_status >> 7) && 1 ); // if bit 7 == 0 data was updated
        
         if(bit_mode == bit18)
         Serial.write((byte*)&adc18, sizeof(adc18));
         else
         Serial.write((byte*)&adc12_16, sizeof(adc12_16));

         break;
        
  //      default:
        
      }


    }
    
}

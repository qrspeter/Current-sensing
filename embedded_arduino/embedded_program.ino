/*
 * Программа для микроконтроллера Анруино Nano Every
 * Предназначение - управление работой сенсора на основе полевого транзистора.
 * автор - Парфенов Петр, qrspeter@gmail.com для Орловой Анны Олеговны (Университет ИТМО)
 * 
 * Принимает управляющие байты (operation codes) от USB порта для сброса и установки напряжения двух ЦАП MCP4725 (затвор и канал), 
 * а также настройки/запуска и опроса одного АЦП MCP3421(или MCP3424 - 
 * в этом случае можно добавлять еще АЦП с дописыванием кода и добавлением команд).
 * Последняя команда возвращает в порт результаты измерения при опросе АЦП (длиной 2 или 3 байта в зависимости от разрядности).
 * Программа не преобразует напряжение в отсчеты и обратно, все это вынесено в управляющую программу на ПК (для облегчения отладки)
 * Также не информирует о статусе преобразования и о успешности записи команд в периферийные устройства.
 * 
 * Требуется "переинициализация", когда работа начнется с биполярным напряжением, 
 * тк сейчас по умолчанию на выходах ЦАП ноль, 
 * который будет равен мину ХХ В в биполярном режиме/
 * А также изменение констант смещения ЦАП.
 * 
 * Может еще не хватает функции "разбудить-усыпить", чтобы в обычное время опрос порта шел с задержкой в цикле 100 мс, а с приемом первого байта - без задержек. 
 * Но тогда надо знать сколко байтов будет принято
 * 
 * */

 // " ардуино шлет пакеты младшим байтом вперед, отсюда и преобразование 321 ->132"  https://ru.stackoverflow.com/questions/1237290/



#include "Wire.h" // This library allows you to communicate with I2C/TWI devices


// адреса устройств
const byte DAC_GATE_ADDRESS   = 0x60; // Troyka - 0x62
const byte DAC_DRAIN_ADDRESS  = 0x61; // Troyka - 0x63
const byte ADC_DRAIN_ADDRESS  = 0x68;

void(* resetFunc) (void) = 0;


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
    byte volt_1;
    byte volt_0;
};

struct DAC_voltage voltage;
struct DAC_voltage drain_bias =  {0x80, 0}; // потом это должен быть не 0, а ~4096/2=2048 = x800 = x08 + x00, а то будет висеть минус ХЗ
struct DAC_voltage gate_bias  =  {0x80, 0}; // потом это должен быть не 0, а ~4096/2=2048 = x800, а то будет висеть минус ХЗ


void Sensor_reset()
{

  resetFunc(); //  NVIC_SystemReset(); // asm volatile (”jmp 0″); // void (softReset){ asm volatile (" jmp 0");
  
   Wire.beginTransmission(0x00); // The general call addresses all devices on the bus using the I2C address 0.
   Wire.write(0x06); 
   Wire.endTransmission();

        Wire.beginTransmission(DAC_GATE_ADDRESS);
        Wire.write(gate_bias.volt_1);     
        Wire.write(gate_bias.volt_0);
        Wire.endTransmission();  

        Wire.beginTransmission(DAC_DRAIN_ADDRESS);
        Wire.write(drain_bias.volt_1);    
        Wire.write(drain_bias.volt_0);
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


  // for initial programming only ============================   set initial voltage    
/*
        Wire.beginTransmission(DAC_GATE_ADDRESS);
        Wire.write(0b01100000);  // c2=0, c1=1, c2=1 (DAC + EEPROM), XX unused, PD1=PD0=0, X unused
        Wire.write(gate_bias.volt_1);     
        Wire.write(gate_bias.volt_0);
        Wire.endTransmission();  

        Wire.beginTransmission(DAC_DRAIN_ADDRESS);
        Wire.write(0b01100000);  // c2=0, c1=1, c2=1 (DAC + EEPROM), XX unused, PD1=PD0=0, X unused
        Wire.write(drain_bias.volt_1);    
        Wire.write(drain_bias.volt_0);
        Wire.endTransmission();

*/ // ============================       


        

        Wire.beginTransmission(DAC_GATE_ADDRESS);
        Wire.write(gate_bias.volt_1);     
        Wire.write(gate_bias.volt_0);
        Wire.endTransmission();  

        Wire.beginTransmission(DAC_DRAIN_ADDRESS);
        Wire.write(drain_bias.volt_1);    
        Wire.write(drain_bias.volt_0);
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

 // int first_lap_sign = 0;

    if( Serial.available()) 
    {
      delay(7); // задержка тормозит получение данных при большой скорость выборки, а если убрать - то на 18бит начинаются странные данные, ну и помехи на других режимах. Тут 70 вроде минимум

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

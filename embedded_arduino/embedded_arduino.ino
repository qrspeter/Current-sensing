/*
 * Pprogram for Arduino Nano Every microcontroller
 * Purpose: control of the operation of a sensor based on a field-effect transistor, 
 * resistive sensors, and also measurement of IV characteristics.
 * Author - Peter Parfenov, qrspeter@gmail.com for Orlova Anna (ITMO University)

 * This work was supported by the Ministry of Science and Higher Education of the Russian Federation, 
 * goszаdanie no.2019-1080.
 *
 * International Research and Education Centre for Physics of Nanostructures, 
 * ITMO University, Saint Petersburg 197101, Russia
 * 
 * Receives operation codes from USB port to reset and set voltage 
 * of two MCP4725 DACs (gate and channel), 
 * as well as setup/start and polling of one ADC MCP3421.
 * Returns measurement results to the port when polling the ADC
 * (2 or 3 bytes depending on the bit depth of the ADC conversion).
 * The program does not convert voltage to DAC readings and back from ADC readings to voltage. 
 * It receives and sends data in ADC and DAC format.
 * It also does not check the  success of writing  commands to peripheral devices.
 * 
 * The DAC offset constants need to be changed.
 * 
 * 
 * */

 // arduino sends packets with the least significant byte first, hence the conversionе 321 ->132
 //  https://ru.stackoverflow.com/questions/1237290/



#include "Wire.h" // This library allows you to communicate with I2C/TWI devices

const char model[] = "Current Sensor";
const char version[] = "ver. 1.1";

// Device addresses
const byte DAC_GATE_ADDRESS   = 0x60; // Troyka - 0x62
const byte DAC_DRAIN_ADDRESS  = 0x61; // Troyka - 0x63
const byte ADC_DRAIN_ADDRESS  = 0x68;

// 12bit = 1000/240 = 5ms, 14bit = 1000/60 = 17ms, 16bit = 1000/15 = 67ms, 18bit = 1000/3.75 = 267ms
const byte conversion_time[4]{ 5, 17, 67, 267 }; 

void(* resetFunc) (void) = 0;


// ADC returns 2 bytes in 12 and 16 bit mode, and 3 bytes in 18-bit mode. 
enum adc_bit_mode
{
  bit18     = 0,
  bit12_16  = 1
};

// mask clears the high nibble of the high byte, just in case 
// (there is a 12-bit value, where the upper 4 are ignored, because there are flags)
const byte dac_mask   = 0b00001111; // ? 010-Sets in Write mode? 0b01000000

// initial DAC state
byte status_ADC_drain = 0b10001100;

// mask for bits that correspond to the conversion bit depth
const byte ADC_mode_mask = 0b00001100;

byte averaging{1};
 

// operation codes // or enum/struct? https://radioprog.ru/post/655
const byte sensor_reset   = 0;
const byte setDAC_Gate    = 1;
const byte setDAC_Drain   = 2;
const byte setADC         = 3; 
const byte getADC         = 4; 
const byte setLaser_On    = 5;
const byte setLaser_Off   = 6;
const byte setADCav       = 7;


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

union
{
  uint32_t adc_result;
  struct
  {
    byte meas_3;
    byte meas_2;
    byte meas_1;
    byte meas_0;
  };
}adc;

byte meas_status{};


struct DAC_voltage
{
    byte volt_1;
    byte volt_0;
};

struct DAC_voltage voltage;
struct DAC_voltage drain_bias =  {0x80, 0}; // 4096/2=2048 = x800 = x08 + x00
struct DAC_voltage gate_bias  =  {0x80, 0}; // 


byte adc_status;
adc_bit_mode bit_mode;

void Sensor_reset()
{

//   resetFunc(); //  NVIC_SystemReset(); // asm volatile (”jmp 0″); 
// void (softReset){ asm volatile (" jmp 0");
  
   Wire.beginTransmission(0x00); // The general call addresses all devices using the I2C address 0.
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
   Wire.write(ADC_mode_mask); // channel 1 (00), single poll (0), 18 bit (11), amplification=1 (00)
   Wire.endTransmission();  
           
}



void setup() {

// ==============
  // TEST initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);  // pinMode(13, OUTPUT);
  
// ==============
  // TEST wait for a second
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(500);   
  digitalWrite(LED_BUILTIN, LOW);   

// ==============
  
  Serial.begin(9600);
  Wire.begin();

 // ADC setup
  Wire.beginTransmission(ADC_DRAIN_ADDRESS);// 
  Wire.write(ADC_mode_mask); // Set 12 bit - Wire.write(B10011)... 
  Wire.endTransmission();


  // for initial programming only ============================   set initial voltage ~2.5V   
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

	int first_lap_sign {1};

    if(Serial.available()) 
    {

      switch(Serial.read())
      {
        // or using pointers http://mypractic.ru/urok-15-ukazateli-v-c-dlya-arduino-preobrazovanie-raznyx-tipov-dannyx-v-bajty.html 
	      case sensor_reset:
            Sensor_reset();

            break;
  //===============================================================
        case setDAC_Gate:
        while(Serial.available() < 2){ delay(2); }
        // get 12 bit
        voltage.volt_1 = Serial.read();
        voltage.volt_0 = Serial.read();
        
        Wire.beginTransmission(DAC_GATE_ADDRESS);
//        Wire.write(dac_control);
        voltage.volt_1 = voltage.volt_1 & dac_mask;
        Wire.write(voltage.volt_1);
        Wire.write(voltage.volt_0);  
        Wire.endTransmission();   
               
        break;
        
  //===============================================================        
        case setDAC_Drain:
        // get 12 bit
        while(Serial.available() < 2){ delay(2); }
        voltage.volt_1 = Serial.read();
        voltage.volt_0 = Serial.read();
  
        Wire.beginTransmission(DAC_DRAIN_ADDRESS);
        voltage.volt_1 = voltage.volt_1 & dac_mask;
        Wire.write(voltage.volt_1);
        Wire.write(voltage.volt_0);     
        Wire.endTransmission();           
  
        break;
  
  //===============================================================
        case setADC:
        while(Serial.available() < 1){ delay(2); }
        status_ADC_drain = Serial.read();
        Wire.beginTransmission(ADC_DRAIN_ADDRESS);
        //  12-14-16-18 bit are b10000000-b10000100-b10001000-b10001100 (x80/x84/x88/x8C)
        Wire.write(status_ADC_drain);  
        Wire.endTransmission();

   //     status_byte = 1;           
  //      Serial.write(status_byte);      
        break;

  //===============================================================
        case getADC:

        adc_status = status_ADC_drain & ADC_mode_mask; // устанавливается при запуске АЦП, то есть опрос не проводить до запуска

        if(adc_status == ADC_mode_mask) // status_ADC_drain = 0b10001100, ADC_mode_mask = 0b00001100
        bit_mode = bit18;
        else
        bit_mode = bit12_16;

		Wire.beginTransmission(ADC_DRAIN_ADDRESS);
		
    // debugging:
        if(averaging == 1)
        {         
    
          do
          {
            if(!first_lap_sign)
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
            first_lap_sign = 0;
            
          }
          while( (adc_status >> 7) && 1 ); // if bit 7 == 0 data was updated

          if(bit_mode == bit18)
          Serial.write((byte*)&adc18, sizeof(adc18));
          else
          Serial.write((byte*)&adc12_16, sizeof(adc12_16));
        }        
        else
        {
          break;
          /*
          byte mode = adc_status >> 2;
          uint32_t accumulation{0};
 
          if(adc_status == ADC_mode_mask) // status_ADC_drain = 0b10001100, ADC_mode_mask = 0b00001100
          bit_mode = bit18;
          else
          bit_mode = bit12_16;


          for(byte i = 0; i < averaging; ++i)
          {
            adc.adc_result = 0;

            Wire.write(status_ADC_drain);  //  12-14-16-18 бит это b10000000-b10000100-b10001000-b10001100 (x80/x84/x88/x8C)

            delay(conversion_time[mode]);            

            Wire.beginTransmission(ADC_DRAIN_ADDRESS);
      
            do
            {
              if(!first_lap_sign)
              delay(50); 
              
              if(bit_mode == bit18) 
              {
                Wire.requestFrom(ADC_DRAIN_ADDRESS, sizeof(adc18)); 
      
                adc.meas_2 = Wire.read();
                adc.meas_1 = Wire.read();
                adc.meas_0 = Wire.read();
                meas_status = Wire.read();
                adc_status = meas_status;
                
              }
              else
              {
                Wire.requestFrom(ADC_DRAIN_ADDRESS, sizeof(adc12_16)); 
                adc.meas_1 = Wire.read();
                adc.meas_0 = Wire.read();
                meas_status = Wire.read();
                adc_status = meas_status;
              
              }
              first_lap_sign = 0;
              
            }
            while( (adc_status >> 7) && 1 ); // if bit 7 == 0 data was updated

            int raw_adc{0};
            if(bit_mode == bit18)
            {
              if (adc.meas_2 > 0x7F)
              {
                raw_adc =  0xFF000000 + static_cast<unsigned int> (0x10000 * adc.meas_2) + static_cast<unsigned int> (0x100 * adc.meas_1) + static_cast<unsigned int> (adc.meas_0);
              }
              else
                raw_adc = static_cast<unsigned int> (0x10000 * adc.meas_2) + static_cast<unsigned int> (0x100 * adc.meas_1) + static_cast<unsigned int> (adc.meas_0);
            }
            else
            {
              if(adc.meas_1 > 0x7F)
              {
                raw_adc =  0xFFFF0000 + static_cast<unsigned int> (0x100 * adc.meas_1) + static_cast<unsigned int> (adc.meas_0);                
              }
              else
                raw_adc =  static_cast<unsigned int> (0x100 * adc.meas_1) + static_cast<unsigned int> (adc.meas_0);
            }
            accumulation += raw_adc;

          }
 
        
        adc.adc_result = accumulation / averaging; //;


          if(bit_mode == bit18){ Serial.write((byte*)&adc.meas_2, sizeof(byte)); }

            Serial.write((byte*)&adc.meas_1, sizeof(byte));
            Serial.write((byte*)&adc.meas_0, sizeof(byte));
          */
        }

		    Wire.endTransmission();

        break;

  //===============================================================
        case setLaser_On:
			    digitalWrite(LED_BUILTIN, HIGH);
//			digitalWrite(13, HIGH);

        break;

  //===============================================================
        case setLaser_Off:
			    digitalWrite(LED_BUILTIN, LOW);

        break;

  //===============================================================      
        case setADCav:
          while(Serial.available() < 1){ delay(2); }
//          status_ADC_drain = Serial.read();
          averaging = Serial.read();

   //     status_byte = 1;           
  //      Serial.write(status_byte);      

        break;

  //===============================================================
        default:
          byte error_byte = 0xFF;
          Serial.write(error_byte);
        
      }

    }
   
}

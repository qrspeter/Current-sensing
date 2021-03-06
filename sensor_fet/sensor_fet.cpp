/**
 *  ?????????? ??????? ?????? ? ????????.
 *  ????????????? ?????????? ???? ??? (?????? ? ????), ??????????? ?????????? double ? ??????? ???.
 *  ????????? ??? ? ?????? ??? ????????? (????? ???, ???????????, ????????)
 *  ????????? ??????? ??? ? ???????????? ?? ? ?????????? (???? ??? ????? ????? ???).
 *  ? ????? ????????????? ????????? ????????? ???????? ? ?????????? ? ???????.
 *
 */




//#include <iostream>
//#include <wx/filename.h> // ??? wxYield

#include <windows.h>
#include <string>
#include <cmath> // pow(a, b) = ab

#include "sensor_fet.h"

HANDLE hSerial = INVALID_HANDLE_VALUE;

resolution current_res;
gain current_gain;
uint8_t status;

const int INPUT_BUFF_SIZE = 16;


DWORD bc;

union{
  uint16_t  voltage;
  struct{
    uint8_t volt_0;
    uint8_t volt_1;
  };
} dac_voltage;

struct ADC_result
{
    uint8_t meas_2;
    uint8_t meas_1;
    uint8_t meas_0;
    uint8_t meas_status;
} adc;
/*
union{
  int32_t voltage;
  struct{
    uint8_t volt_3;
    uint8_t volt_2;
    uint8_t volt_1;
    uint8_t volt_0;
  };
} adc_voltage;

uint8_t adc_status;*/


/*
struct ADC_result
{
    byte meas_2;
    byte meas_1;
    byte meas_0;
    byte meas_status;
} adc;
*/


// operation codes // change to enum with fixed values, e.g. enum Suit { Diamonds = 5, Hearts, Clubs = 4, Spades };
const uint8_t resetSensor   = 0;
const uint8_t setDAC_Gate   = 1;
const uint8_t setDAC_Drain  = 2;
const uint8_t setADC        = 3; // ? ????? ? ?? ?????, ???? ????? ???????? ? ??????? ?????? ???. ???? ????? ?????? ??????? ?? ?????, ?????? ??????? ? ????????????????
const uint8_t getADC        = 4;


SENSOR_FET::SENSOR_FET()
{

}

SENSOR_FET::~SENSOR_FET()
{
    if(hSerial != INVALID_HANDLE_VALUE)
	CloseHandle(hSerial);

}

int SENSOR_FET::Open(int port)
{

    char com_name[] = "COM3";
    if(port > 9)
	{
		MessageBox(NULL, "Wrong number!", "Error", MB_OK);
		CloseHandle(hSerial);
		return(0);
	}

    com_port = port;

    com_name[3] = (char)(port + 0x30);

    hSerial = CreateFileA((LPCSTR)com_name, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);

	if(hSerial == INVALID_HANDLE_VALUE) // ???????? ????????? ?????. NULL for 32-bit Win and INVALID_HANDLE_VALUE for 64bit
	{
	    if(GetLastError() == ERROR_FILE_NOT_FOUND)
        MessageBox(NULL, "COM port is not found", "Error", MB_OK);
        else
		MessageBox(NULL, "Error Open COM", "Error", MB_OK);
		return(0);
	}

	SetupComm(hSerial, INPUT_BUFF_SIZE, INPUT_BUFF_SIZE); // ??????? ???????? ?????? ?????.


	DCB dcb; //  = {0}
	if(!GetCommState(hSerial, &dcb))
	{
		MessageBox(NULL, "Port DCB error!","Error", MB_OK);
		CloseHandle(hSerial);
		return(0);
	}


	dcb.BaudRate    = CBR_9600;
	dcb.ByteSize    = 8;
	dcb.Parity      = NOPARITY;
	dcb.StopBits    = ONESTOPBIT;
	dcb.fDtrControl = DTR_CONTROL_ENABLE; // ????? ???????? ?????? ????? ????????? ????????? ?????, ???? ?????? https://github.com/dmicha16/simple_serial_port/blob/master/simple-serial-port/simple-serial-port/SimpleSerial.cpp
	// ???? ???????, ???????? ???????? ?? ????? ??? ?????????
	// DTR_CONTROL_DISABLE; // disable DTR to avoid reset SetCommState(m_hCom, &dcb);  https://forum.arduino.cc/t/disable-auto-reset-by-serial-connection/28248/12
	// ???? ... ??? "Data terminal ready" https://docs.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-dcb

    // ????????? ????????? ?????.
	if(!SetCommState(hSerial, &dcb))
	{
		MessageBox(NULL,"Port parameter error!", "Error", MB_OK);
		CloseHandle(hSerial);
		return(0);
	}

	COMMTIMEOUTS  ct;

	ct.ReadIntervalTimeout = MAXDWORD ; // 3000;//MAXDWORD;
	ct.ReadTotalTimeoutMultiplier = MAXDWORD; // 3000;// MAXDWORD;
	ct.ReadTotalTimeoutConstant = 2000; //2000;//100;
	ct.WriteTotalTimeoutMultiplier = 0;
	ct.WriteTotalTimeoutConstant = 0;


	if(!SetCommTimeouts(hSerial, &ct))
	{
		MessageBox(NULL, "Timeouts parameter error!","Error", MB_OK);
		CloseHandle(hSerial);
		return(0);
	}

	if(!PurgeComm(hSerial, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))
	{
		MessageBox(NULL, "PurgeComm() failed!", "Error", MB_OK);
		CloseHandle(hSerial);
		return(0);
	}

	if(!ClearCommBreak(hSerial))
	{


		MessageBox(NULL, "ClearCommBreak() failed!", "Error", MB_OK);
		CloseHandle(hSerial);
		return(0);
	}




	// ?? ??????? ???????? ???????????, ?????? ?? ????? ?????????, ? ?? ??? ?????? ?????????? ? ??? ?????? ????.

/*	uint8_t cleat_port = 0;
    WriteFile(hSerial, &cleat_port, sizeof(cleat_port), &bc, NULL);
    WriteFile(hSerial, &cleat_port, sizeof(cleat_port), &bc, NULL);
    ReadFile(hSerial, &cleat_port, sizeof(cleat_port), &bc, NULL);
*/

	Reset();
	Sleep(1000);
//	Set_ADC(14, 1, 0);
	Start_ADC(bit14, x1);
	Sleep(200);
	Get_voltage();

	if(adc.meas_status != 0b00000100) // ??? 14 ???.
    {
        MessageBox(NULL, "Connection with periphery is failed", "Error", MB_OK);
        CloseHandle(hSerial);
		return(0);
    }

	return 1;

}


int SENSOR_FET::Reset()
{
   WriteFile(hSerial, &resetSensor, sizeof(resetSensor), &bc, NULL);

   return 1;

}

int SENSOR_FET::Set_voltage(terminal  term, double voltage)
{

    int16_t dac_count;

    if(hSerial == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, "Connection is failed", "Error", MB_OK);
        return 0; //  // messagebox?
    }


    double voltage_gain;
    double voltage_max;
    if(term == GATE)
    {
        voltage_gain = gate_gain;
        voltage_max = voltage_gate_max;
    }
    else
    {
        voltage_gain = drain_gain;
        voltage_max = voltage_drain_max;
    }

    if(voltage > voltage_max)
        voltage = voltage_max;

    if(voltage < -voltage_max)
        voltage = -voltage_max;

 //   if( (voltage <= voltage_dd) && (voltage >= voltage_ss))
 //   {
//     dac_count = (uint16_t) ( ((double) dac_counts) * voltage / (voltage_dd - voltage_ss) ); // ??? ???? ?????????? ?? 0 ?? ~5?
// ? ???? ? ??? ?????? ????????? ??? ?????????? ?? ??? ? ????????? ?? ?????????? ??????? ????, ? ????? ?????????? ?? ????????, ?? ???? ????????? ? ???????? ???????:
//
        dac_count = (int16_t) ( ((double) dac_counts) * 0.5 * ((voltage / voltage_gain) + dac_ref) / dac_ref);
        // ????? ???????? ?? ????????????
        if(dac_count < 0)
            dac_count = 0;

         if(dac_count > dac_counts - 1)
            dac_count = dac_counts - 1;


 //       dac_count = (uint16_t) ( ((double) dac_counts) * voltage / (voltage_dd - voltage_ss) );

//    }
//    else
//    return(0); // messagebox?



    dac_voltage.voltage = dac_count;

    if(term == GATE)
   {
        WriteFile(hSerial, &setDAC_Gate, sizeof(setDAC_Gate), &bc, NULL);
        WriteFile(hSerial, &dac_voltage.volt_1, sizeof(dac_voltage.volt_1), &bc, NULL);
        WriteFile(hSerial, &dac_voltage.volt_0, sizeof(dac_voltage.volt_0), &bc, NULL);
    }
    else
    {
        WriteFile(hSerial, &setDAC_Drain, sizeof(setDAC_Drain), &bc, NULL);
        WriteFile(hSerial, &dac_voltage.volt_1, sizeof(dac_voltage.volt_1), &bc, NULL);
        WriteFile(hSerial, &dac_voltage.volt_0, sizeof(dac_voltage.volt_0), &bc, NULL);

    }


    return 1;

}

/*
int SENSOR_FET::Set_ADC(int bits, int gain = 1, int channel = 0) // or port number, number or ADC and gain (port, 1-4 and gain)... ????? - ???? (0-4), ??????????? (12-18), ???????? (1-8).
{
    // ??????????? 12-14-16-18 -> ???????? 12 ? ????? ?? 3, ???????? 0...3, ??? ????
    // ???????? 1-2-4-8 -> ...
    // ????? ?????? - ??? ????, 0...3

    int8_t bits_channel = channel;
    int8_t bits_bits = (bits - 12)/2;
    int8_t bits_gain = 0;

    if(hSerial == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, "Connection is failed", "Error", MB_OK);
        return 0; //  // messagebox?
    }

    switch(gain)
    {
        case 1:
            bits_gain = 0;
            break;
        case 2:
            bits_gain = 1;
            break;
        case 4:
            bits_gain = 2;
            break;
        case 8:
            bits_gain = 3;
            break;
        default:
            break;

    }


    uint8_t status = 0b10000000 + 32 * bits_channel + 4 * bits_bits + bits_gain;
    // 12-14-16-18 ??? ??? b10000000-b10000100-b10001000-b10001100 (x80/x84/x88/x8C)

    WriteFile(hSerial, &setADC, sizeof(setADC), &bc, NULL);
    WriteFile(hSerial, &status, sizeof(status), &bc, NULL);

    return 1;

}
*/



void SENSOR_FET::Start_ADC(resolution bits,  gain gain_x, int channel)
{

// ??????????, ?.?. ?????? ????? ????-?? ???????? ? ?????? ?????????? ??????????, ? ??? ???? ??????? ????????.

	if(!PurgeComm(hSerial, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))
	{
		MessageBox(NULL, "PurgeComm() failed!", "Error", MB_OK);
		CloseHandle(hSerial);
		return;
	}


    // ??????????? 12-14-16-18 -> ???????? 12 ? ????? ?? 3, ???????? 0...3, ??? ????
    // ???????? 1-2-4-8 -> ...
    // ????? ?????? - ??? ????, 0...3

    if(hSerial == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, "Connection is failed", "Error", MB_OK);
        return; //  // messagebox?
    }

    current_res = bits;
    current_gain = gain_x;

    status = 0b10000000 + 32 * channel + 4 * bits + gain_x;
    // 12-14-16-18 ??? ??? b10000000-b10000100-b10001000-b10001100 (x80/x84/x88/x8C)

    WriteFile(hSerial, &setADC, sizeof(setADC), &bc, NULL);
    WriteFile(hSerial, &status, sizeof(status), &bc, NULL);

}

double SENSOR_FET::Get_voltage()
{

//    std::cout << "We are going to get ADC data" << std::endl;

    if(hSerial == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, "Connection is failed", "Error", MB_OK);
        return 0; //  // messagebox?
    }

    uint8_t expected_status = status - 0b10000000;

    do
    {

        WriteFile(hSerial, &getADC, sizeof(getADC), &bc, NULL);

        // ReadFile(hSerial, &adc18, sizeof(adc18), &bc, NULL);
        if(current_res == bit18)
        {
            ReadFile(hSerial, &adc.meas_2, 1, &bc, NULL);
        }
        else
        {
            adc.meas_2 = 0;
        }

        ReadFile(hSerial, &adc.meas_1, 1, &bc, NULL);
        ReadFile(hSerial, &adc.meas_0, 1, &bc, NULL);
        ReadFile(hSerial, &adc.meas_status, 1, &bc, NULL);


        // ???? ???? ?????????????? ?????????? ? ????? ??????? - ?? ??? ??????????? ????????, ?? ???????, ?? ??? ???? ??? ???????? ?????? 10 (!)
        if(adc.meas_status  != expected_status)
        {
            if(!PurgeComm(hSerial, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))
            {
                MessageBox(NULL, "PurgeComm() failed!", "Error", MB_OK);
 //                  CloseHandle(hSerial);
                return 0;
            }

            Reset();

 //           wxYield();
            Beep(523,50);

            WriteFile(hSerial, &setADC, sizeof(setADC), &bc, NULL);
            WriteFile(hSerial, &status, sizeof(status), &bc, NULL);
        // ?? ???? ??????-?? ????? ????????, ? ?????? ?? ??????????.



        }
    }
    while(adc.meas_status  != expected_status);



// both corrections equal 0 for 18 bit and x1 gain.  But smth is wrong exactly with 0.
// may be better without static_cast and 0 power.
    double adc_correction = std::pow(2.0, double(( 3 - static_cast<int>(current_res)) * 2)); // reduction of  ADC counts for 12-14-16 bit (to 18 bit)
    double gain_correction = std::pow(2.0, double(static_cast<int>(current_gain))); // embedded ADC gain - 1-2-4-8.




    int raw_adc;
    if(current_res == bit18)
    {
        if(adc.meas_2 > 0x7F)
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


// ??? ????????? ?? ???????? ????, ?????? ????????? ????????? ???:
    double result = adc_ref * (double)raw_adc / (1 * ((double) adc_counts / adc_correction)); // ???????? ??? ????????? ?? ???????? ,? ????? ??? ? ???????? ?????....
//    double result = adc_ref * ( 0x10000 * adc.meas_2 + 0x100 * adc.meas_1 + adc.meas_0) / (gain_correction * ((double) adc_counts / adc_correction));

   return double(result);

}

double SENSOR_FET::Get_current() // mA
{
    double voltage = Get_voltage();
//    double current = drain_zero_current + 1000 * ( ( voltage - adc_ref / 2.0) / r_shunt) / ( 2.0 * drain_current_gain ) ;
//    double current =  1000 * ( voltage / r_shunt) / ( drain_detection_gain ) - drain_zero_current; // for high-side sensing
    double current =  1000 * ( voltage ) / ( drain_detection_gain ) - drain_zero_current;

    return current; // mA
}

/*
double SENSOR_FET::Get_Vdd()
{
    return voltage_dd;
}

double SENSOR_FET::Get_Vss()
{
    return voltage_ss;
}

int SENSOR_FET::Set_Vdd(double Vdd)
{
    voltage_dd = Vdd;
    return 1;
}

int SENSOR_FET::Set_Vss(double Vss)
{
    voltage_ss = Vss;
    return 1;
}
*/

double SENSOR_FET::Get_voltage_drain_max()
{
    return voltage_drain_max;
}

double SENSOR_FET::Get_voltage_gate_max()
{
    return voltage_gate_max;
}





int SENSOR_FET::Set_dac_ref(double reference_V)
{
    dac_ref = reference_V;

    return 1;
}

int SENSOR_FET::Set_adc_ref(double reference_V)
{
    adc_ref = reference_V;

    return 1;
}

double SENSOR_FET::Get_adc_ref()
{
    return adc_ref;
}

double SENSOR_FET::Get_dac_ref()
{
    return dac_ref;
}


int SENSOR_FET::CheckState() // ?????????? ??/???, ? ??????????? ?? ????????? ???????????.
{
	if(hSerial == INVALID_HANDLE_VALUE) // ?.?. ??????? ?? ???????????.
	return 0;
	else
	return 1;
}

void SENSOR_FET::Close()
{
	if(hSerial != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hSerial); // ?????? ?? ???????????... - ??? ?????????? ?? ??????????.
		hSerial = INVALID_HANDLE_VALUE;
	}
}

int SENSOR_FET::GetPortNumber()
{
    return com_port;
}

/*
void SENSOR_FET::SetPortNumber(int port_number)
{
	if((port_number > 0) && (port_number < 10) && (hSerial == INVALID_HANDLE_VALUE)) //  ?????? ???? ???????????? ????????
 	com_port = port_number;
}

*/
/*
void SENSOR_FET::Set_ADC(resolution set_res, gain set_gain)
{
    ADC_res = set_res;
    ADC_amp = set_gain;
}
*/
void SENSOR_FET::Set_shunt(double shunt)
{
    if( (shunt > 0) && (shunt < 10))
    r_shunt = shunt;
    else
    Beep(223, 50);


}

double SENSOR_FET::Get_shunt()
{

    return r_shunt;
}

void SENSOR_FET::Set_zero_current(double current)
{
    if( (current > -100) && (current < 100))
    drain_zero_current = current;
    else
    Beep(223, 50);

}

double SENSOR_FET::Get_zero_current()
{
    return drain_zero_current;
}

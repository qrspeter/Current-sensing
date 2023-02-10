/**
 *  библиотека функций работы с сенсором.
 *  Устанавливает напряжение двух ЦАП (затвор и сток), конвертируя напряжение double в отсчеты ЦАП.
 *  Запускает АЦП и задает его настройки (выбор АЦП, разрядность, усиление)
 *  Считывает отсчеты АЦП и конвертирует их в напряжение (хотя нам нужен будет ток).
 *  Включает-выключает лазер через цифровой вывод Ардуино.
 *  А также устанавливает константы пересчета отсчетов в напряжение и обратно.
 *
 */




//#include <iostream>
//#include <wx/filename.h> // для wxYield

#include <windows.h>
#include <string>
#include <cmath> // pow(a, b) = ab, std::nan, std::isnan

#include "sensor_fet.h"


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

    com_name[3] = static_cast<char>(port + 0x30);

    hSerial = CreateFileA((LPCSTR)com_name, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);

	if(hSerial == INVALID_HANDLE_VALUE) // проверка выделения порта. NULL for 32-bit Win and INVALID_HANDLE_VALUE for 64bit
	{
	    if(GetLastError() == ERROR_FILE_NOT_FOUND)
        MessageBox(NULL, "COM port is not found", "Error", MB_OK);
        else
		MessageBox(NULL, "Error Open COM", "Error", MB_OK);
		return(0);
	}

	SetupComm(hSerial, INPUT_BUFF_SIZE, INPUT_BUFF_SIZE); // задание размеров буфера порта.


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
	dcb.fDtrControl = DTR_CONTROL_ENABLE; // иначе работает только после включения терминала порта, идея отсюда https://github.com/dmicha16/simple_serial_port/blob/master/simple-serial-port/simple-serial-port/SimpleSerial.cpp
	// хотя странно, параметр отвечает за сброс при включении
	// DTR_CONTROL_DISABLE; // disable DTR to avoid reset SetCommState(m_hCom, &dcb);  https://forum.arduino.cc/t/disable-auto-reset-by-serial-connection/28248/12
	// хотя ... это "Data terminal ready" https://docs.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-dcb

    // Установка параметры порта.
	if(!SetCommState(hSerial, &dcb))
	{
		MessageBox(NULL,"Port parameter error!", "Error", MB_OK);
		CloseHandle(hSerial);
		return(0);
	}

	COMMTIMEOUTS  ct;

	ct.ReadIntervalTimeout = 50 ; // 3000;//MAXDWORD;
	ct.ReadTotalTimeoutMultiplier = 50; //MAXDWORD; // 3000;// MAXDWORD;
	ct.ReadTotalTimeoutConstant = 300; // 2000; //2000;//100;
	// was MAXDWORD/MAXDWORD/2000, it means "take bytes what we have, do not wait for the other bytes"
	// now 50 (max between bytes) / 50 (max 50 for each byte) / 300 (total time is slightly above over 267ms for 18-bit mode)

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



	Reset();
	Sleep(1000);
//	Set_ADC(14, 1, 0);
	Start_ADC(bit14, x1);
	Sleep(200);

	if(std::isnan(Get_voltage()))
    {
        MessageBox(NULL, "Connection with periphery is failed. Check analog power", "Error", MB_OK);
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

    int16_t dac_count{};

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

    dac_count = static_cast<int16_t> (static_cast<double> (dac_counts) * 0.5 * ((voltage / voltage_gain) + dac_ref) / dac_ref);
    // нужна проверка на переполнение
    if(dac_count < 0)
        dac_count = 0;

    if(dac_count > dac_counts - 1)
        dac_count = dac_counts - 1;



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



void SENSOR_FET::Start_ADC(resolution bits,  gain gain_x, int channel)
{

// сбрасываем, т.к. иногда байты куда-то теряются и скачки происходят ступенькой, а так хоть разовым выбросом.

	if(!PurgeComm(hSerial, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))
	{
		MessageBox(NULL, "PurgeComm() failed!", "Error", MB_OK);
		CloseHandle(hSerial);
		return;
	}


    // разрядность 12-14-16-18 -> вычитаем 12 и делим на 3, получаем 0...3, как надо
    // усиление 1-2-4-8 -> ...
    // номер канала - как есть, 0...3

    if(hSerial == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, "Connection is failed", "Error", MB_OK);
        return; //  // messagebox?
    }

    current_res = bits;
    current_gain = gain_x;

    status = 0b10000000 + 32 * channel + 4 * bits + gain_x;
    // 12-14-16-18 бит это b10000000-b10000100-b10001000-b10001100 (x80/x84/x88/x8C)

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

    int reRead {0};
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

        if(adc.meas_status  == expected_status)
            break;

        if(reRead > 1)
            return std::nan("");

        Beep(523,50);
        // если сбой сопровождается изменением в байте статуса - то АЦП запускается повторно, со сбросом, тк без него это занимает секунд 10 (!)
        if(!PurgeComm(hSerial, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))
        {
            MessageBox(NULL, "PurgeComm() failed!", "Error", MB_OK);
 //                  CloseHandle(hSerial);
            return 0;
        }

        Reset();

 //           wxYield();

        WriteFile(hSerial, &setADC, sizeof(setADC), &bc, NULL);
        WriteFile(hSerial, &status, sizeof(status), &bc, NULL);
        // но пока почему-то прога зависает, а окошко не появляется.
        ++reRead;

    }
    while(adc.meas_status  != expected_status); // а зачем дважды проверять? аналогичная проверка выше идет ведь...


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


// Без коррекции на смещение нуля, только результат измерения АЦП:
    double result = adc_ref * static_cast<double>(raw_adc) / (1 * (static_cast<double> (adc_counts) / adc_correction)); // временно без коррекции на усиление ,а может так и понятнее будет....
//    double result = adc_ref * ( 0x10000 * adc.meas_2 + 0x100 * adc.meas_1 + adc.meas_0) / (gain_correction * ((double) adc_counts / adc_correction));

    return double(result);

}

double SENSOR_FET::Get_current() // mA
{
    double voltage = Get_voltage();

    if(std::isnan(voltage))
    return std::nan("");

    double current =  1000 * ( voltage ) / ( drain_detection_gain ) - drain_zero_current;
//    double current = drain_zero_current + 1000 * ( ( voltage - adc_ref / 2.0) / r_shunt) / ( 2.0 * drain_current_gain ) ;
//    double current =  1000 * ( voltage / r_shunt) / ( drain_detection_gain ) - drain_zero_current; // for high-side sensing

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


int SENSOR_FET::CheckState() // Возвращает да/нет, в зависимости от состояния подключения.
{
	if(hSerial == INVALID_HANDLE_VALUE) // Т.е. счетчик не подключался.
        return 0;
	else
        return 1;
}

void SENSOR_FET::Close()
{
	if(hSerial != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hSerial); // похоже не срабатывает... - при отключении не сбрасывает.
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
	if((port_number > 0) && (port_number < 10) && (hSerial == INVALID_HANDLE_VALUE)) //  только если монохроматор отключен
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

void SENSOR_FET::Laser(laser las)
{
    if(las == LASER_ON)
        WriteFile(hSerial, &setLaser_On, sizeof(setLaser_On), &bc, NULL);
    else
        WriteFile(hSerial, &setLaser_Off, sizeof(setLaser_Off), &bc, NULL);

}

void SENSOR_FET::setPulseDuraion(double duration)
{
    pulse_duration = duration;
}

void SENSOR_FET::setPulseNumbers(int numbers)
{
    pulse_numbers = numbers;
}

double SENSOR_FET::getPulseDuraion()
{
    return pulse_duration;
}

int SENSOR_FET::getPulseNumbers()
{
    return pulse_numbers;
}


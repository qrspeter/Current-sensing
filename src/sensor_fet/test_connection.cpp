#include <iostream>
#include <windows.h>
#include <string>

#include "sensor_fet.h"


int main()
{
    SENSOR_FET sensor;

    if(!sensor.Open(3))
	{
		std::cout << "Device is not connected"  << std::endl;
		return 0;
	}
	std::cout << "Device is connected"  << std::endl;

    std::string line;

    while(1)
    {
        std::cout << "Give a command! 0 - Reset, 1 - setDAC_Gate, 2 - setDAC_Drain, 3 - get Voltage, 4 - get Current, 10 - exit, >10 - check" << std::endl;
        int operand_code;
        std::getline(std::cin, line);
        operand_code = std::stoi(line);

           switch(operand_code)
            {
                case 0:
                sensor.Reset();
                break;

                case 1:

                    std::cout << "Give the Gate voltage in +/-" << sensor.GetGateLimit() << " V range" << std::endl;

                    std::getline(std::cin, line);
                    std::cout << "Voltage = " << std::stod(line) << "V" << std::endl;

                    sensor.Set_voltage(SENSOR_FET::GATE, std::stod(line));

                break;

                case 2:

                    std::cout << "Give the Drain voltage in +/-" << sensor.GetDrainLimit() << " V range" << std::endl;

                    std::getline(std::cin, line);
                    std::cout << "Voltage = " << std::stod(line) << "V" << std::endl;

                    sensor.Set_voltage(SENSOR_FET::DRAIN, std::stod(line));


                break;

                case 3:
                    sensor.Start_ADC(SENSOR_FET::bit18, SENSOR_FET::x1, SENSOR_FET::DRAIN);
					Sleep(1000);

                    std::cout << "Voltage is " << sensor.Get_voltage() << " V"  << std::endl;

                break;

                case 4:
                    sensor.Start_ADC(SENSOR_FET::bit18, SENSOR_FET::x1, SENSOR_FET::DRAIN);
					Sleep(1000);

                    std::cout << "Current is " << sensor.Get_current() << " mA" << std::endl;

                break;

                case 10:
                    exit(0);

                break;

                default:
                if(sensor.Check())
                    std::cout << "Device is connected" << std::endl;
                else
                    std::cout << "Device is not connected" << std::endl;

            }

    }


    return 0;
}

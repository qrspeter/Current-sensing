#include <iostream>
#include <windows.h>
#include <string>
#include<vector>

#include "sensor_fet.h"

SENSOR_FET sensor;

std::vector<std::string> SerialList()
{
    std::vector<std::string> serialList;
    std::string COMName("COM"), queryName("");
    char bufferTragetPath[5000];
    long path_size{0};

    //test each COM name to get the one used by the system and get his description name
    for (int i{1}; i < 24; i++)
    {
        queryName = COMName + std::to_string(i);

        //Query the path of the COMName
        path_size = QueryDosDeviceA(queryName.c_str(), bufferTragetPath, 5000);
        //std::cout << std::endl << "Path for " << queryName << ":" << path_size << "   " << queryName;
        if (path_size != 0) {
            std::cout << queryName << " on " << bufferTragetPath << std::endl;
            serialList.push_back(queryName);
        }
    }
    return serialList;
}

int connect(std::string port)
{
    if(!sensor.Open(port))
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
	return 1;
}


int main()
{

	int port_no{0};
	auto ports = SerialList();
	std::cout << "Availible ports: " << '\n';
	for (auto port: ports)
	{
		std::cout << port << '\n';
		//port_no = std::stoi(port.substr(3)); // "COM" + port_no
		if(connect(port) == 1)
			break;

	}
	if(!port_no)
		std::cout << "Availible ports are not found" << std::endl;

    return 0;
}

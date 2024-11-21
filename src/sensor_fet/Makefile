# in Windows with mingw use: mingw32-make -f Makefile
default:
	g++ -c -Wall sensor_fet.cpp test_connection.cpp
	g++ -o test_connection.exe sensor_fet.o test_connection.o
	del -f rm *.o


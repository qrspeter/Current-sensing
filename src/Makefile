# mingw32-make -f Makefile 

# https://wiki.codeblocks.org/index.php/Code::Blocks_and_Makefiles
# works for MinGW 13.2.0 and does not work in MinGW 13.2.0: 
# obj\Debug\current_sensingMain.o: in function `void std::this_thread::sleep_for<long long, std::ratio<1ll, 1000000000ll> >(std::chrono::duration<long long, std::ratio<1ll, 1000000000ll> > const&)':
#C:/Program Files/CodeBlocks/MinGW/lib/gcc/x86_64-w64-mingw32/8.1.0/include/c++/thread:369: undefined reference to `nanosleep'

# https://stackoverflow.com/questions/4438084/stdthis-threadsleep-for-and-gcc
# Need to define _GLIBCXX_USE_NANOSLEEP on top of the source code.
#  #define _GLIBCXX_USE_NANOSLEEP  //add it top of c++ code
# OR, Compile with following command:
# g++ a.cpp -o a -std=c++0x -D_GLIBCXX_USE_NANOSLEEP

# Project name and version
Proj := current_sensing
Version := Debug

#paths for Project (Ppath) Object files (Opath) and binary path (Bpath)
Ppath := E:\programming\FET_sensor\Current-sensing\src
# Ppath := E:\programming\FET_sensor\$(Proj) # if Current_sensing -> current_sensing
Opath := obj\$(Version)
Bpath := bin\$(Version)

#Library & header paths
WXpath := C:\wxwidgets
IncWX := $(WXpath)\include
IncMSW := $(WXpath)\lib\gcc_dll\mswu
Libpath := $(WXpath)\lib\gcc_dll

flags = -pipe -mthreads -D__GNUWIN32__ -D__WXMSW__ -DWXUSINGDLL -DwxUSE_UNICODE -Wall -g 

CXX = C:\Program Files\CodeBlocks\MinGW\bin\g++.exe
#CXX = g++


Obj := $(Opath)\$(Proj)App.o $(Opath)\$(Proj)Main.o $(Opath)\mathplot\mathplot.o $(Opath)\mathplot\MathPlotConfig.o $(Opath)\sensor_fet\sensor_fet.o $(Opath)\resource.0

$(Proj).exe : $(Obj)
	$(CXX) -L$(Libpath) -o $(Bpath)\$(Proj).exe $(Obj) -lwxmsw31u -mwindows

$(Opath)\$(Proj)Main.o : $(Ppath)\$(Proj)Main.cpp
	$(CXX) $(flags) -I$(IncWX) -I$(IncMSW) -c $^ -o $@

$(Opath)\$(Proj)App.o : $(Ppath)\$(Proj)App.cpp
	$(CXX) $(flags) -I$(IncWX) -I$(IncMSW) -c $^ -o $@

$(Opath)\mathplot\mathplot.o : $(Ppath)\mathplot\mathplot.cpp
	$(CXX) $(flags) -I$(IncWX) -I$(IncMSW) -c $^ -o $@

$(Opath)\mathplot\MathPlotConfig.o : $(Ppath)\mathplot\MathPlotConfig.cpp
	$(CXX) $(flags) -I$(IncWX) -I$(IncMSW) -c $^ -o $@

$(Opath)\sensor_fet\sensor_fet.o : $(Ppath)\sensor_fet\sensor_fet.cpp
	$(CXX) $(flags) -I$(IncWX) -I$(IncMSW) -c $^ -o $@

$(Opath)\resource.0 : $(Ppath)\resource.rc
	windres -I$(IncWX) -I$(IncMSW) -i$^ -o $@

.PHONEY : clean

clean:
	del $(Bpath)\$(Proj).exe $(Obj) $(Opath)\resource.0

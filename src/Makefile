# in Win: mingw32-make -f Makefile 

# Project name and version
Proj := current_sensing
Version := Debug
#Version := Release

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
#flags = -pipe -mthreads -D__GNUWIN32__ -D__WXMSW__ -DWXUSINGDLL -DwxUSE_UNICODE -Wall -O1 

#CXX = C:\Program Files\CodeBlocks\MinGW\bin\g++.exe
CXX = g++


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

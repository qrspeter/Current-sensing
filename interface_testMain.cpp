// идеи:
// еще неплохо бы в лестничном режиме выводить текущее напряжение на экран. Ну или просто выделить два окошка текущего напряжения.

// добавить управление лазером, чтобы включать периодически, и при этом вести регистрацию и всего сигнала, и разностного, может строя их на одном графике... Или выводя тот или иной, по выборку кнопкой.
// Тогда надо еще вкладку создать, а то места уже нет.

/***************************************************************
 * Name:      interface_testMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Peter ()
 * Created:   2022-03-14
 * Copyright: Peter ()
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif


#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "interface_testMain.h"

#include <wx/filename.h>
#include <wx/progdlg.h> // https://docs.wxwidgets.org/trunk/classwx_progress_dialog.html или https://flylib.com/books/en/3.138.1.74/1/

#include <vector>
#include <fstream>
#include <chrono>


SENSOR_FET sensor;

std::vector <double> I_data;
std::vector <double> V_data;
std::vector <double> T_data;

const wxString ports[] = { wxT("COM1"), wxT("COM2"), wxT("COM3"), wxT("COM4"), wxT("COM5"), wxT("COM6"), wxT("COM7"), wxT("COM8"), wxT("COM9")};
const wxString adc_resolution[] = { wxT("12 bit"), wxT("14 bit"), wxT("16 bit"), wxT("18 bit")};
const wxString adc_gain[] = { wxT("1x"), wxT("2x"), wxT("4x"), wxT("8x")};
const wxString iv_modes[] = { wxT("Ids(Uds)"), wxT("Ids(Ugs)")};
const wxString transient_modes[] = { wxT("Adjustment"), wxT("Lagger")};


bool measurementStop{FALSE}; // Flag

//int screen_elements = 300;

//int ret_code;

// https://stackoverflow.com/questions/2808398/easily-measure-elapsed-time
template <
    class result_t   = std::chrono::milliseconds,
    class clock_t    = std::chrono::steady_clock,
    class duration_t = std::chrono::milliseconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}



bool compLess(int a, int b) // for minmax_element
{
    return (a < b);
}

const int sleep_time[4]{ 5, 17, 67, 267 }; // 12bit = 1000/240 = 5, 14bit = 1000/60 = 17, 16bit = 1000/15 = 67, 18bit = 1000/3.75 = 267

const double gate_voltage_max{40.0}; // voltage_max
const double drain_voltage_max{12.0}; // voltage_min


//extern enum terminal { DRAIN, GATE };

//extern enum resolution { bit12 = 0, bit14 = 1, bit16 = 2, bit18 = 3};


//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__WXMAC__)
        wxbuild << _T("-Mac");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

BEGIN_EVENT_TABLE(interface_testFrame, wxFrame)
    EVT_CLOSE(interface_testFrame::OnClose)
    EVT_MENU(idMenuSaveAs, interface_testFrame::OnSaveAs)
    EVT_MENU(idMenuAbout, interface_testFrame::OnAbout)
    EVT_MENU(idMenuQuit, interface_testFrame::OnQuit)

    EVT_MENU(idMenuSettingSave, interface_testFrame::OnSettingSave)
    EVT_MENU(idMenuSettingChange, interface_testFrame::OnSettingChange)
    EVT_MENU(idMenuSettingDefault, interface_testFrame::OnSettingDefault)

    EVT_BUTTON(idSensor_connect, interface_testFrame::Sensor_connect)

    EVT_BUTTON(idIV_meas_start, interface_testFrame::Meas_start)
    EVT_BUTTON(idIV_meas_stop, interface_testFrame::Meas_stop)

    EVT_RADIOBOX(idIV_modes, interface_testFrame::IV_modes)

    EVT_BUTTON(idTransient_start, interface_testFrame::Transient_start)
    EVT_BUTTON(idTransient_stop, interface_testFrame::Transient_stop)

    EVT_RADIOBOX(idTransient_modes, interface_testFrame::Transient_modes)

    // проверить какое из них работает, так то нужна всего одна
//    EVT_BUTTON(idSetting_OK, interface_testFrame::Setting_OK)
//    EVT_BUTTON(wxID_OK, interface_testFrame::Setting_OK)



END_EVENT_TABLE()



interface_testFrame::interface_testFrame(wxFrame *frame, const wxString& title)
    : wxFrame(frame, -1, title)
{
#if wxUSE_MENUS
    // create a menu bar
    wxMenuBar* mbar = new wxMenuBar();
    wxMenu* fileMenu = new wxMenu(_T(""));
    fileMenu->Append(idMenuSaveAs, _("&Save as...\tCtrl-s"), _("Save data"));
    fileMenu->Append(idMenuQuit, _("&Quit\tAlt-F4"), _("Quit the application"));
    mbar->Append(fileMenu, _("&File"));

    wxMenu* settingMenu = new wxMenu(_T(""));
    settingMenu->Append(idMenuSettingSave, _("&Save setting\tCtrl-p"), _("Save current setting"));
    settingMenu->Append(idMenuSettingChange, _("&Change settings\tCtrl-a"), _("Change ADC-DAC settings"));
    settingMenu->Append(idMenuSettingDefault, _("&Load default\tCtrl-d"), _("Load default setting"));
    mbar->Append(settingMenu, _("&Setting"));


    wxMenu* helpMenu = new wxMenu(_T(""));
    helpMenu->Append(idMenuAbout, _("&About\tF1"), _("Show info about this application"));
    mbar->Append(helpMenu, _("&Help"));

    SetMenuBar(mbar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // create a status bar with some information about the used wxWidgets version
    CreateStatusBar(2);
    SetStatusText(_("Hello Code::Blocks user!"),0);
    SetStatusText(wxbuildinfo(short_f), 1);
#endif // wxUSE_STATUSBAR



    wxPanel *framework_panel = new wxPanel(this, -1);

    wxBoxSizer *framework_sizer = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer *control_sizer   = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer *wxSensor          = new wxStaticBoxSizer(wxVERTICAL, framework_panel, wxT("Sensor"));
    wxStaticBoxSizer *wxIV              = new wxStaticBoxSizer(wxVERTICAL, framework_panel, wxT("IV"));
 //   wxStaticBoxSizer *wxTransfer        = new wxStaticBoxSizer(wxVERTICAL, framework_panel, wxT("Transfer Ids(Vgs)"));
    wxStaticBoxSizer *wxTransient       = new wxStaticBoxSizer(wxVERTICAL, framework_panel, wxT("Transient Ids(t)"));

    control_sizer -> Add(wxSensor,         0, wxEXPAND, 0);
    control_sizer -> Add(new wxStaticText(framework_panel, -1, wxT("")), 0, wxEXPAND);

    IV_mode = new wxRadioBox(framework_panel, idIV_modes, wxT("IV mode"), wxDefaultPosition, wxDefaultSize, 2, iv_modes, 0);
    control_sizer -> Add(IV_mode, 0, wxCENTER);

//    control_sizer -> Add(new wxStaticText(framework_panel, -1, wxT("")), 0, wxEXPAND);

    control_sizer -> Add(wxIV,   0, wxEXPAND, 0);

    IV_meas_start = new wxButton(framework_panel, idIV_meas_start, wxT("Start"));
    control_sizer -> Add(IV_meas_start, 0, wxCENTER); //  | wxEXPAND
    IV_meas_stop = new wxButton(framework_panel, idIV_meas_stop, wxT("Stop"));
    control_sizer -> Add(IV_meas_stop, 0, wxCENTER); //  | wxEXPAND

    control_sizer -> Add(new wxStaticText(framework_panel, -1, wxT("")), 0, wxEXPAND);


    transient_mode = new wxRadioBox(framework_panel, idTransient_modes, wxT("Transient mode"), wxDefaultPosition, wxDefaultSize, 2, transient_modes, 0);
    control_sizer -> Add(transient_mode, 0, wxCENTER);

    control_sizer -> Add(wxTransient,      0, wxEXPAND, 0);
    transient_meas_start = new wxButton(framework_panel, idTransient_start, wxT("Start"));
    control_sizer -> Add(transient_meas_start, 0, wxCENTER); //  | wxEXPAND
    transient_meas_stop = new wxButton(framework_panel, idTransient_stop, wxT("Stop"));
    control_sizer -> Add(transient_meas_stop, 0, wxCENTER); //  | wxEXPAND

    // idIV_mode

    IV_meas_start -> Disable();
    IV_meas_stop -> Disable();
    transient_meas_start -> Disable();
    transient_meas_stop -> Disable();



// sensor =========================
    wxGridSizer *wxSensorGrid  = new wxGridSizer(4,2,3,3);

    // https://docs.wxwidgets.org/3.0/classwx_choice.html
    sensor_com_choice = new wxChoice(framework_panel, -1, wxDefaultPosition, wxDefaultSize, 9, ports, wxCB_SORT);
    wxSensorGrid -> Add(sensor_com_choice, 0, wxSHAPED);
    sensor_com_connect = new wxButton(framework_panel, idSensor_connect, wxT("Connect"));
    wxSensorGrid -> Add(sensor_com_connect, 0, wxSHAPED);
    wxSensorGrid -> Add(new wxStaticText(framework_panel, -1, wxT("ADC resolution")), 0, wxSHAPED);
    sensor_resolution = new wxChoice(framework_panel, -1, wxDefaultPosition, wxDefaultSize, 4, adc_resolution); //, wxCB_SORT
    wxSensorGrid -> Add(sensor_resolution, 0, wxSHAPED);
    wxSensorGrid -> Add(new wxStaticText(framework_panel, -1, wxT("ADC sensitivity")), 0, wxSHAPED);
    sensor_gain = new wxChoice(framework_panel, -1, wxDefaultPosition, wxDefaultSize, 4, adc_gain); //, wxCB_SORT
    wxSensorGrid -> Add(sensor_gain, 0, wxSHAPED);


    wxSensorGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Delay, s")), 0, wxSHAPED);
    delay_meas =        new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 1000, 0.0, 0.0, wxT("smth"));
    delay_meas -> SetDigits(1);
    delay_meas -> SetIncrement(0.1);
    delay_meas -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxSensorGrid -> Add(delay_meas, 0, wxSHAPED);



    // idSensor_COM idSensor_resolution idSensor_gain


    wxSensor -> Add(wxSensorGrid, 0, wxSHAPED); // wxSHAPED
    sensor_com_choice   -> SetSelection(2);
    sensor_resolution   -> SetSelection(3);
    sensor_gain         -> SetSelection(0);

// IV =======================

    wxGridSizer *wxIVGrid  = new wxGridSizer(4,2,3,3);

    wxIVGrid -> Add(new wxStaticText(framework_panel, -1, wxT("U bias, V")), 0, wxSHAPED);
    IV_bias = new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*gate_voltage_max, gate_voltage_max, 0.0, 0.0, wxT("smth"));
    wxIVGrid -> Add(IV_bias, 0, wxSHAPED);
    IV_bias -> SetDigits(1);
    IV_bias -> SetIncrement(0.1);
    wxIVGrid -> Add(new wxStaticText(framework_panel, -1, wxT("U start, V")), 0, wxSHAPED);
    IV_start = new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*drain_voltage_max, drain_voltage_max, 0.0, 0.0, wxT("smth"));
    IV_start -> SetDigits(1);
    IV_start -> SetIncrement(0.1);
    wxIVGrid -> Add(IV_start, 0, wxSHAPED);
    wxIVGrid -> Add(new wxStaticText(framework_panel, -1, wxT("U stop, V")), 0, wxSHAPED);
    IV_stop = new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*drain_voltage_max, drain_voltage_max, 0.0, 0.0, wxT("smth"));
    IV_stop -> SetDigits(1);
    IV_stop -> SetIncrement(0.1);
    wxIVGrid -> Add(IV_stop, 0, wxSHAPED);
    wxIVGrid -> Add(new wxStaticText(framework_panel, -1, wxT("U step, V")), 0, wxSHAPED);
    IV_step = new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 10.0, 0.1, 0.0, wxT("smth"));
    IV_step -> SetDigits(2);
    IV_step -> SetIncrement(0.05);
    wxIVGrid -> Add(IV_step, 0, wxSHAPED);

    // idIV_bias idIV_start idIV_stop idIV_step

    // wxSpinCtrlDouble example: https://stackoverflow.com/questions/42706594/distinguishing-between-different-wxspinctrldouble-events
    // wxSpinCtrlDouble(frame, wxID_ANY, "1.0", wxPoint(10, 10), wxSize(150, 20), wxSP_ARROW_KEYS | wxALIGN_RIGHT | wxTE_PROCESS_ENTER, 1.0, 20.0, 1.0, 1.0);

    // use SetDigits - https://forums.wxwidgets.org/viewtopic.php?f=1&t=48925&p=210156



    wxIV -> Add(wxIVGrid, 0, wxSHAPED); // wxSHAPED


// Transient =======================

    wxGridSizer *wxTransientGrid  = new wxGridSizer(5,2,3,3);



    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Drain bias, V")), 0, wxSHAPED);
    trans_drain_bias =  new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*drain_voltage_max, drain_voltage_max, 0.0, 0.0, wxT("smth")); //
    trans_drain_bias -> SetDigits(1);
    trans_drain_bias -> SetIncrement(0.1);
    trans_drain_bias -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_drain_bias, 0, wxSHAPED);

/*    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Gate bias, V")), 0, wxSHAPED);
    trans_gate_bias =   new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*gate_voltage_max, gate_voltage_max, 0.0, 0.0, wxT("smth"));
    trans_gate_bias -> SetDigits(1);
    trans_gate_bias -> SetIncrement(0.1);
    trans_gate_bias -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_gate_bias, 0, wxSHAPED);
*/
/*    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Delay, s")), 0, wxSHAPED);
    trans_step =        new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 1000, 0.0, 0.0, wxT("smth"));
    trans_step -> SetDigits(1);
    trans_step -> SetIncrement(0.1);
    trans_step -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_step, 0, wxSHAPED);


*/


    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Gate start, V")), 0, wxSHAPED);
    trans_gate_start =   new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*gate_voltage_max, gate_voltage_max, 0.0, 0.0, wxT("smth"));
    trans_gate_start -> SetDigits(1);
    trans_gate_start -> SetIncrement(0.1);
    trans_gate_start -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_gate_start, 0, wxSHAPED);

    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Gate stop, V")), 0, wxSHAPED);
    trans_gate_stop =   new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*gate_voltage_max, gate_voltage_max, 0.0, 0.0, wxT("smth"));
    trans_gate_stop -> SetDigits(1);
    trans_gate_stop -> SetIncrement(0.1);
    trans_gate_stop -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_gate_stop, 0, wxSHAPED);

    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Gate step, V")), 0, wxSHAPED);
    trans_gate_step =   new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*gate_voltage_max, gate_voltage_max, 1.0, 0.0, wxT("smth"));
    trans_gate_step -> SetDigits(1);
    trans_gate_step -> SetIncrement(0.1);
    trans_gate_step -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_gate_step, 0, wxSHAPED);

    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Interval, s")), 0, wxSHAPED);
    trans_interval =   new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*gate_voltage_max, gate_voltage_max, 10.0, 0.0, wxT("smth"));
    trans_interval -> SetDigits(1);
    trans_interval -> SetIncrement(0.1);
    trans_interval -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_interval, 0, wxSHAPED);



    wxTransient -> Add(wxTransientGrid, 0, wxSHAPED); // wxSHAPED

    // idTrans_drain_bias idTrans_gate_bias idTrans_step

    trans_gate_stop -> Disable();
    trans_gate_step -> Disable();
    trans_interval -> Disable();


// end of control panel ==========================


    framework_sizer->Add(control_sizer, 0, wxSHAPED); // wxEXPAND

    framework_graph = new mpWindow(framework_panel, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    framework_sizer->Add(framework_graph, 1, wxEXPAND);

    framework_panel->SetSizer(framework_sizer);

    Centre();

}





interface_testFrame::~interface_testFrame()
{
    if(sensor.CheckState())
    {
        sensor.Close();
    }
}

void interface_testFrame::OnClose(wxCloseEvent &event)
{
    Destroy();
}

void interface_testFrame::OnQuit(wxCommandEvent &event)
{
    Destroy();
}

void interface_testFrame::OnAbout(wxCommandEvent &event)
{
    wxString msg = wxbuildinfo(long_f);
    wxMessageBox(msg, _("Welcome to..."));
}


void interface_testFrame::OnSaveAs(wxCommandEvent &event)
{
	wxFileDialog *SaveDialog = new wxFileDialog(this, _("Save File As..."), wxEmptyString, wxEmptyString, _("Text files (*.txt)|*.txt|ASCII Files (*.asc)|*.asc"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

	if (SaveDialog->ShowModal() == wxID_OK) // If the user clicked "OK"
	{

        wxString path;
		path.append( SaveDialog->GetDirectory() );
		path.append( wxFileName::GetPathSeparator() );
		path.append( SaveDialog->GetFilename() );

		std::ofstream fsave;
		fsave.open(path, std::ios::out); // (path); //  | ios::binary

        if (fsave.is_open()) // если файл открыт
        {
            fsave << "#Time, sec  \t Voltage, V \t Current, mA" << std::endl; //
            for(unsigned int i = 0; i < I_data.size(); i++) //
            {
                fsave << T_data[i] << "\t" << V_data[i] << "\t" << I_data[i] << std::endl;
            }

            fsave.close();
            SetTitle(wxString("Data saved - ") << SaveDialog->GetFilename());
        }
        else
        {
           wxMessageBox( wxT("Can not open file"), wxT("Save file"), wxOK | wxICON_INFORMATION);

        }



	}

	// Clean up after ourselves
	SaveDialog->Destroy();

 }



void interface_testFrame::Sensor_connect(wxCommandEvent &event)
{

   if(sensor.CheckState())
    {   // already opened
        sensor.Close();
        sensor_com_connect->SetLabel("Connect");
        sensor_com_choice -> Enable();
        IV_meas_start -> Disable();
        transient_meas_start -> Disable();
   }
    else
    { // is not opened yet
//        if(sensor.Open(sensor.GetPortNumber()))
        sensor_com_connect -> Disable();
        if(sensor.Open((sensor_com_choice -> GetSelection()) + 1))
        {

            // change button label to Disconnect
            sensor_com_connect -> Enable();
            sensor_com_connect->SetLabel("Disconnect");
            sensor_com_choice -> Disable();
            IV_meas_start -> Enable();
            transient_meas_start -> Enable();

//            sensor.Set_Vdd(voltage_dd);
//            sensor.Set_Vss(voltage_ss);

        }
        else
        {
            sensor.Close();
            sensor_com_connect -> Enable();
            // nothing to do or  make an error message - currently we already have a lot of MessageBoxes from Open
        }

    }

}

void interface_testFrame::Meas_start(wxCommandEvent &event)
{
    if(!sensor.CheckState())
    {
        wxMessageDialog *dial = new wxMessageDialog (NULL, wxT( "Sensor is not connected" ) , wxT( " Error" ) , wxOK | wxICON_ERROR);
        dial -> ShowModal();
        return;
    }

    SetTitle(wxString("IV measurement – Data not saved"));

    IV_meas_stop -> Enable();
    IV_meas_start -> Disable();
    IV_mode -> Disable();
    transient_meas_start -> Disable();
    measurementStop = FALSE;


    framework_graph -> DelAllLayers(true, true);
    mpInfoCoords *frame_coord = new mpInfoCoords();
    framework_graph->AddLayer(frame_coord);
    mpFXYVector *frameworkVector = new mpFXYVector();
    frameworkVector->SetContinuity(true);
    frameworkVector->SetPen(wxPen(wxColor(0xFF, 0x00, 0x00), 2, wxPENSTYLE_SOLID));


    framework_graph->AddLayer(frameworkVector);

    mpScaleY *scaleY = new mpScaleY(wxT("Current, mA"), mpALIGN_BOTTOM, true);

    mpScaleX *scaleX = new mpScaleX(wxT("Voltage, V"), mpALIGN_LEFT, true);

    framework_graph->AddLayer(scaleX);
    framework_graph->AddLayer(scaleY);

    I_data.resize(0);
    V_data.resize(0);
    T_data.resize(0);

    auto start_time = std::chrono::steady_clock::now();


    // сперва смещение, терминал определяется по состоянию переключателя. Выбор 0 - значит смещение на затвор, а сканирование на сток, 1 - наоборот.
    if(IV_mode -> GetSelection() == 0)
    sensor.Set_voltage(GATE, IV_bias -> GetValue());
    else
    sensor.Set_voltage(DRAIN, IV_bias -> GetValue());

    double start = IV_start -> GetValue();
    double stop = IV_stop -> GetValue();
    double step = IV_step -> GetValue();

    double scanDirection = 1.0;
    if(start > stop)
    {
        scanDirection = -1.0;
    }

    int elements = 1 + static_cast<int> ( scanDirection * (stop - start) / static_cast<double>(step));


//    double edge_x = (stop - start)/50;

    I_data.reserve(elements);
    V_data.reserve(elements);
    T_data.reserve(elements);


    for(double voltage = start; scanDirection * voltage <= scanDirection * stop + 0.0000000001;  voltage += scanDirection * step) // миллиарндая добавлена чтобы компенсировать набегание погрешности и окончания до последней точки
    {

        wxYield();
        if(measurementStop)
        break;

        sensor.Set_voltage(static_cast<terminal>(IV_mode -> GetSelection()) , voltage);

        // тут надо подождать при первом измерении секунду, а то скачок сигнала...
/*        if(voltage == start)
        {
            Sleep(1000);  // костыль, но хоть так!

        }
*/

        // конвертировать меню в тип разрешения?
        resolution res = (resolution) sensor_resolution  -> GetSelection();

        sensor.Start_ADC(res, static_cast<gain>(sensor_gain  -> GetSelection()));

//        Sleep(sleep_time[sensor_resolution  -> GetSelection()]);
        if( (sleep_time[sensor_resolution  -> GetSelection()]) < static_cast<int>(delay_meas -> GetValue()*1000))
        Sleep(static_cast<int>(delay_meas -> GetValue()*1000));
        else
        Sleep(sleep_time[sensor_resolution  -> GetSelection()]);



        V_data.push_back(voltage);
        I_data.push_back(sensor.Get_current()); //  Get_ADC
        T_data.push_back(static_cast<double>(since(start_time).count())/1000.0);
//        T_data.push_back((double)since(start_time).count()/1000.0);


        frameworkVector -> SetData(V_data, I_data);

        framework_graph -> Fit();

// //глючит вертикальное отображение когда кривая не растет, а падает, выглядит нормально если мышкой вызывать Fit...
/*
        auto vMinmax = std::minmax_element(I_data.begin(), I_data.end(), compLess);

        if(start < stop)
        framework_graph -> Fit(start - edge_x, stop + edge_x, *vMinmax.first, *vMinmax.second);
        else
        framework_graph -> Fit();
//        framework_graph -> Fit(stop + edge_x, start - edge_x, *vMinmax.second, *vMinmax.first);

*/


        if(measurementStop)
        break;
    }

    sensor.Set_voltage(GATE, 0);
    sensor.Set_voltage(DRAIN, 0);

    IV_meas_stop -> Disable();
    IV_meas_start -> Enable();
    IV_mode -> Enable();
    transient_meas_start -> Enable();
    measurementStop = FALSE;

}

void interface_testFrame::Meas_stop(wxCommandEvent &event)
{
    IV_meas_stop -> Disable();

    measurementStop = TRUE;
}

void interface_testFrame::Transient_start(wxCommandEvent &event)
{

    if(!sensor.CheckState())
    {
        wxMessageDialog *dial = new wxMessageDialog (NULL, wxT( "Sensor is not connected" ) , wxT( " Error" ) , wxOK | wxICON_ERROR);
        dial -> ShowModal();
        return;
    }
    SetTitle(wxString("Transient measurement – Data not saved"));

    transient_meas_stop -> Enable();
    transient_meas_start -> Disable();
    IV_mode -> Disable();
    IV_meas_start -> Disable();
    measurementStop = FALSE;


    framework_graph -> DelAllLayers(true, true);
    mpInfoCoords *frame_coord = new mpInfoCoords();
    framework_graph -> AddLayer(frame_coord);
    mpFXYVector *frameworkVector = new mpFXYVector();
    frameworkVector -> SetContinuity(false); // true
    frameworkVector -> SetPen(wxPen(wxColor(0xFF, 0x00, 0x00), 5, wxPENSTYLE_SOLID));  // 2 for SetContinuity(true) and 5 for false

    framework_graph -> AddLayer(frameworkVector);

    mpScaleY *scaleY = new mpScaleY(wxT("Current, mA"), mpALIGN_BOTTOM, true);

    mpScaleX *scaleX = new mpScaleX(wxT("Time, sec"), mpALIGN_LEFT, true);

    framework_graph -> AddLayer(scaleX);
    framework_graph -> AddLayer(scaleY);

    frameworkVector -> SetData(V_data, I_data);


 //   double current_time = 0;
    int current_count = 0;

    // or insert to do loop and save to additional vector and data?
//    sensor.Set_voltage(GATE, trans_gate_start -> GetValue());
//    sensor.Set_voltage(DRAIN, trans_drain_bias -> GetValue());

    I_data.resize(0);
    V_data.resize(0);
    T_data.resize(0);

//    double edge_x = screen_elements * trans_step -> GetValue() /50;

    auto start_time = std::chrono::steady_clock::now();
//    double previous_gate = 0.0;
//    double previous_drain = 0.0;

    if(transient_mode -> GetSelection() == 0)
    {
        do
        {
        sensor.Set_voltage(GATE, trans_gate_start -> GetValue());
        sensor.Set_voltage(DRAIN, trans_drain_bias -> GetValue());


        resolution res = (resolution) sensor_resolution -> GetSelection();
        sensor.Start_ADC(res, static_cast<gain>(sensor_gain -> GetSelection()));

        if( (sleep_time[sensor_resolution -> GetSelection()]) < static_cast<int>(delay_meas -> GetValue()*1000))
        Sleep(static_cast<int>(delay_meas -> GetValue()*1000));
        else
        Sleep(sleep_time[sensor_resolution -> GetSelection()]);

        I_data.push_back(sensor.Get_current()); //  Get_ADC
        T_data.push_back(static_cast<double>(since(start_time).count())/1000.0);
        V_data.push_back(trans_gate_start -> GetValue());



        frameworkVector -> SetData(T_data, I_data);
        framework_graph -> Fit();


/*        if(current_count < screen_elements)
        {

            auto vMinmax = std::minmax_element(I_data.begin(), I_data.end(), compLess);
            double min_ = *vMinmax.first;
            double max_ = *vMinmax.second;
            framework_graph -> Fit(-1*edge_x, screen_elements + edge_x, *vMinmax.first, *vMinmax.second);
        }
        else
        {
            // последние 300 только
            auto vMinmax = std::minmax_element(I_data.end() - screen_elements, I_data.end(), compLess);
            framework_graph -> Fit(current_count - screen_elements - edge_x, current_count + edge_x, *vMinmax.first, *vMinmax.second);
        }
*/

        wxYield();

        current_count++;


        }
        while(!measurementStop);

    }
    else
    {

        int current_progress = 0;
        wxProgressDialog *ProgressMeas = new wxProgressDialog (wxT("Progress"), wxT("Measuring..."), 100, NULL, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_CAN_ABORT|wxPD_ELAPSED_TIME|wxPD_ESTIMATED_TIME|wxPD_REMAINING_TIME);


/* https://docs.wxwidgets.org/trunk/classwx_generic_progress_dialog.html
    wxProgressDialog dialog(...);
    for ( int i = 0; i < 100; ++i ) {
        if ( !dialog.Update(i)) {
            // Cancelled by user.
            break;
        }

        ... do something time-consuming (but not too much) ...
    }
*/

/* or I will create a thread then. and how should I create dialogue on stack?
Just create it as wxProgressDialog dlg("Heading", "Message", max) instead of using new to create it on the heap.

*/

        double start    = trans_gate_start -> GetValue();
        double stop     = trans_gate_stop -> GetValue();
        double step     = trans_gate_step -> GetValue();
        double interval = trans_interval -> GetValue();

        double scanDirection = 1.0;
        if(start > stop)
        {
            scanDirection = -1.0;
        }
//        double edge_x = (stop - start)/50;

        sensor.Set_voltage(DRAIN, trans_drain_bias -> GetValue());

        for(double voltage = start; scanDirection * voltage <= scanDirection * stop + 0.0000000001;  voltage += scanDirection * step) // миллиарндая добавлена чтобы компенсировать набегание погрешности и окончания до последней точки
        {
          // Stop using wxProgressDialog now
          //  if(measurementStop)
          //  break;


            // конвертировать меню в тип разрешения?
            resolution res = (resolution) sensor_resolution  -> GetSelection();

            auto start_interval = std::chrono::steady_clock::now();
            do
            {

                sensor.Start_ADC(res, static_cast<gain>(sensor_gain  -> GetSelection()));
                sensor.Set_voltage(GATE, voltage);

        //        Sleep(sleep_time[sensor_resolution  -> GetSelection()]);
                if( (sleep_time[sensor_resolution  -> GetSelection()]) < static_cast<int> (delay_meas -> GetValue()*1000))
                Sleep(static_cast<int>(delay_meas -> GetValue()*1000));
                else
                Sleep(sleep_time[sensor_resolution  -> GetSelection()]);


                V_data.push_back(voltage);
                I_data.push_back(sensor.Get_current()); //  Get_ADC
                T_data.push_back(static_cast<double>(since(start_time).count())/1000.0);


                frameworkVector -> SetData(T_data, I_data);
                framework_graph -> Fit();

                wxYield();

            }
            while(static_cast<double>(since(start_interval).count())/1000.0 < interval);

            current_progress = static_cast<int> (100.0 * voltage * scanDirection / (stop - start));
            if ( !ProgressMeas -> Update(current_progress))
            {
                delete ProgressMeas;
                break;

            }


        }

    }



    sensor.Set_voltage(GATE, 0);
    sensor.Set_voltage(DRAIN, 0);




    SetTitle(wxString("IV measure – Data not saved"));
    transient_meas_stop -> Disable();
    transient_meas_start -> Enable();
    IV_mode -> Enable();
    IV_meas_start -> Enable();
    measurementStop = FALSE;


}
void interface_testFrame::Transient_stop(wxCommandEvent &event)
{
    transient_meas_stop -> Disable();

    measurementStop = TRUE;
}

void interface_testFrame::OnSettingChange(wxCommandEvent &event)
{

    // запрашивать результаты измерений выходного напряжения (тестером), например напряжение при задании на АЦП 0x000000, 0x888888 и 0xFFFFFF. И затем пересчет уже по этим данным
    // то есть если сейчас расчет идет по двум значениям (мин и макс), то там еще и ноль. И они также хранятся в виде данных класса, и меняются при настройке.
    // может начать пока с мин и макс, а ноль - если потребуется. Тогда надо будет отдельную функцию писать расчета величины, которая бы проверяла больше или меньше нуля.

    // и м.б. также калибровать АЦП, чтобы при нуле он показывал ноль.


// https://docs.wxwidgets.org/2.8.8/wx_wxdialog.html#wxdialog
    setting_change = new wxDialog(this, -1, _("Setting"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, "dialogBox");



    wxPanel *setting_panel = new wxPanel(setting_change, -1);

    wxBoxSizer *setting_sizer = new wxBoxSizer( wxVERTICAL );

    wxGridSizer *adjustment_sizer = new wxGridSizer(5,2,3,3); //     wxGridSizer *wxIVGrid  = new wxGridSizer(4,2,3,3);
    wxGridSizer *button_sizer = new wxGridSizer(1,2,3,3);




    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("DAC ref, V")), 0, wxSHAPED);
    wxSpinCtrlDouble *dac_ref_voltage = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, drain_voltage_max, sensor.Get_dac_ref(), 0.0, wxT("smth"));
    adjustment_sizer -> Add(dac_ref_voltage, 0, wxSHAPED);
    dac_ref_voltage -> SetDigits(2);
    dac_ref_voltage -> SetIncrement(0.01);

/*    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("Vds bias, V")), 0, wxSHAPED);
    wxSpinCtrlDouble *vds_bias = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, drain_voltage_max, sensor.Get_dac_ref(), 0.0, wxT("smth"));
    adjustment_sizer -> Add(vds_bias, 0, wxSHAPED);
    vds_bias -> SetDigits(2);
    vds_bias -> SetIncrement(0.01);
*/
    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("??Drain voltage max")), 0, wxSHAPED);
    output_v_max = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 40, sensor.Get_voltage_drain_max(), 0.0, wxT("smth"));
    output_v_max -> SetDigits(1);
    output_v_max -> SetIncrement(0.1);
    adjustment_sizer -> Add(output_v_max, 0, wxSHAPED);

    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("??Gate voltage max")), 0, wxSHAPED);
    output_v_min = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -40, 0, sensor.Get_voltage_gate_max(), 0.0, wxT("smth"));
    output_v_min -> SetDigits(1);
    output_v_min -> SetIncrement(0.1);
    adjustment_sizer -> Add(output_v_min, 0, wxSHAPED);

    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("R shunt")), 0, wxSHAPED);
    r_shunt = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 10, sensor.Get_shunt(), 0.0, wxT("smth"));
    r_shunt -> SetDigits(1);
    r_shunt -> SetIncrement(0.1);
    adjustment_sizer -> Add(r_shunt, 0, wxSHAPED);

    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("Zero correction, mA")), 0, wxSHAPED);
    adc_zero = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -100, 100, sensor.Get_zero_current(), 0.0, wxT("smth"));
    adc_zero -> SetDigits(3);
    adc_zero -> SetIncrement(0.001);
    adjustment_sizer -> Add(adc_zero, 0, wxSHAPED);

    // idDAC_ref_voltage idOutput_v_max idOutput_v_min idADC_ref_voltage idADC_zero


 //  IV_meas_start = new wxButton(setting_panel, idIV_meas_start, wxT("Start"));

//     sensor_com_connect = new wxButton(framework_panel, idSensor_connect, wxT("Connect"));


    wxButton* setting_OK = new wxButton(setting_panel, wxID_OK, "Ok"); // как совестить автоматическое закрывание окна и обработку данных?
//    wxButton* setting_OK = new wxButton(setting_panel, idSetting_OK, "Ok");
    wxButton* setting_cancel = new wxButton(setting_panel, wxID_CANCEL, "Cancel");
    button_sizer -> Add(setting_OK, 0, wxCENTER);
    button_sizer -> Add(setting_cancel, 0, wxCENTER);

    setting_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("")), 0, wxEXPAND);
    setting_sizer -> Add(adjustment_sizer, 0, wxCENTER);
    setting_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("")), 0, wxEXPAND);
    setting_sizer -> Add(button_sizer, 0, wxCENTER);

    setting_panel -> SetSizer(setting_sizer);


 //   ret_code = setting_change -> ShowModal();
  //  ret_code = setting_change -> Show(true);

	if (setting_change -> ShowModal() == wxID_OK) // If the user clicked "OK"
	{
//	    Beep(523,50);

        // bias for Vgs and Vds

        sensor.Set_dac_ref(dac_ref_voltage  -> GetValue());
  //      sensor.Set_Vdd(output_v_max -> GetValue());
    //    sensor.Set_Vss(output_v_min -> GetValue());
   //     sensor.Set_adc_ref(adc_ref_voltage -> GetValue());
        sensor.Set_shunt(r_shunt -> GetValue());
        sensor.Set_zero_current(adc_zero -> GetValue());

	}
	// Clean up after ourselves
	setting_change -> Destroy();

}

// функция чтобы обрабатывать ENTER https://forums.wxwidgets.org/viewtopic.php?f=1&t=46747&p=196346#p196332
// Catch wxEVT_TEXT_ENTER event and call Navigate() in the event handler
// в идеале надо ловить другое - EVT_SPINCTRLDOUBLE(id, func):
// Generated whenever the numeric value of the spin control is changed, that is, when the up/down spin button is clicked or when the control loses focus and the new value is different from the last one.

void interface_testFrame::OnSpinCtrlTextEnter(wxCommandEvent& evt)
{
    wxWindow* win = dynamic_cast<wxWindow*>(evt.GetEventObject());

    win -> Navigate();

}

void interface_testFrame::OnSettingSave(wxCommandEvent& evt)
{
    Beep(523,50);
}


void interface_testFrame::OnSettingDefault(wxCommandEvent& evt)
{
    Beep(523,50);
}

void interface_testFrame::Transient_modes(wxCommandEvent& evt)
{
    if (transient_mode -> GetSelection() == 0)
    {
        trans_gate_stop -> Disable();
        trans_gate_step -> Disable();
        trans_interval -> Disable();

    }
    else
    {
        trans_gate_stop -> Enable();
        trans_gate_step -> Enable();
        trans_interval -> Enable();
    }

}

void interface_testFrame::IV_modes(wxCommandEvent& evt)
{
    if (IV_mode -> GetSelection() == 0)
    {
        // change limites
        IV_bias -> SetRange(-1*gate_voltage_max, gate_voltage_max);
        IV_start -> SetRange(-1*drain_voltage_max, drain_voltage_max);
        IV_stop -> SetRange(-1*drain_voltage_max, drain_voltage_max);
    }
    else
    {
        // change limites
        IV_bias -> SetRange(-1*drain_voltage_max, drain_voltage_max);
        IV_start -> SetRange(-1*gate_voltage_max, gate_voltage_max);
        IV_stop -> SetRange(-1*gate_voltage_max, gate_voltage_max);
    }

}

/*
void interface_testFrame::Setting_OK(wxCommandEvent& evt)
// void interface_testFrame::OnButtonOkClick(wxCommandEvent& evt)
{
    Beep(523,50);
    sensor.Set_dac_ref(dac_ref_voltage  -> GetValue());
    sensor.Set_Vdd(output_v_max         -> GetValue());
    sensor.Set_Vss(output_v_min         -> GetValue());
    sensor.Set_adc_ref(adc_ref_voltage  -> GetValue());
    sensor.Set_shunt(r_shunt            -> GetValue());

    // установка нуля уже необязательно должна в класс сенсора грузиться? Или тоже в него? Надо еще подумать как именно.

    // close the dialog window
//    setting_change -> EndModal(idSetting_change); // virtual void wxDialog::EndModal 	( 	int  	retCode	)
  setting_change -> Show(false);


    // or
    // EndModal(wxID_OK)

 //   setting_change -> EndModal(ret_code); // virtual void wxDialog::EndModal 	( 	int  	retCode	)
    // Ends a modal dialog, passing a value to be returned from the ShowModal() invocation
    // retCode	The value that should be returned by ShowModal
// или надо свою таблицу собыитй создавать?


//

// wxWidgets does show wxID_OK and wxID_CANCEL specially in the dialogs by default and provides built-in logic for standard behaviour.
// However this shouldn't matter at all if you're not using wxDialog at all as you seem to be saying, although this is not totally clear.
// And, in fact, if you don't want to show it modally, there is no real reason to use wxDialog.
// If you really just create a normal wxFrame and call Close() from an event handler for one of its buttons, this should work independently of the ID you use for it.
// https://stackoverflow.com/questions/61026581/how-to-correctly-use-wxid-ok-and-wxid-cancel

// The wxID_* in the button ctor only allow you to implement event handlers in the popup window


// https://forums.wxwidgets.org/viewtopic.php?t=28613
// https://wxwidgets.info/wxaui_tutorial_2_ru/
// https://github.com/wxWidgets/wxWidgets/blob/master/src/univ/dialog.cpp

// or custom dialog
// https://zetcode.com/gui/wxwidgets/dialogs/?msclkid=9a667badd13311ec8f27e2b34b5aede3

// м.б. мы уже имеем "wxID_OK" - в обработке сохранения файла.... может этот ID занят уже. Надо делать произвольный, с закрытием. Но пока нет перехода и на произвольный....

}
*/

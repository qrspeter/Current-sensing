// идеи:
// еще неплохо бы в лестничном режиме выводить текущее напряжение на экран. Ну или просто выделить два окошка текущего напряжения.



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
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream> // std::stringstream
#include <algorithm> // is_sorted() in opening  file


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
    EVT_MENU(idMenuOpen, interface_testFrame::OnOpen)

    EVT_MENU(idMenuSettingSave, interface_testFrame::OnSettingSave)
    EVT_MENU(idMenuSettingChange, interface_testFrame::OnSettingChange)
    EVT_MENU(idMenuSettingDefault, interface_testFrame::OnSettingDefault)
    EVT_MENU(idMenuSettingLaser, interface_testFrame::OnSettingLaser)

    EVT_BUTTON(idSensor_connect, interface_testFrame::Sensor_connect)

    EVT_BUTTON(idIV_start, interface_testFrame::IV_start)
    EVT_BUTTON(idIV_stop, interface_testFrame::IV_stop)

    EVT_RADIOBOX(idIV_modes, interface_testFrame::IV_modes)
//    EVT_CHECKBOX(idPhotoelectric_mode, interface_testFrame::Photoelectric_mode)

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
    fileMenu->Append(idMenuOpen, _("&Open\tCtrl-o"), _("Open"));
    fileMenu->Append(idMenuQuit, _("&Quit\tAlt-F4"), _("Quit the application"));
    mbar->Append(fileMenu, _("&File"));

    wxMenu* settingMenu = new wxMenu(_T(""));
    settingMenu->Append(idMenuSettingSave, _("&Save setting\tCtrl-p"), _("Save current setting"));
    settingMenu->Append(idMenuSettingChange, _("&Change settings\tCtrl-a"), _("Change ADC-DAC settings"));
    settingMenu->Append(idMenuSettingLaser, _("&Laser setting\tCtrl-a"), _("Change laser settings"));
    settingMenu->Append(idMenuSettingDefault, _("Load &default\tCtrl-d"), _("Load default setting"));
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


    iv_mode = new wxRadioBox(framework_panel, idIV_modes, wxT("IV mode"), wxDefaultPosition, wxDefaultSize, 2, iv_modes, 0);
    control_sizer -> Add(iv_mode, 0, wxCENTER);

//    control_sizer -> Add(new wxStaticText(framework_panel, -1, wxT("")), 0, wxEXPAND);

    control_sizer -> Add(wxIV,   0, wxEXPAND, 0);

    iv_meas_start = new wxButton(framework_panel, idIV_start, wxT("Start"));
    control_sizer -> Add(iv_meas_start, 0, wxCENTER); //  | wxEXPAND
    iv_meas_stop = new wxButton(framework_panel, idIV_stop, wxT("Stop"));
    control_sizer -> Add(iv_meas_stop, 0, wxCENTER); //  | wxEXPAND

    control_sizer -> Add(new wxStaticText(framework_panel, -1, wxT("")), 0, wxEXPAND);


    transient_mode = new wxRadioBox(framework_panel, idTransient_modes, wxT("Laser"), wxDefaultPosition, wxDefaultSize, 2, transient_modes, 0);
    control_sizer -> Add(transient_mode, 0, wxCENTER);

    control_sizer -> Add(wxTransient,      0, wxEXPAND, 0);
    transient_meas_start = new wxButton(framework_panel, idTransient_start, wxT("Start"));
    control_sizer -> Add(transient_meas_start, 0, wxCENTER); //  | wxEXPAND
    transient_meas_stop = new wxButton(framework_panel, idTransient_stop, wxT("Stop"));
    control_sizer -> Add(transient_meas_stop, 0, wxCENTER); //  | wxEXPAND

    // idiv_mode

    iv_meas_start -> Disable();
    iv_meas_stop -> Disable();
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
    iv_bias = new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*sensor.GetGateLimit(), sensor.GetGateLimit(), 0.0, 0.0, wxT("smth"));
    wxIVGrid -> Add(iv_bias, 0, wxSHAPED);
    iv_bias -> SetDigits(1);
    iv_bias -> SetIncrement(0.1);
    wxIVGrid -> Add(new wxStaticText(framework_panel, -1, wxT("U start, V")), 0, wxSHAPED);
    iv_start = new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*sensor.GetDrainLimit(), sensor.GetDrainLimit(), 0.0, 0.0, wxT("smth"));
    iv_start -> SetDigits(1);
    iv_start -> SetIncrement(0.1);
    wxIVGrid -> Add(iv_start, 0, wxSHAPED);
    wxIVGrid -> Add(new wxStaticText(framework_panel, -1, wxT("U stop, V")), 0, wxSHAPED);
    iv_stop = new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*sensor.GetDrainLimit(), sensor.GetDrainLimit(), 2.0, 0.0, wxT("smth"));
    iv_stop -> SetDigits(1);
    iv_stop -> SetIncrement(0.1);
    wxIVGrid -> Add(iv_stop, 0, wxSHAPED);
    wxIVGrid -> Add(new wxStaticText(framework_panel, -1, wxT("U step, V")), 0, wxSHAPED);
    iv_step = new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 10.0, 0.1, 0.0, wxT("smth"));
    iv_step -> SetDigits(2);
    iv_step -> SetIncrement(0.05);
    wxIVGrid -> Add(iv_step, 0, wxSHAPED);


    // wxSpinCtrlDouble example: https://stackoverflow.com/questions/42706594/distinguishing-between-different-wxspinctrldouble-events
    // wxSpinCtrlDouble(frame, wxID_ANY, "1.0", wxPoint(10, 10), wxSize(150, 20), wxSP_ARROW_KEYS | wxALIGN_RIGHT | wxTE_PROCESS_ENTER, 1.0, 20.0, 1.0, 1.0);

    // use SetDigits - https://forums.wxwidgets.org/viewtopic.php?f=1&t=48925&p=210156



    wxIV -> Add(wxIVGrid, 0, wxSHAPED); // wxSHAPED


// Transient =======================

    wxGridSizer *wxTransientGrid  = new wxGridSizer(6,2,3,3);



    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Drain bias, V")), 0, wxSHAPED);
    trans_drain_bias =  new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*sensor.GetDrainLimit(), sensor.GetDrainLimit(), 0.0, 0.0, wxT("smth")); //
    trans_drain_bias -> SetDigits(1);
    trans_drain_bias -> SetIncrement(0.1);
    trans_drain_bias -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_drain_bias, 0, wxSHAPED);

    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Gate bias, V")), 0, wxSHAPED);
    trans_gate_bias =   new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*sensor.GetGateLimit(), sensor.GetGateLimit(), 0.0, 0.0, wxT("smth"));
    trans_gate_bias -> SetDigits(1);
    trans_gate_bias -> SetIncrement(0.1);
    trans_gate_bias -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_gate_bias, 0, wxSHAPED);

    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Step, s")), 0, wxSHAPED);
    trans_time_step =   new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0.0, 100.0, 1.0, 0.0, wxT("smth"));
    trans_time_step -> SetDigits(1);
    trans_time_step -> SetIncrement(0.1);
    trans_time_step -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_time_step, 0, wxSHAPED);

/*    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Delay, s")), 0, wxSHAPED);
    trans_step =        new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 1000, 0.0, 0.0, wxT("smth"));
    trans_step -> SetDigits(1);
    trans_step -> SetIncrement(0.1);
    trans_step -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_step, 0, wxSHAPED);


*/


/*    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Gate start, V")), 0, wxSHAPED);
    trans_gate_start =   new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*sensor.GetGateLimit(), sensor.GetGateLimit(), 0.0, 0.0, wxT("smth"));
    trans_gate_start -> SetDigits(1);
    trans_gate_start -> SetIncrement(0.1);
    trans_gate_start -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_gate_start, 0, wxSHAPED);
*/
    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Pulse period, s")), 0, wxSHAPED);
    trans_pulse_period =   new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 300, 10.0, 0.0, wxT("smth"));
    trans_pulse_period -> SetDigits(1);
    trans_pulse_period -> SetIncrement(0.1);
    trans_pulse_period -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_pulse_period, 0, wxSHAPED);

    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Pulse length")), 0, wxSHAPED);
    trans_pulse_length =   new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 300, 3.0, 0.0, wxT("smth"));
    trans_pulse_length -> SetDigits(1);
    trans_pulse_length -> SetIncrement(0.1);
    trans_pulse_length -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_pulse_length, 0, wxSHAPED);

    wxTransientGrid ->  Add(new wxStaticText(framework_panel, -1, wxT("Pulse delay")), 0, wxSHAPED);
    trans_pulse_delay =   new wxSpinCtrlDouble(framework_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 300, 10.0, 0.0, wxT("smth"));
    trans_pulse_delay -> SetDigits(1);
    trans_pulse_delay -> SetIncrement(0.1);
    trans_pulse_delay -> Bind(wxEVT_TEXT_ENTER, &OnSpinCtrlTextEnter, this);
    wxTransientGrid -> Add(trans_pulse_delay, 0, wxSHAPED);



    wxTransient -> Add(wxTransientGrid, 0, wxSHAPED); // wxSHAPED

    // idTrans_drain_bias idTrans_gate_bias idTrans_step

    trans_pulse_period -> Disable();
    trans_pulse_length -> Disable();
    trans_pulse_delay -> Disable();


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
	if(V_data.empty())
	{
        wxMessageDialog *dial = new wxMessageDialog (NULL, wxT( "No data to save!" ) , wxT( " Error" ) , wxOK | wxICON_ERROR);
        dial -> ShowModal();
	    return;
	}


	wxFileDialog *SaveDialog = new wxFileDialog(this, _("Save File As..."), wxEmptyString, wxEmptyString, _("Text files (*.csv)|*.csv|ASCII Files (*.txt)|*.txt"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

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

            if(T_data.size()) // Transient data to save
            {
                fsave << "#Time (sec), Current (A), Voltage (V)" << std::endl; //
                for(unsigned int i = 0; i < I_data.size(); ++i) //
                {
                    fsave << T_data[i] << "," << I_data[i] / 1000.0 << ","  << V_data[i] << std::endl;
                }
            }
            else // IV data to save
            {
                fsave << "#Voltage (V), Current (A)" << std::endl; //
                for(unsigned int i = 0; i < I_data.size(); ++i) //
                {
                    fsave << V_data[i] << "," << I_data[i] / 1000.0 << std::endl;
                }
            }

            fsave.close();
            SetTitle(wxString("Data saved - ") << SaveDialog -> GetFilename());
        }
        else
        {
           wxMessageBox( wxT("Can not open file"), wxT("Save file"), wxOK | wxICON_INFORMATION);

        }



	}

	// Clean up after ourselves
	SaveDialog -> Destroy();

 }

void interface_testFrame::OnOpen(wxCommandEvent &event)
{

    wxFileDialog *openFileDialog = new wxFileDialog(this, _("Open File..."), wxEmptyString, wxEmptyString, _("Comma Separated Files (*.csv)|*.csv|ASCII Files (*.asc)|*.asc*.csv|All Files (*.*)|*.*"), wxFD_OPEN|wxFD_FILE_MUST_EXIST, wxDefaultPosition);

    if (openFileDialog->ShowModal() == wxID_CANCEL)
        return;     // the user changed idea...

    std::ifstream fopen; // std::getline is designed for use with input stream classes (std::basic_istream) so you should be using the std::ifstream class instead of std::ofstream
    fopen.open(openFileDialog->GetPath(), std::ios::in);

    if (fopen.is_open())
    {
        std::string myline;

        I_data.resize(0);
        V_data.resize(0);
        T_data.resize(0);

        int ctr = -1;

        while(std::getline(fopen, myline))
        {
            if(myline[0] == '#'){continue;}

            if(ctr == -1)
            {
                ctr = 0;
                for (std::size_t i = 0; i < myline.length(); ++i) // / Example with C++23 size_t literal   for (auto i = 0uz; i != a.size(); ++i)
                {
                    if (myline[i] == ',')
                        ++ctr;
                }
            }


            std::string word;
            std::stringstream str(myline); // https://java2blog.com/read-csv-file-in-cpp/, https://stackoverflow.com/questions/34218040/how-to-read-a-csv-file-data-into-an-array

            if(ctr == 1)
            {
                std::getline(str, word, ',');
                V_data.push_back(std::stod(word, nullptr));
                std::getline(str, word, ',');
                I_data.push_back(1000 * std::stod(word, nullptr)); // A -> mA
            }
        else
            if(ctr == 2)
            {
                std::getline(str, word, ',');
                T_data.push_back(std::stod(word, nullptr));
                std::getline(str, word, ',');
                I_data.push_back(1000 * std::stod(word, nullptr)); // A -> mA
                std::getline(str, word, ',');
                V_data.push_back(std::stod(word, nullptr));
            }
            else{wxMessageBox( wxT("Wrong column numbers"), wxT("Open file"), wxOK | wxICON_INFORMATION); return;}
        }



        fopen.close();

        if(V_data.size() == 0)
        {
            wxMessageBox( wxT("File is empty"), wxT("Error"), wxOK | wxICON_INFORMATION);
            return;
        }


        SetTitle(wxString("Opened file - ") << openFileDialog -> GetFilename()); // SetTitle(wxString("Data saved - ") << SaveDialog->GetFilename());


        framework_graph -> DelAllLayers(true, true);
        mpInfoCoords *frame_coord = new mpInfoCoords();
        framework_graph->AddLayer(frame_coord);
        mpFXYVector *frameworkVector = new mpFXYVector();
        frameworkVector->SetContinuity(true);
        frameworkVector->SetPen(wxPen(wxColor(0xFF, 0x00, 0x00), 2, wxPENSTYLE_SOLID));


        framework_graph->AddLayer(frameworkVector);

        mpScaleY *scaleY = new mpScaleY(wxT("Current, mA"), mpALIGN_BOTTOM, true);
        mpScaleX *scaleX = new mpScaleX(wxT("X"), mpALIGN_LEFT, true);


// IV mode if V_data is strictly sorted, increasing or decreasing. is_sorted is not ehough
// https://stackoverflow.com/questions/43219003/c-is-there-stl-algorithm-to-check-if-the-range-is-strictly-sorted

//        )
        if((ctr == 1) &&
        (std::adjacent_find(V_data.begin(), V_data.end(), std::greater_equal<double>()) == V_data.end() || std::adjacent_find(V_data.begin(), V_data.end(), std::less_equal<double>()) == V_data.end()))
        {


            frameworkVector -> SetData(V_data, I_data);
            scaleX -> SetName(wxT("Voltage, V"));
        }
        else // Transient mode if T_data is strictly sorted
//        if
        if((ctr == 2) &&
        (std::adjacent_find(T_data.begin(), T_data.end(), std::greater_equal<double>()) == T_data.end()))
        {
            frameworkVector -> SetData(T_data, I_data);
            scaleX -> SetName(wxT("Time, T"));

        }
        else{wxMessageBox( wxT("Unknown data format"), wxT("Open file"), wxOK | wxICON_INFORMATION); return;}


        framework_graph -> AddLayer(scaleX);
        framework_graph -> AddLayer(scaleY);


        framework_graph -> Fit();

    }
    else{ wxMessageBox( wxT("Can not open file"), wxT("Open file"), wxOK | wxICON_INFORMATION);  }

}

void interface_testFrame::Sensor_connect(wxCommandEvent &event)
{

   if(sensor.CheckState())
    {   // already opened
        sensor.Close();
        sensor_com_connect->SetLabel("Connect");
        sensor_com_choice -> Enable();
        iv_meas_start -> Disable();
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
            sensor_com_connect -> SetLabel("Disconnect");
            sensor_com_choice -> Disable();
            iv_meas_start -> Enable();
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


void interface_testFrame::OnSettingChange(wxCommandEvent &event)
{

// wxWidgets does show wxID_OK and wxID_CANCEL specially in the dialogs by default and provides built-in logic for standard behaviour.
// However this shouldn't matter at all if you're not using wxDialog at all as you seem to be saying, although this is not totally clear.
// And, in fact, if you don't want to show it modally, there is no real reason to use wxDialog.
// If you really just create a normal wxFrame and call Close() from an event handler for one of its buttons, this should work independently of the ID you use for it.
// https://stackoverflow.com/questions/61026581/how-to-correctly-use-wxid-ok-and-wxid-cancel

// The wxID_* in the button ctor only allow you to implement event handlers in the popup window


// https://forums.wxwidgets.org/viewtopic.php?t=28613
// https://wxwidgets.info/wxaui_tutorial_2_ru/
// https://github.com/wxWidgets/wxWidgets/blob/master/src/univ/dialog.cpp


    // запрашивать результаты измерений выходного напряжения (тестером), например напряжение при задании на АЦП 0x000000, 0x888888 и 0xFFFFFF. И затем пересчет уже по этим данным
    // то есть если сейчас расчет идет по двум значениям (мин и макс), то там еще и ноль. И они также хранятся в виде данных класса, и меняются при настройке.
    // может начать пока с мин и макс, а ноль - если потребуется. Тогда надо будет отдельную функцию писать расчета величины, которая бы проверяла больше или меньше нуля.

    // и м.б. также калибровать АЦП, чтобы при нуле он показывал ноль.


// https://docs.wxwidgets.org/2.8.8/wx_wxdialog.html#wxdialog
    wxDialog *setting_change = new wxDialog(this, -1, _("Setting"), wxDefaultPosition, wxSize(250, 300), wxDEFAULT_DIALOG_STYLE, "dialogBox"); // wxDefaultSize



    wxPanel *setting_panel = new wxPanel(setting_change, -1);

    wxBoxSizer *setting_sizer = new wxBoxSizer( wxVERTICAL );

    wxGridSizer *adjustment_sizer = new wxGridSizer(7,2,3,3); //     wxGridSizer *wxIVGrid  = new wxGridSizer(4,2,3,3);
    wxGridSizer *button_sizer = new wxGridSizer(1,2,3,3);


    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("Auto zero")), 0, wxSHAPED);
    wxCheckBox *zero_correction = new wxCheckBox(setting_panel, -1, wxT("Enable"));
    adjustment_sizer -> Add(zero_correction, 0, wxSHAPED);
    zero_correction ->  SetValue(sensor.GetZeroCorrMode());


    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("Averaging")), 0, wxSHAPED);
    wxSpinCtrl *averaging = new wxSpinCtrl(setting_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 1, 64, sensor.GetAveraging(), wxT("smth"));
//    wxSpinCtrl *averaging = new wxSpinCtrl(setting_panel, wxID_ANY, wxT("5"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 5);
//    wxSpinCtrlDouble *averaging = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 1, 64, 4, 1, wxT("smth"));
    adjustment_sizer -> Add(averaging, 0, wxSHAPED);
//    averaging -> SetDigits(2);
//    averaging -> SetIncrement(1);


    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("Bias correction, V")), 0, wxSHAPED);
    wxSpinCtrlDouble *bias_corr = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -100, 100, sensor.Get_bias_corr(), 0.0, wxT("smth"));
    bias_corr -> SetDigits(2);
    bias_corr -> SetIncrement(0.01);
    adjustment_sizer -> Add(bias_corr, 0, wxSHAPED);

    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("DAC ref, V")), 0, wxSHAPED);
    wxSpinCtrlDouble *dac_ref_voltage = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, sensor.GetDrainLimit(), sensor.Get_dac_ref(), 0.0, wxT("smth"));
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
    wxSpinCtrlDouble *drain_max = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 40, sensor.GetDrainLimit(), 0.0, wxT("smth"));
    drain_max -> SetDigits(1);
    drain_max -> SetIncrement(0.1);
    adjustment_sizer -> Add(drain_max, 0, wxSHAPED);

    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("??Gate voltage max")), 0, wxSHAPED);
    wxSpinCtrlDouble *gate_max = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 40, sensor.GetGateLimit(), 0.0, wxT("smth"));
    gate_max -> SetDigits(1);
    gate_max -> SetIncrement(0.1);
    adjustment_sizer -> Add(gate_max, 0, wxSHAPED);

//    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("R shunt")), 0, wxSHAPED);
//    wxSpinCtrlDouble *r_shunt = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 10, sensor.Get_shunt(), 0.0, wxT("smth"));
//    r_shunt -> SetDigits(1);
//    r_shunt -> SetIncrement(0.1);
//    adjustment_sizer -> Add(r_shunt, 0, wxSHAPED);


    // idDAC_ref_voltage idOutput_v_max idOutput_v_min idADC_ref_voltage idADC_zero


 //  iv_meas_start = new wxButton(setting_panel, idIV_start, wxT("Start"));

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


	if (setting_change -> ShowModal() == wxID_OK) // If the user clicked "OK"
	{

        // bias for Vgs and Vds
        sensor.SetZeroCorrMode(zero_correction -> GetValue());
        sensor.SetAveraging(averaging -> GetValue());
        sensor.Set_bias_corr(bias_corr -> GetValue());
        sensor.Set_dac_ref(dac_ref_voltage  -> GetValue());
  //      sensor.Set_Vdd(output_v_max -> GetValue());
    //    sensor.Set_Vss(output_v_min -> GetValue());
   //     sensor.Set_adc_ref(adc_ref_voltage -> GetValue());
  //      sensor.Set_shunt(r_shunt -> GetValue());
//        sensor.Set_zero_current(adc_zero -> GetValue());
        sensor.SetGateLimit(gate_max -> GetValue());
        sensor.SetDrainLimit(drain_max  -> GetValue());

	}
	// Clean up after ourselves
	setting_change -> Destroy();

}


void interface_testFrame::OnSettingLaser(wxCommandEvent &event)
{
    wxDialog *setting_change = new wxDialog(this, -1, _("Laser setting"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, "dialogBox");



    wxPanel *setting_panel = new wxPanel(setting_change, -1);

    wxBoxSizer *setting_sizer = new wxBoxSizer( wxVERTICAL );

    wxGridSizer *adjustment_sizer = new wxGridSizer(4,2,3,3); //     wxGridSizer *wxIVGrid  = new wxGridSizer(4,2,3,3);
    wxGridSizer *button_sizer = new wxGridSizer(1,2,3,3);




    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("Pulse duration, s")), 0, wxSHAPED);
    wxSpinCtrlDouble *pulse_duration = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 100, sensor.GetPulseDuraion(), 0.0, wxT("smth"));
    adjustment_sizer -> Add(pulse_duration, 0, wxSHAPED);
    pulse_duration -> SetDigits(2);
    pulse_duration -> SetIncrement(0.1);

    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("Number of pulses")), 0, wxSHAPED);
    wxSpinCtrlDouble *pulse_numbers = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 40, sensor.GetPulseNumbers(), 0.0, wxT("smth"));
    adjustment_sizer -> Add(pulse_numbers, 0, wxSHAPED);
    pulse_numbers -> SetDigits(0);
    pulse_numbers -> SetIncrement(1);

    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("Pulse delay")), 0, wxSHAPED);
    wxSpinCtrlDouble *pulse_delay = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 40, sensor.GetPulseDelay(), 0.0, wxT("smth"));
    adjustment_sizer -> Add(pulse_delay, 0, wxSHAPED);
    pulse_delay -> SetDigits(2);
    pulse_delay -> SetIncrement(0.1);

    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("Total time is")), 0, wxSHAPED);
    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("2 * duration * number + delay")), 0, wxSHAPED);

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

	if (setting_change -> ShowModal() == wxID_OK) // If the user clicked "OK"
	{

        sensor.SetPulseDuraion(pulse_duration   -> GetValue());
        sensor.SetPulseNumbers(pulse_numbers    -> GetValue());
        sensor.SetPulseDelay(pulse_delay        -> GetValue());

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
        trans_pulse_period -> Disable();
        trans_pulse_length -> Disable();
        trans_pulse_delay -> Disable();

    }
    else
    {
        trans_pulse_period -> Enable();
        trans_pulse_length -> Enable();
        trans_pulse_delay -> Enable();
    }

}

void interface_testFrame::IV_modes(wxCommandEvent& evt)
{
    if (iv_mode -> GetSelection() == 0)
    {
        // change limites
        iv_bias -> SetRange(-1*sensor.GetGateLimit(), sensor.GetGateLimit());
        iv_start -> SetRange(-1*sensor.GetDrainLimit(), sensor.GetDrainLimit());
        iv_stop -> SetRange(-1*sensor.GetDrainLimit(), sensor.GetDrainLimit());
    }
    else
    {
        // change limites
        iv_bias -> SetRange(-1*sensor.GetDrainLimit(), sensor.GetDrainLimit());
        iv_start -> SetRange(-1*sensor.GetGateLimit(), sensor.GetGateLimit());
        iv_stop -> SetRange(-1*sensor.GetGateLimit(), sensor.GetGateLimit());
    }

}



// ===============================================================================================
// Measurements
// ===============================================================================================

void interface_testFrame::Start_ADC_wait(SENSOR_FET::resolution res, SENSOR_FET::gain gain_x)
{
    sensor.Start_ADC(res, gain_x);

    auto sleeping = sensor.GetAveraging() * sleep_time[sensor_resolution  -> GetSelection()];
    if(sleeping < static_cast<int>(delay_meas -> GetValue()*1000))
    Sleep(static_cast<int>(delay_meas -> GetValue()*1000));
    else
    Sleep(sleeping);
}


void interface_testFrame::IV_start(wxCommandEvent &event)
{
    if(!sensor.CheckState())
    {
        wxMessageDialog *dial = new wxMessageDialog (NULL, wxT( "Sensor is not connected" ) , wxT( " Error" ) , wxOK | wxICON_ERROR);
        dial -> ShowModal();
        return;
    }

    SetTitle(wxString("IV measurement – Data not saved"));

    iv_meas_stop -> Enable();
    iv_meas_start -> Disable();
    iv_mode -> Disable();
    transient_meas_start -> Disable();
    measurementStop = FALSE;


    framework_graph -> DelAllLayers(true, true);
    mpInfoCoords *frame_coord = new mpInfoCoords();
    framework_graph -> AddLayer(frame_coord);
    mpFXYVector *frameworkVector = new mpFXYVector();
    frameworkVector -> SetContinuity(true);
    frameworkVector -> SetPen(wxPen(wxColor(0x00, 0x00, 0xFF), 2, wxPENSTYLE_SOLID));
    framework_graph -> AddLayer(frameworkVector);



    mpScaleY *scaleY = new mpScaleY(wxT("Current, mA"), mpALIGN_BOTTOM, true);

    mpScaleX *scaleX = new mpScaleX(wxT("Voltage, V"), mpALIGN_LEFT, true);

    framework_graph -> AddLayer(scaleX);
    framework_graph -> AddLayer(scaleY);

    I_data.resize(0);
    V_data.resize(0);
    T_data.resize(0);

    // сперва смещение, терминал определяется по состоянию переключателя. Выбор 0 - значит смещение на затвор, а сканирование на сток, 1 - наоборот.
    if(iv_mode -> GetSelection() == 0)
    sensor.Set_voltage(SENSOR_FET::GATE, iv_bias -> GetValue());
    else
    sensor.Set_voltage(SENSOR_FET::DRAIN, iv_bias -> GetValue());

    double start = iv_start -> GetValue();
    double stop = iv_stop -> GetValue();
    double step = iv_step -> GetValue();

    double scanDirection = 1.0;
    if(start > stop)
    {
        scanDirection = -1.0;
    }

    int elements = 1 + static_cast<int> ( scanDirection * (stop - start) / static_cast<double>(step));


    I_data.reserve(elements);
    V_data.reserve(elements);


    SENSOR_FET::resolution res = static_cast<SENSOR_FET::resolution>(sensor_resolution  -> GetSelection());

    double zero_correction{0};
    if(sensor.GetZeroCorrMode())
    {
        sensor.Set_voltage(SENSOR_FET::DRAIN, 0);
        sensor.Set_voltage(SENSOR_FET::GATE, 0);
        Start_ADC_wait(res, static_cast<SENSOR_FET::gain>(sensor_gain  -> GetSelection()));
        zero_correction = sensor.Get_current();
    }


    for(double voltage = start; scanDirection * voltage <= scanDirection * stop + 1e-10;  voltage += scanDirection * step) // 0.0000000001 добавлена чтобы компенсировать набегание погрешности и окончания до последней точки
    {

        wxYield();
        if(measurementStop)
        break;

        sensor.Set_voltage(static_cast<SENSOR_FET::terminal>(iv_mode -> GetSelection()), voltage);

        // тут надо подождать при первом измерении секунду, а то скачок сигнала...
//        if(voltage == start)
//        {
//            Sleep(1000);  // костыль, но хоть так!
//
//        }


        res = static_cast<SENSOR_FET::resolution>(sensor_resolution  -> GetSelection());

        Start_ADC_wait(res, static_cast<SENSOR_FET::gain>(sensor_gain  -> GetSelection()));

        V_data.push_back(voltage);
        I_data.push_back(sensor.Get_current() - zero_correction); //  Get_ADC

        frameworkVector -> SetData(V_data, I_data);

        auto vMinmax = std::minmax_element(I_data.begin(), I_data.end(), [](int a, int b){return (a < b);} ); //compLess);

        double edge_x = (stop - start) / 50.0;
        if(start < stop)
            framework_graph -> Fit(start - edge_x, stop + edge_x, *vMinmax.first, *vMinmax.second);
        else
            framework_graph -> Fit(stop + edge_x, start - edge_x, *vMinmax.second, *vMinmax.first);




        if(measurementStop)
        break;
    }

    framework_graph -> Fit(); // иногда последний фит не срабатывает почему-то.

    sensor.Set_voltage(SENSOR_FET::GATE, 0);
    sensor.Set_voltage(SENSOR_FET::DRAIN, 0);

    iv_meas_stop    -> Disable();
    iv_meas_start   -> Enable();
    transient_meas_start    -> Enable();
    measurementStop = FALSE;

}

void interface_testFrame::IV_stop(wxCommandEvent &event)
{
    iv_meas_stop -> Disable();

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
    transient_mode -> Disable();
    trans_pulse_period -> Disable();
    trans_pulse_length -> Disable();
    trans_pulse_delay -> Disable();
    iv_mode -> Disable();
    iv_meas_start -> Disable();
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


    I_data.resize(0);
    V_data.resize(0);
    T_data.resize(0);


    SENSOR_FET::resolution res = static_cast<SENSOR_FET::resolution>(sensor_resolution  -> GetSelection());

    double zero_correction{0};
    if(sensor.GetZeroCorrMode())
    {
        sensor.Set_voltage(SENSOR_FET::DRAIN, 0);
        sensor.Set_voltage(SENSOR_FET::GATE, 0);
        Start_ADC_wait(res, static_cast<SENSOR_FET::gain>(sensor_gain  -> GetSelection()));
        zero_correction = sensor.Get_current();
    }



    auto start = std::chrono::steady_clock::now();

    auto target_time = start;
    sensor.Laser(SENSOR_FET::LASER_OFF);

    auto period = trans_pulse_period -> GetValue();
    auto length = trans_pulse_length -> GetValue();
    auto delay = trans_pulse_delay -> GetValue();
    do
    {
        std::this_thread::sleep_until(target_time);
        target_time = target_time + std::chrono::milliseconds(static_cast<int>((trans_time_step -> GetValue()) * 1000));
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() / 1000.0;

        double working_time = static_cast<double>(duration) - delay;
        if(((transient_mode -> GetSelection()) == 1) && (working_time > 0))
        {

            auto counter = static_cast<int>(working_time / period);
            if((working_time - counter * period) < length)
            {
                sensor.Laser(SENSOR_FET::LASER_ON);
            }
            else{sensor.Laser(SENSOR_FET::LASER_OFF);}
        }

        sensor.Set_voltage(SENSOR_FET::GATE, trans_gate_bias -> GetValue());
        sensor.Set_voltage(SENSOR_FET::DRAIN, trans_drain_bias -> GetValue());


        res = static_cast<SENSOR_FET::resolution>(sensor_resolution  -> GetSelection());

        Start_ADC_wait(res, static_cast<SENSOR_FET::gain>(sensor_gain  -> GetSelection()));


        I_data.push_back(sensor.Get_current() - zero_correction); //  Get_ADC
        T_data.push_back(static_cast<double>(duration));
      //  T_data.push_back(static_cast<double>(since(start_time).count())/1000.0);
        V_data.push_back(trans_drain_bias -> GetValue());



        frameworkVector -> SetData(T_data, I_data);
        framework_graph -> Fit();

        wxYield();

    }
    while(!measurementStop);
    sensor.Laser(SENSOR_FET::LASER_OFF);

    sensor.Set_voltage(SENSOR_FET::GATE, 0);
    sensor.Set_voltage(SENSOR_FET::DRAIN, 0);




    SetTitle(wxString("IV measure – Data not saved"));
    transient_meas_stop -> Disable();
    transient_meas_start -> Enable();
    transient_mode -> Enable();
    trans_pulse_period -> Enable();
    trans_pulse_length -> Enable();
    trans_pulse_delay -> Enable();
    iv_mode -> Enable();
    iv_meas_start -> Enable();
    measurementStop = FALSE;


}
void interface_testFrame::Transient_stop(wxCommandEvent &event)
{
    transient_meas_stop -> Disable();

    measurementStop = TRUE;
}


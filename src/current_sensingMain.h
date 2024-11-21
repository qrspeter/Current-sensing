/***************************************************************
 * Name:      current_sensingMain.h
 * Purpose:   Defines Application Frame
 * Author:    Peter ()
 * Created:   2022-03-14
 * Copyright: Peter ()
 * License:
 **************************************************************/

#ifndef CURRENT_SENSINGMAIN_H
#define CURRENT_SENSINGMAIN_H

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <vector>

#include "current_sensingApp.h"

#include <wx/dialog.h>
#include <wx/string.h>

#include "mathplot/mathplot.h"
#include "mathplot/MathPlotConfig.h"
#include "simpleini/SimpleIni.h"

#include "sensor_fet/sensor_fet.h"

class current_sensingFrame: public wxFrame
{
    public:
        current_sensingFrame(wxFrame *frame, const wxString& title);
        ~current_sensingFrame();
    private:

        std::vector <double> I_data;
        std::vector <double> V_data;
        std::vector <double> T_data;

        std::vector<std::string> actual_ports;
        //const wxString ports[50] = { wxT("COM1"), wxT("COM2"), wxT("COM3"), wxT("COM4"), wxT("COM5"), wxT("COM6"), wxT("COM7"), wxT("COM8"), wxT("COM9")};
        const wxString adc_resolution[4] = { wxT("12 bit"), wxT("14 bit"), wxT("16 bit"), wxT("18 bit")};
        const wxString adc_gain[4] = { wxT("1x"), wxT("2x"), wxT("4x"), wxT("8x")};
        const wxString iv_modes[2] = { wxT("Ids(Uds)"), wxT("Ids(Ugs)")};
        const wxString transient_modes[2] = { wxT("Off"), wxT("On")};

        bool measurementStop{FALSE}; // Flag

        // move to SENSOR class
        const int sleep_time[4]{ 5, 17, 67, 267 }; // 12bit = 1000/240 = 5, 14bit = 1000/60 = 17, 16bit = 1000/15 = 67, 18bit = 1000/3.75 = 267


        SENSOR_FET sensor;
        CSimpleIniA ini;
        const std::string file_ini = "setting.ini";
        const std::string default_ini_data = "[Settings] Auto_zero = false  \n Averaging = 1  \n Bias_correction = -1.2  \n Current_correction = 1.1 \n  \n [Laser_setting] \n Pulse_duration = 0.2 \n Pulse_numbers = 4 \n Pulse_delay = 0.05";


        enum
        {
            idMenuQuit = 1000,
            idMenuAbout,
            idMenuSaveAs,
            idMenuOpen,
            idMenuSettingSave,
            idMenuSettingChange,
            idMenuSettingDefault,
            idMenuSettingLaser,

            idIV_start,
            idIV_stop,
            idIV_modes,

            idTransient_start,
            idTransient_stop,
            idTransient_modes,

            idSensor_connect,

        };
        void OnClose(wxCloseEvent& event);
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnSaveAs(wxCommandEvent& event);
        void OnOpen(wxCommandEvent& event);
        void OnSettingSave(wxCommandEvent& event);
        void OnSettingChange(wxCommandEvent& event);
        void OnSettingDefault(wxCommandEvent& event);
        void OnSettingLaser(wxCommandEvent& event);


//        void Sensor_COM(wxCommandEvent &event);
        void Sensor_connect(wxCommandEvent &event);

        void IV_start(wxCommandEvent &event);
        void IV_stop(wxCommandEvent &event);
        void IV_modes(wxCommandEvent &event);
        void Start_ADC_wait(SENSOR_FET::resolution, SENSOR_FET::gain);


        void Transient_start(wxCommandEvent &event);
        void Transient_stop(wxCommandEvent &event);
        void Transient_modes(wxCommandEvent &event);
        void OnSpinCtrlTextEnter(wxCommandEvent& evt);

        void Setting_OK(wxCommandEvent& evt);
        std::vector<std::string> SerialList();

        wxPanel *framework_panel;

        wxButton *iv_meas_start;
        wxButton *iv_meas_stop;
        wxButton *transient_meas_start;
        wxButton *transient_meas_stop;

        mpWindow *framework_graph;

        wxButton *sensor_com_connect;
        wxChoice *sensor_com_choice;
        wxChoice *sensor_resolution;
        wxChoice *sensor_gain;
        wxSpinCtrlDouble *delay_meas;

        wxRadioBox *iv_mode;

        wxSpinCtrlDouble *iv_bias;
        wxSpinCtrlDouble *iv_start;
        wxSpinCtrlDouble *iv_stop;
        wxSpinCtrlDouble *iv_step;

        wxRadioBox *transient_mode;

        wxSpinCtrlDouble *trans_drain_bias;
        wxSpinCtrlDouble *trans_gate_bias;
        wxSpinCtrlDouble *trans_time_step;
        wxSpinCtrlDouble *trans_pulse_period;
        wxSpinCtrlDouble *trans_pulse_length;
        wxSpinCtrlDouble *trans_pulse_delay;


        DECLARE_EVENT_TABLE()
};




#endif // CURRENT_SENSINGMAIN_H

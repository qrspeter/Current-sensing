/***************************************************************
 * Name:      interface_testMain.h
 * Purpose:   Defines Application Frame
 * Author:    Peter ()
 * Created:   2022-03-14
 * Copyright: Peter ()
 * License:
 **************************************************************/

#ifndef INTERFACE_TESTMAIN_H
#define INTERFACE_TESTMAIN_H

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "interface_testApp.h"

#include <wx/dialog.h>

#include "mathplot/mathplot.h"
#include "mathplot/MathPlotConfig.h"

#include "sensor_fet/sensor_fet.h"

class interface_testFrame: public wxFrame
{
    public:
        interface_testFrame(wxFrame *frame, const wxString& title);
        ~interface_testFrame();
    private:
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
        void Start_ADC_wait(resolution, gain);


        void Transient_start(wxCommandEvent &event);
        void Transient_stop(wxCommandEvent &event);
        void Transient_modes(wxCommandEvent &event);
        void OnSpinCtrlTextEnter(wxCommandEvent& evt);

        void Setting_OK(wxCommandEvent& evt);


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
        wxSpinCtrlDouble *trans_gate_stop;
        wxSpinCtrlDouble *trans_gate_step;
        wxSpinCtrlDouble *trans_interval;


        DECLARE_EVENT_TABLE()
};




#endif // INTERFACE_TESTMAIN_H

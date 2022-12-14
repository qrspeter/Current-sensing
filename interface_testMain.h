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
extern SENSOR_FET sensor;

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
            idMenuSettingSave,
            idMenuSettingChange,
            idMenuSettingDefault,

            idIV_meas_start,
            idIV_meas_stop,
            idIV_modes,
//            idTransferStart,
 //           idTransferStop,
            idTransient_start,
            idTransient_stop,
            idTransient_modes,

            idSensor_connect,

            // кажется лишние
/*            idSensor_COM,
            idSensor_resolution,
            idSensor_gain,
*/
    /*        idIV_mode,
            idIV_bias,
            idIV_start,
            idIV_stop,
            idIV_step,
*/
            // кажется лишние
/*            idTrans_drain_bias,
            idTrans_gate_bias,
            idTrans_step,
*/

// зачем столько событий, если все можно обрабатывать в событиях кнопки "ОК" или "Apply"?
// да и в списке выше куча незадействованных id, к которым нет обращения.
   /*         idDAC_ref_voltage,
            idOutput_v_max,
            idOutput_v_min,
            idADC_ref_voltage,
            idADC_zero,
*/
            // проверить какое из них работает, так то нужна всего одна
   //         idSetting_OK,
   //         idSetting_change

        };
        void OnClose(wxCloseEvent& event);
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnSaveAs(wxCommandEvent& event);
        void OnSettingSave(wxCommandEvent& event);
        void OnSettingChange(wxCommandEvent& event);
        void OnSettingDefault(wxCommandEvent& event);


//        void Sensor_COM(wxCommandEvent &event);
        void Sensor_connect(wxCommandEvent &event);

        void Meas_start(wxCommandEvent &event);
        void Meas_stop(wxCommandEvent &event);
        void IV_modes(wxCommandEvent &event);


        void Transient_start(wxCommandEvent &event);
        void Transient_stop(wxCommandEvent &event);
        void Transient_modes(wxCommandEvent &event);
        void OnSpinCtrlTextEnter(wxCommandEvent& evt);

        void Setting_OK(wxCommandEvent& evt);


        wxPanel *framework_panel;

        wxButton *IV_meas_start;
        wxButton *IV_meas_stop;
        wxButton *transient_meas_start;
        wxButton *transient_meas_stop;

        mpWindow *framework_graph;

        wxButton *sensor_com_connect;
        wxChoice *sensor_com_choice;
        wxChoice *sensor_resolution;
        wxChoice *sensor_gain;
        wxSpinCtrlDouble *delay_meas;

        wxRadioBox *IV_mode;

        wxSpinCtrlDouble *IV_bias;
        wxSpinCtrlDouble *IV_start;
        wxSpinCtrlDouble *IV_stop;
        wxSpinCtrlDouble *IV_step;

        wxRadioBox *transient_mode;

        wxSpinCtrlDouble *trans_drain_bias;
        wxSpinCtrlDouble *trans_gate_start;
        wxSpinCtrlDouble *trans_gate_stop;
        wxSpinCtrlDouble *trans_gate_step;
        wxSpinCtrlDouble *trans_interval;


  //      wxDialog settingDAC;
        wxSpinCtrlDouble *dac_ref_voltage;
        wxSpinCtrlDouble *output_v_max;
        wxSpinCtrlDouble *output_v_min;
//        wxSpinCtrlDouble *adc_ref_voltage;
        wxSpinCtrlDouble *r_shunt;
        wxSpinCtrlDouble *adc_zero;

        wxDialog *setting_change;
 //       wxButton* setting_OK;


        DECLARE_EVENT_TABLE()
};




#endif // INTERFACE_TESTMAIN_H

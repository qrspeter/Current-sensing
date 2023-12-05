#include "setting_change.h"

BEGIN_EVENT_TABLE(setting_change, wxFrame)
    EVT_CLOSE(setting_change::OnClose)
    EVT_MENU(idMenuSaveAs, setting_change::OnSaveAs)
    EVT_MENU(idMenuAbout, setting_change::OnAbout)
    EVT_MENU(idMenuQuit, setting_change::OnQuit)

    EVT_MENU(idMenuSettingChange, setting_change::OnSettingChange)

    EVT_BUTTON(idSensor_connect, setting_change::Sensor_connect)

    EVT_BUTTON(idIV_meas_start, setting_change::Meas_start)
    EVT_BUTTON(idIV_meas_stop, setting_change::Meas_stop)

    EVT_BUTTON(idTransient_start, setting_change::Transient_start)
    EVT_BUTTON(idTransient_stop, setting_change::Transient_stop)
//    EVT_BUTTON(idSetting_OK, setting_change::Setting_OK)
    EVT_BUTTON(wxID_OK, setting_change::Setting_OK)



END_EVENT_TABLE()


setting_change::setting_change(wxFrame *frame, const wxString& title)
    : wxFrame(frame, -1, title)
{

    wxPanel *setting_panel = new wxPanel(setting_change, -1);

    wxBoxSizer *setting_sizer = new wxBoxSizer( wxVERTICAL );

    wxGridSizer *adjustment_sizer = new wxGridSizer(5,2,3,3); //     wxGridSizer *wxIVGrid  = new wxGridSizer(4,2,3,3);
    wxGridSizer *button_sizer = new wxGridSizer(1,2,3,3);




    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("DAC ref voltage")), 0, wxSHAPED);
    dac_ref_voltage = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, drain_voltage_max, sensor.Get_dac_ref(), 0.0, wxT("smth"));
    adjustment_sizer -> Add(dac_ref_voltage, 0, wxSHAPED);
    dac_ref_voltage -> SetDigits(2);
    dac_ref_voltage -> SetIncrement(0.01);
    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("Output voltage max voltage_dd")), 0, wxSHAPED);
    output_v_max = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 40, sensor.Get_Vdd(), 0.0, wxT("smth"));
    output_v_max -> SetDigits(1);
    output_v_max -> SetIncrement(0.1);
    adjustment_sizer -> Add(output_v_max, 0, wxSHAPED);
    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("Output voltage min voltage_ss")), 0, wxSHAPED);
    output_v_min = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -40, 0, sensor.Get_Vss(), 0.0, wxT("smth"));
    output_v_min -> SetDigits(1);
    output_v_min -> SetIncrement(0.1);
    adjustment_sizer -> Add(output_v_min, 0, wxSHAPED);
    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("ADC ref voltage")), 0, wxSHAPED);
    adc_ref_voltage = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, -1*drain_voltage_max, drain_voltage_max, sensor.Get_adc_ref(), 0.0, wxT("smth"));
    adc_ref_voltage -> SetDigits(2);
    adc_ref_voltage -> SetIncrement(0.01);
    adjustment_sizer -> Add(adc_ref_voltage, 0, wxSHAPED);
    adjustment_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("R shunt")), 0, wxSHAPED);
    r_shunt = new wxSpinCtrlDouble(setting_panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 10, 1, 0.0, wxT("smth"));
    r_shunt -> SetDigits(1);
    r_shunt -> SetIncrement(0.1);
    adjustment_sizer -> Add(r_shunt, 0, wxSHAPED);

    // idDAC_ref_voltage idOutput_v_max idOutput_v_min idADC_ref_voltage

/*    adc_zero = new wxSpinCtrlDouble(setting_panel, idADC_zero, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 10, 1, 0.0, wxT("smth"));
    adc_zero -> SetDigits(1);
    adc_zero -> SetIncrement(0.1);
    adjustment_sizer -> Add(adc_zero, 0, wxSHAPED);
*/


 //  IV_meas_start = new wxButton(setting_panel, idIV_meas_start, wxT("Start"));

//     sensor_com_connect = new wxButton(framework_panel, idSensor_connect, wxT("Connect"));


    wxButton* setting_OK = new wxButton(setting_panel, wxID_OK, "Ok"); // как совестить автоматическое закрывание окна и обработку данных?
//    setting_OK = new wxButton(setting_panel, idSetting_OK, "Ok");
    wxButton* setting_cancel = new wxButton(setting_panel, wxID_CANCEL, "Cancel");
    button_sizer -> Add(setting_OK, 0, wxCENTER);
    button_sizer -> Add(setting_cancel, 0, wxCENTER);

    setting_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("")), 0, wxEXPAND);
    setting_sizer -> Add(adjustment_sizer, 0, wxCENTER);
    setting_sizer -> Add(new wxStaticText(setting_panel, -1, wxT("")), 0, wxEXPAND);
    setting_sizer -> Add(button_sizer, 0, wxCENTER);





}

#include "measurement.h"

#include <wx/filename.h>
#include <wx/progdlg.h> // https://docs.wxwidgets.org/trunk/classwx_progress_dialog.html или https://flylib.com/books/en/3.138.1.74/1/


#include <vector>
#include <chrono>

extern SENSOR_FET sensor;

extern std::vector <double> I_data;
extern std::vector <double> V_data;
extern std::vector <double> T_data;
extern std::vector <double> P_data;

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

extern bool measurementStop;

//extern void interface_testFrame::Transient_stop(wxCommandEvent &event)

// void interface_testFrame::Start_ADC_wait(resolution, gain);
// void Start_ADC_wait(resolution res , gain gain_x)
void interface_testFrame::Start_ADC_wait(resolution res , gain gain_x)
{
    sensor.Start_ADC(res, gain_x);

    if((sleep_time[sensor_resolution -> GetSelection()]) < static_cast<int>(delay_meas -> GetValue()*1000))
    Sleep(static_cast<int>(delay_meas -> GetValue()*1000));
    else
    Sleep(sleep_time[sensor_resolution  -> GetSelection()]);
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
    photoelectric_mode -> Disable();
    transient_meas_start -> Disable();
    measurementStop = FALSE;


    framework_graph -> DelAllLayers(true, true);
    mpInfoCoords *frame_coord = new mpInfoCoords();
    framework_graph->AddLayer(frame_coord);
    mpFXYVector *frameworkVector = new mpFXYVector();
    frameworkVector->SetContinuity(true);
    frameworkVector->SetPen(wxPen(wxColor(0x00, 0x00, 0xFF), 2, wxPENSTYLE_SOLID));
    framework_graph->AddLayer(frameworkVector);

    mpFXYVector *frameworkVector_photo = new mpFXYVector();
    if(photoelectric_mode -> GetValue())
    {
        frameworkVector_photo->SetContinuity(true);
        frameworkVector_photo->SetPen(wxPen(wxColor(0xFF, 0x00, 0x00), 2, wxPENSTYLE_SOLID));
        framework_graph->AddLayer(frameworkVector_photo);

    }



    mpScaleY *scaleY = new mpScaleY(wxT("Current, mA"), mpALIGN_BOTTOM, true);
//    mpScaleY *scaleY_photo = new mpScaleY(wxT("Current, mA"), mpALIGN_BOTTOM, true);

    mpScaleX *scaleX = new mpScaleX(wxT("Voltage, V"), mpALIGN_LEFT, true);

    framework_graph->AddLayer(scaleX);
    framework_graph->AddLayer(scaleY);
//    if(photoelectric_mode -> GetValue())
  //      framework_graph->AddLayer(scaleY_photo);

    I_data.resize(0);
    V_data.resize(0);
    T_data.resize(0);
    P_data.resize(0);
    sensor.Laser(LASER_OFF);

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
    if(photoelectric_mode -> GetValue())
        P_data.reserve(elements);

    for(double voltage = start; scanDirection * voltage <= scanDirection * stop + 1e-10;  voltage += scanDirection * step) // 0.0000000001 добавлена чтобы компенсировать набегание погрешности и окончания до последней точки
    {

        wxYield();
        if(measurementStop)
        break;

        sensor.Set_voltage(static_cast<terminal>(IV_mode -> GetSelection()) , voltage);

        // тут надо подождать при первом измерении секунду, а то скачок сигнала...
//        if(voltage == start)
//        {
//            Sleep(1000);  // костыль, но хоть так!
//
//        }


        resolution res = static_cast<resolution>(sensor_resolution  -> GetSelection());

        // вариант 1 - считать при каждом напряжении сигнал с лазером, без, и записывать усредненный темновой сигнал в I_data, а свeтовую разницу - в P_data.
        // тогда особо переделывать нечего - только массив добавить, и м.б. возможность отображения той или иной кривой, или обе, если это возможно... например одна истинная, а вторая нормированная к первой.
        // и записывать в файл тоже по-разному, то ли отдельными командами, то ли одной -но вместе, одноименные с дополнительной приставкой к имени ("_photo")
        // вариант 2 - писать то что выше, и одновременно полную кривую, но не выводить на экран (а то порвется в режиме вывода IV тк будет много повторяющихся V).
        // Решено, реализуем п. 1 и дополняет пунктом 2.
        // эту исходную кривую можно поначалу в фотоном режиме в файл скидывать, и вообще не предполагать её вывод на экран, кроме как... кроме как в режиме вывода I(t))


        if(photoelectric_mode -> GetValue())
        {
            double pulse_delay = sensor.GetPulseDelay();
            double pulse_duration = sensor.GetPulseDuraion();
            int pulse_numbers = sensor.GetPulseNumbers();
            double accum_dark{0};
            double accum_light{0};
            for(int i = 0; i < pulse_numbers; ++i)
            {
                int measurements{0};
                Sleep(pulse_delay);
                auto start = std::chrono::steady_clock::now();
                double accum_curr{0};
                while(static_cast<double>(since(start).count())/1000.0 < pulse_duration)
                {
                    Start_ADC_wait(res, static_cast<gain>(sensor_gain  -> GetSelection()));

//                    sensor.Start_ADC(res, static_cast<gain>(sensor_gain  -> GetSelection()));
//                    if((sleep_time[sensor_resolution -> GetSelection()]) < static_cast<int>(delay_meas -> GetValue()*1000))
//                    Sleep(static_cast<int>(delay_meas -> GetValue()*1000));
//                    else
//                    Sleep(sleep_time[sensor_resolution  -> GetSelection()]);

                    accum_curr += sensor.Get_current();
                    measurements++;
                }

                accum_dark += accum_curr / static_cast<double>(measurements);

                sensor.Laser(LASER_ON);
                Sleep(pulse_delay);
                measurements = 0;
                accum_curr = 0;

                while(static_cast<double>(since(start).count())/1000.0 < pulse_duration)
                {
                    Start_ADC_wait(res, static_cast<gain>(sensor_gain  -> GetSelection()));

//                    sensor.Start_ADC(res, static_cast<gain>(sensor_gain  -> GetSelection()));
//                    if( (sleep_time[sensor_resolution  -> GetSelection()]) < static_cast<int>(delay_meas -> GetValue()*1000))
//                    Sleep(static_cast<int>(delay_meas -> GetValue()*1000));
//                    else
//                    Sleep(sleep_time[sensor_resolution  -> GetSelection()]);

                    accum_curr +=  sensor.Get_current();
                    measurements++;
                }


                accum_light += accum_curr / static_cast<double>(measurements);
                sensor.Laser(LASER_OFF);

            }


            V_data.push_back(voltage);
            I_data.push_back(accum_dark/static_cast<double>(pulse_numbers)); //  Get_ADC
            P_data.push_back((accum_light - accum_dark)/static_cast<double>(pulse_numbers));
            T_data.push_back(static_cast<double>(since(start_time).count())/1000.0);

            // можно отображать и P_data и I_data?
            // https://sourceforge.net/p/wxmathplot/discussion/297266/thread/9c3512d9a6/
            frameworkVector -> SetData(V_data, I_data);
            frameworkVector_photo -> SetData(V_data, P_data);



        }
        else
        {
            Start_ADC_wait(res, static_cast<gain>(sensor_gain  -> GetSelection()));

//            sensor.Start_ADC(res, static_cast<gain>(sensor_gain  -> GetSelection()));
//            if((sleep_time[sensor_resolution -> GetSelection()]) < static_cast<int>(delay_meas -> GetValue()*1000))
//            Sleep(static_cast<int>(delay_meas -> GetValue()*1000));
//            else
//            Sleep(sleep_time[sensor_resolution  -> GetSelection()]);

            V_data.push_back(voltage);
            I_data.push_back(sensor.Get_current()); //  Get_ADC
            T_data.push_back(static_cast<double>(since(start_time).count())/1000.0);

            frameworkVector -> SetData(V_data, I_data);
        }

        framework_graph -> Fit();

// //глючит вертикальное отображение когда кривая не растет, а падает, выглядит нормально если мышкой вызывать Fit...

//        auto vMinmax = std::minmax_element(I_data.begin(), I_data.end(), compLess);
//
//        if(start < stop)
//        framework_graph -> Fit(start - edge_x, stop + edge_x, *vMinmax.first, *vMinmax.second);
//        else
//        framework_graph -> Fit();
//       // framework_graph -> Fit(stop + edge_x, start - edge_x, *vMinmax.second, *vMinmax.first);




        if(measurementStop)
        break;
    }

    sensor.Set_voltage(GATE, 0);
    sensor.Set_voltage(DRAIN, 0);

    IV_meas_stop    -> Disable();
    IV_meas_start   -> Enable();
    if(photoelectric_mode -> GetValue() == 0)
    {
        IV_mode -> Enable();
    }
    photoelectric_mode      -> Enable();
    transient_meas_start    -> Enable();
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

        Start_ADC_wait(res, static_cast<gain>(sensor_gain  -> GetSelection()));

//        sensor.Start_ADC(res, static_cast<gain>(sensor_gain -> GetSelection()));
//        if( (sleep_time[sensor_resolution -> GetSelection()]) < static_cast<int>(delay_meas -> GetValue()*1000))
//        Sleep(static_cast<int>(delay_meas -> GetValue()*1000));
//        else
//        Sleep(sleep_time[sensor_resolution -> GetSelection()]);

        I_data.push_back(sensor.Get_current()); //  Get_ADC
        T_data.push_back(static_cast<double>(since(start_time).count())/1000.0);
        V_data.push_back(trans_gate_start -> GetValue());



        frameworkVector -> SetData(T_data, I_data);
        framework_graph -> Fit();


//        if(current_count < screen_elements)
//        {
//
//            auto vMinmax = std::minmax_element(I_data.begin(), I_data.end(), compLess);
//            double min_ = *vMinmax.first;
//            double max_ = *vMinmax.second;
//            framework_graph -> Fit(-1*edge_x, screen_elements + edge_x, *vMinmax.first, *vMinmax.second);
//        }
//        else
//        {
//            // последние 300 только
//            auto vMinmax = std::minmax_element(I_data.end() - screen_elements, I_data.end(), compLess);
//            framework_graph -> Fit(current_count - screen_elements - edge_x, current_count + edge_x, *vMinmax.first, *vMinmax.second);
//        }


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

                sensor.Set_voltage(GATE, voltage);

                Start_ADC_wait(res, static_cast<gain>(sensor_gain  -> GetSelection()));

//                sensor.Start_ADC(res, static_cast<gain>(sensor_gain  -> GetSelection()));
//                if( (sleep_time[sensor_resolution  -> GetSelection()]) < static_cast<int> (delay_meas -> GetValue()*1000))
//                Sleep(static_cast<int>(delay_meas -> GetValue()*1000));
//                else
//                Sleep(sleep_time[sensor_resolution  -> GetSelection()]);


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

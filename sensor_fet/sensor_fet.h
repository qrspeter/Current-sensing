#ifndef SENSOR_FET_H_INCLUDED
#define SENSOR_FET_H_INCLUDED

enum terminal{
    DRAIN, GATE };

enum resolution{
    bit12 = 0, bit14 = 1, bit16 = 2, bit18 = 3};

enum gain{
    x1 = 0, x2 = 1, x4 = 2, x8 = 3};


class SENSOR_FET
{
public:
    SENSOR_FET();
    ~SENSOR_FET();

    int  Open(int);
    void Close();

	int GetPortNumber(); // Считывание номера порта.
//	void SetPortNumber(int); // Установить номер СОМ-порта.

//    int Set_ADC(int, int, int);
//    void Set_ADC(resolution, gain);
    int Reset();

//    int SetADC_range(); // при запуске устанавливать границы для опроса и корректировать измерением при измерении, видимо. Можно два - для разных ЦАП, если отличается.

//    int Set_Vdd(double);
//    int Set_Vss(double);
    int Set_dac_ref(double);
    int Set_adc_ref(double);
//    double Get_Vdd();
//    double Get_Vss();
    double Get_voltage_drain_max();
    double Get_voltage_gate_max();

    double Get_dac_ref();
    double Get_adc_ref();
    int CheckState(); // Считывание состояния счетчика, подключен (1)/отключен (0).
    void Start_ADC(resolution, gain, int channel = 0);

    int Set_voltage(terminal, double);

    double Get_current();

    void Set_shunt(double);
    double Get_shunt();

    void Set_zero_current(double);
    double Get_zero_current();

private:


    double Get_voltage();

	int com_port;// Номер подключаемого COM порта.
//    double voltage_dd = 12.00;
//    double voltage_ss = -12.06;
    double voltage_gate_max = 40.0;
    double voltage_drain_max = 12.0;
    double dac_ref = 4.98;
    double adc_ref = 2.048;

    double gate_corr_coeff = 1.0; // 0.982;
    double drain_corr_coef = 1.0; // 0.898;

    //  а как получилось что тут не надо на 2 умножать при введении смещения? потому что уже умножаем при расчете отсчетов ЦАП.
    double gate_gain = gate_corr_coeff * (1.0 + 24.0/3.9); // = 7.15
    double drain_gain =  drain_corr_coef * (1.0 + 4.7/3.9); // = 2.205

    double r_shunt = 1.0; // Ohm

    double drain_detection_coeff = 2.0;
    double drain_detection_gain = drain_detection_coeff * 0.5 * 24.0 * 8.2 / 5.0; // 19.68
//    double drain_detection_gain = 0.5 * 390.0/5.0; // For high-side sensing 5000 / 50; //100.0; 0.5 *  умножение на входном усилителе х100, но по факту выходит *1000 )))
    double drain_zero_current = 0; // 11.82; //9.6; // 0.498; // 0.997

    const int dac_counts = 4096; // 2^12

    const int adc_counts = 131072; // 2^18 если сигнал биполярный, 32768 для 16бит, 8192 для 14 бит, и 2048 для 12 бит.


    resolution ADC_res  = bit18;
    gain ADC_amp        = x1;

};


#endif // SENSOR_FET_H_INCLUDED

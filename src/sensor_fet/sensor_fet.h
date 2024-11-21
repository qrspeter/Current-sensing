#ifndef SENSOR_FET_H_INCLUDED
#define SENSOR_FET_H_INCLUDED

#include <cstdint>
#include <windows.h>

class SENSOR_FET
{
public:
    SENSOR_FET();
    ~SENSOR_FET();

    enum    terminal{DRAIN = 0, GATE = 1}; //    enum terminal {DRAIN, GATE};
    enum    resolution{bit12 = 0, bit14 = 1, bit16 = 2, bit18 = 3};
    enum    gain{x1 = 0, x2 = 1, x4 = 2, x8 = 3};
    enum    laser{LASER_OFF = 0, LASER_ON = 1};

    int     Open(int);
    void    Close();

	int     GetPortNumber(); // Считывание номера порта.
//	void SetPortNumber(int); // Установить номер СОМ-порта.

//    int Set_ADC(int, int, int);
//    void Set_ADC(resolution, gain);
    int     Reset();

//    int SetADC_range(); // при запуске устанавливать границы для опроса и корректировать измерением при измерении, видимо. Можно два - для разных ЦАП, если отличается.

//    int Set_Vdd(double);
//    int Set_Vss(double);
//    double Get_Vdd();
//    double Get_Vss();
//    double Get_voltage_drain_max();
//    double Get_voltage_gate_max();

    int     CheckState(); // Check status of the object of class, exist (1)/ not exist (0).
    int Check(); // Check answer from the Arduino
    void    Start_ADC(resolution, gain, int channel = 0);

    int     Set_voltage(terminal, double);

    double  Get_voltage();
    double  Get_current();

    void    Set_current_correction(double);
    double  Get_current_correction();

    void    Set_bias_corr(double);
    double  Get_bias_corr();

// useless without laser
    void    Laser(laser);
    void    SetPulseDuraion(double);
    void    SetPulseNumbers(int);
    void    SetPulseDelay(double);
    double  GetPulseDuraion();
    int     GetPulseNumbers();
    double  GetPulseDelay();

    double  GetDrainLimit();
    void    SetDrainLimit(double);
    double  GetGateLimit();
    void    SetGateLimit(double);



    void    SetZeroCorrMode(bool);
    bool    GetZeroCorrMode();
    void    SetAveraging(int);
    int     GetAveraging();

// unused:
    double  Get_dac_ref();
    double  Get_adc_ref();
    int     Set_dac_ref(double);
    int     Set_adc_ref(double);


private:

    bool    zero_corr_{false};
    uint8_t averaging_{1};

    double  gate_limit_{40.0}; // voltage_max
    double  drain_limit_{12.0}; // voltage_min


    int     pulse_numbers_{4};
    double  pulse_duration_{0.2};
    double  pulse_delay_{0.05};

	int     com_port_;// Номер подключаемого COM порта.
//    double voltage_dd = 12.00;
//    double voltage_ss = -12.06;
//    double voltage_gate_max = 40.0;
//    double voltage_drain_max = 12.0;
    double  dac_ref_{4.98};
    double  adc_ref_{2.048};

    double  gate_corr_coeff_{1.0}; // 0.982;
    double  drain_corr_coef_{1.0}; // 0.898;
//    double bias_correction = 0;

    //  тут не надо на 2 умножать при введении смещения потому что уже умножаем при расчете отсчетов ЦАП.
    double  gate_gain_  = gate_corr_coeff_ * (1.0 + 24.0/3.9); // = 7.15 // equation for op amp
    double  drain_gain_ = drain_corr_coef_ * (1.0 + 4.7/3.9); // = 2.205 // equation for op amp

    double  bias_correction_{-0.3}; // correction for output voltage at 0V setting


    double  current_correction_{1.088}; // real current = current_correction * (measured)
    double  current_detection_coeff{2.0}; // shifter divide voltage at half
    double  current_detection_gain = current_detection_coeff * 0.5 * 24.0 * 8.2 / (5.0 * current_correction_); // 19.68 // based on nominals from detection part
//    double drain_detection_gain = 0.5 * 390.0/5.0; // For high-side sensing 5000 / 50; //100.0; 0.5 *  умножение на входном усилителе х100, но по факту выходит *1000 )))
//    double drain_zero_current = 0; // 11.82; //9.6; // 0.498; // 0.997

    const   int dac_counts{4096}; // 2^12

    const   int adc_counts{131072}; // 2^18 если сигнал биполярный, 32768 для 16бит, 8192 для 14 бит, и 2048 для 12 бит.


    resolution  current_res{bit18};
    gain        current_gain{x1};

    uint8_t status{0b10000100}; // 14-bit

    HANDLE  hSerial = INVALID_HANDLE_VALUE;

    const   int INPUT_BUFF_SIZE{16};

    DWORD   bc;


    // operation codes // change to enum with fixed values, e.g. enum Suit { Diamonds = 5, Hearts, Clubs = 4, Spades };
    const uint8_t   resetSensor   {0};
    const uint8_t   setDAC_Gate   {1};
    const uint8_t   setDAC_Drain  {2};
    const uint8_t   setADC        {3}; // а может и не нужен, если режим задавать в команде опроса АЦП. Хотя проще период считать на компе, снимая отладку с микроконтроллера
    const uint8_t   getADC        {4};
    const uint8_t   setLaser_On   {5};
    const uint8_t   setLaser_Off  {6};
    const uint8_t   setADCav      {7};

    byte check = 0xFF;


    union
    {
      uint16_t  voltage;
      struct
      {
        uint8_t volt_0;
        uint8_t volt_1;
      };
    } dac_voltage;

    struct ADC_result
    {
        uint8_t meas_2;
        uint8_t meas_1;
        uint8_t meas_0;
        uint8_t meas_status;
    } adc;

};


#endif // SENSOR_FET_H_INCLUDED

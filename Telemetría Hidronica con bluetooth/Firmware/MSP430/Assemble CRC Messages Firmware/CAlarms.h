
class CAlarms
{
public:
  char al_hour;
  char al_min;
  char al_seg;
  //bool alarm_on;        // Habilita la alarma.
private:
  bool alarm_activated; // Habilita la activación de la alarma
public:
  CAlarms(int hour);
  ~CAlarms();
  void setAlarm(char nHour,char nMin);
  void offsetAlarm(char oHour,char oMin,char offsetHour,char offsetMin);
  void setAlarmFlag();
  void clearAlarmFlag();
  bool getAlarmFlag();
};


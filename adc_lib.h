#ifndef adc_lib_h
#define adc_lib_h

void setupADC(void);
int readADC(int ch);
float readAvgVoltage(int ch);

#endif
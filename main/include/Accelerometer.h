#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#ifdef __cplusplus
extern "C" {
#endif

void Accelerometer_Initialise(void);
void Accelerometer_Get_Accel(int16_t *x, int16_t *y, int16_t *z);


#ifdef __cplusplus
}
#endif

#endif

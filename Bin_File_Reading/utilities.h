#ifndef UTILITIES_H
#define UTILITIES_H
#define TILT 20.20
#define kDEG2RAD (3.1415926535897932384626433832795/180.0)
#define PI 3.14159265
#define DEG_2_RAD 0.0174533
#define RAD_2_DEG 57.295780

class Utilities{
public: void ab_to_azel_new(float a, float b, float *paz, float *pel, float tilt, float roll);
}
#endif // UTILITIES_H

#include "utilities.h"
#include "constants.h"
#include <math.h>

void Utilities::ab_to_azel_new(float a, float b, float *paz, float *pel, float tilt, float roll)
{
    double A1;
    double B1;
    double C1;
    double T;
    double R;
    double c;

    T = -tilt * kDEG2RAD;
    R = roll * kDEG2RAD;

    c = -sqrt(1 - a*a - b*b);

    A1 = a * cos(R) + b * sin(R) * cos(T) + c * sin(R) * sin(T);
    B1 = -a * sin(R) + b * cos(R) * cos(T) + c * cos(R) * sin(T);
    C1 = -b * sin(T) + c * cos(T);

    C1 = -C1;

    *paz = atan2(A1, C1) / kDEG2RAD;
    *pel = atan2(B1, sqrt(A1*A1 + C1*C1)) / kDEG2RAD;
}

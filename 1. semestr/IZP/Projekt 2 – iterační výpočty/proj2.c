/*
 * @file proj2.c
 * @author Ondřej Ondryáš (xondry02)
 * @date 2019-11-27
 * @brief Diode operating point calculator.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <limits.h>

// Two methods of current caluclation are possible, as in reality, the current flowing through the resistor and the diode is the same.
// If the voltage on the diode is known, we can calculate the current using Kirchhoff's law. The other option is to use the Shockley diode equation again.
// In theory, the first method may be faster in this program, because it doesn't require calculating the exponential function twice, though the difference
// will probably be immesurable. Set to 0 to use Kirchhoff or to anything else to use Shockley.
#define CURRENT_CALC_METHOD 1

// For extra small precision values, the algorithm often reaches a stall state where the mathematical operations
// don't yield any new meaningful values. Should we reach this state, a maximum possible number of iterations is defined here.
// The program returns the closest value it could calculate and a warning.
#define MAX_ITERATIONS USHRT_MAX

// The reverse saturation current of the diode.
#define I_0 1e-12
// The thermal voltage.
#define U_T 258563e-7

// This represents the function that describes the circuit with a resistor and a diode in series.
// The algorithm tries to find a value for which the value of this expression approaches zero.
#define NETWORK_F(u0, r, up) ((I_0 * (exp((up) / (U_T)) - 1)) - (((u0) - (up)) / (r)))
// Calculates the total current in the circuit based on the source voltage U_0 and voltage on the diode U_p.

#if CURRENT_CALC_METHOD == 0
#define NETWORK_CURRENT(u0, r, up) (((u0) - (up)) / (r))
#else
#define NETWORK_CURRENT(up) (I_0 * (exp((up) / (U_T)) - 1))
#endif

#define CALC_ERR_VALUE DBL_MAX

#define ARGUMENT_ERR_MSG "Error: The program expects three numerical input values: [source voltage] [resistor value] [tolerance]. \
The resistor value and tolerance must be higher than zero.\n"
#define CALC_ERR_MSG "Error: The calculation is not possible for these input values.\n"
#define PRECISION_WARN_MSG "Warning: The program wasn't able to find a result with the specified accuracy in %d iterations. Returning the closest value possible.\n"

#define CALC_ERR_CODE 1
#define ARGUMENT_COUNT_ERR_CODE 2
#define ARGUMENT_FORMAT_ERR_CODE 3

// Calculates and returns the voltage on the diode.
// Returns CALC_ERR_VALUE when any of the arguments is less than zero, or when r or eps equals zero.
// If the MAX_ITERATIONS number of iterations is reached, returns the last calulated value with a NEGATIVE SIGN,
// to indicate that it may not be accurate enough. (No other means of providing indication of such state exist
// as the method must have this exact signature, except for using global variables.)
double diode(double u0, double r, double eps)
{
    if (u0 < 0 || eps <= 0 || r <= 0)
    {
        return CALC_ERR_VALUE;
    }

    if (u0 == 0)
    {
        // When the source voltage is zero, the voltage on the diode will always be zero.
        return 0;
    }

    if (u0 == INFINITY && r == INFINITY)
    {
        // Infinity/infinity is undefined.
        return CALC_ERR_VALUE;
    }

    if (u0 == INFINITY)
    {
        // When the source voltage is "infinite", the voltage on the diode is "infinite" as well.
        return INFINITY;
    }

    if (r == INFINITY)
    {
        return 0;
    }

    double a = 0, b = u0, c = 0, c_val = u0, a_val = 0;
    unsigned int iterations = 0;

    do
    {
        c = (a + b) / 2;
        c_val = NETWORK_F(u0, r, c);

        if (c_val == 0)
        {
            return c;
        }

        a_val = NETWORK_F(u0, r, a);
        if ((c_val > 0 && a_val > 0) || (c_val < 0 && a_val < 0)) // Basically sgn(c_val) == sgn(a_val)
        {
            a = c;
        }
        else
        {
            b = c;
        }

        iterations++;
        if (iterations == MAX_ITERATIONS)
        {
            // Signalise a not accurate enough result by giving it a negative sign.
            return -c;
        }
    } while ((b - a) > eps);

    return c;
}

int main(int argc, char **argv)
{
    // Always require at least 3 arguments (argv[0] should always be the program executable path).
    if (argc < 4)
    {
        fprintf(stderr, ARGUMENT_ERR_MSG);
        return ARGUMENT_COUNT_ERR_CODE;
    }

    // Parse double inputs.
    double input_u0 = strtod(argv[1], NULL);
    double input_r = strtod(argv[2], NULL);
    double input_eps = strtod(argv[3], NULL);

    // Exclude is defined as a number which is not equal to itself by IEEE 754.
    if (input_u0 < 0 || input_r <= 0 || input_eps <= 0 ||
        input_u0 != input_u0 || input_r != input_r || input_eps != input_eps)
    {
        fprintf(stderr, ARGUMENT_ERR_MSG);
        return ARGUMENT_FORMAT_ERR_CODE;
    }

    double up = diode(input_u0, input_r, input_eps);

    if (up == CALC_ERR_VALUE)
    {
        fprintf(stderr, CALC_ERR_MSG);
        return CALC_ERR_CODE;
    }

    double ip;

    if (up < 0)
    {
        fprintf(stderr, PRECISION_WARN_MSG, MAX_ITERATIONS);
        up = -up;
    }

    if (up == INFINITY)
    {
        ip = INFINITY;
    }
    else
    {
#if CURRENT_CALC_METHOD == 0
        ip = NETWORK_CURRENT(input_u0, input_r, up);
#else
        ip = NETWORK_CURRENT(up);
#endif
    }

    printf("Up=%g V\nIp=%g A\n", up, ip);

    return 0;
}
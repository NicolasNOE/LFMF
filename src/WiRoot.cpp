#include "../include/LFMF.h"

#ifndef WIN32
#define abs fabs
#endif

/*=============================================================================
 |
 |  Description:  This routine finds the roots to the equation
 |
 |                  Wi'(ti) - q*Wi(ti) = 0
 |
 |                The parameter i selects the ith root of the equation. The 
 |                function Wi(ti) is the "Airy function of the third kind"
 |                as defined by Hufford[1] and Wait. The root is found by 
 |                iteration starting from a real root.
 |
 |                Note: Although roots that are found for W1 (Wait) and 
 |                Wi(2) (Hufford) will be equal, and the roots found for 
 |                W2 (Wait) and Wi(1) (Hufford) will be equal, the return
 |                values for *Wi and *DWi will not be the same. The input 
 |                parameters for kind and scale are used here as they 
 |                are in Airy() for consistency.
 |
 |   References:  "Airy Functions of the third kind" are found in equation 38 
 |                  of NTIA Report 87-219 "A General Theory of Radio 
 |                  Propagation through a Stratified Atmosphere", George 
 |                  Hufford, July 1987
 |
 |        Input:  i             - The ith complex root of 
 |                                Wi'(2)(ti) - q*Wi(2)(ti) starting with 1
 |                DWi           - Derivative of "Airy function of the 
 |                                third kind" Wi'(2)(ti)
 |                q             - Intermediate value -j*nu*delta
 |                Wi            - "Airy function of the third kind" Wi(2)(ti)
 |                kind          - Kind of Airy function to use
 |                scaling       - Type of scaling to use
 |
 |      Outputs:  DWi           - Derivative of "Airy function of the 
 |                                third kind" Wi'(2)(ti)
 |                Wi            - "Airy function of the third kind" Wi(2)(ti)
 |
 |      Returns:  tw            - ith complex root of the "Airy function of 
 |                                the third kind"
 |
 *===========================================================================*/
complex<double> WiRoot(int i, complex<double> *DWi, complex<double> q, complex<double> *Wi, int kind, int scaling) 
{
    complex<double> ph;             // Airy root phase 
    complex<double> ct;             // Temp
    complex<double> ti;             // the ith complex root of Wi'(2)(ti) - q*Wi(2)(ti) = 0   

    complex<double> tw;             // Return variable 
    complex<double> T;              // Temp
    complex<double> A;              // Temp

    double t, tt;                   // Temp
    double eps;                     // Temp

    int cnt;                        // Temp
    int dkind;                      // Temp

    // From the NIST DLMF (Digital Library of Mathematical Functions) 
    // http://dlmf.nist.gov/
    // The first 10 roots of Ai(Z) and Ai'(Z) can be found in: Table 9.9.1: Zeros of Ai and Ai. 
    // Note: That ak is the root of Ai(ak) while akp (ak') is the root of Ai'(akp)

    // Root of the Airy function, Ai(ak)
    // TZERO(I) in GWINT.FOR
    double akp[] = { -1.0187929716,
                     -3.2481975822,
                     -4.8200992112,
                     -6.1633073556,
                     -7.3721772550,
                     -8.4884867340,
                     -9.5354490524,
                     -10.5276603970,
                     -11.4750666335,
                     -12.3847883718,
                     -13.2636395229 };

    // Root of the derivative of Airy function, Ai'(akp)
    // TINFIN(I) in GWINT.FOR
    double ak[] = { -2.3381074105,
                    -4.0879494441,
                    -5.5205698281,
                    -6.7867080901,
                    -7.9441335871,
                    -9.0226508533,
                   -10.0401743416,
                   -11.0085243037,
                   -11.9360255632,
                   -12.8287867529,
                   -13.6914890352 };

    // Verify that the input data is correct
    // Make sure that the desired root is greater than or equal to one
    if (i <= 0) 
    {
        // There is an input parameter error; printf("WiRoot(): The root must be >= 0 (%d)\n", i);
        tw = complex<double>(-998.8, -998.8);
        return tw;
    };

    if ((scaling != HUFFORD) && (scaling != WAIT)) 
    {
        // There is an input parameter error; printf("WiRoot(): The scaling must be HUFFORD (%d) or WAIT (%d)\n", HUFFORD, WAIT);
        tw = complex<double>(-997.7, -997.7);
        return tw;
    };

    if ((kind != WTWO) && (kind != WONE)) 
    {
        // There is an input parameter error; printf("WiRoot(): The kind must be W1 (%d) or W2 (%d)\n", WONE, WTWO);
        tw = complex<double>(-996.6, -996.6);
        return tw;
    };
    // Input parameters verified

    // Initialize the Wi and Wi'(z)functions 
    *DWi = complex<double>(0.0, 0.0);   // Wi'(z)
    *Wi = complex<double>(0.0, 0.0);    // Wi(z)

    // This routine starts with a real root of the Airy function to find the complex root
    // The real root has to be turned into a complex number. 

    // ph is a factor that is used to find the root of the Wi function
    // Determine what scaling the user wants and which Wi function is used to set ph and
    // the dkind flag. This will allow that the real root that starts this process can be 
    // scaled appropriately.
    // This is the simmilar to the initial scaling that is done in Airy()
    // Note that W1 Wait = Wi(2) Hufford and W2 Wait = Wi(1) Hufford
    // So the following inequalities keep this all straight
    if ((kind == WONE && scaling == HUFFORD) || (kind == WTWO && scaling == WAIT)) 
    {
        // Wi(1)(Z) in Eqn 38 Hufford NTIA Report 87-219 or Wait W2
        ph = complex<double>(cos(-2.0*PI / 3.0), sin(-2.0*PI / 3.0));
        // Set the dkind flag
        if (scaling == WAIT) 
        {
            dkind = DWTWO;
        }
        else 
        {
            dkind = DWONE;
        };
    }
    else if ((kind == WTWO && scaling == HUFFORD) || (kind == WONE && scaling == WAIT)) 
    {
        // Wi(2)(Z) in Eqn 38 Hufford NTIA Report 87-219 or Wait W1
        ph = complex<double>(cos(2.0*PI / 3.0), sin(2.0*PI / 3.0));
        if (scaling == WAIT) 
        {
            dkind = DWONE;
        }
        else 
        {
            dkind = DWTWO;
        };
    };


    // Note: The zeros of the Airy functions i[ak'] and Ak'[ak], ak' and ak, are on the negative real axis. 
    // This is why 4*i+3 and 4*i+1 are used here instead of 4*k-3 and 4*k-1 which are
    // used in 9.9.8 and 9.9.6 in NIST DLMF. We are finding the ith negative root here. 
    if (pow(abs(q), 3.0) <= 4 * (i - 1) + 3) {
        // Small Z, use ak' as the first guess (Ak(ak') = 0)
        if (i <= 10)
        {
            // The desired root is less than 10 so it is in the array above
            tt = akp[i - 1];
        }
        else 
        {
            // The desired root is a higher order than those given in the ak array above
            // so we will approximate it from the first three terms of NIST DLMF 9.9.1.9
            // First find the argument (9.9.8) used in 9.9.1.9 for the ith negative root of Ai'(ak).
            t = (3.0 / 8.0)*PI*(4.0*(i - 1) + 1);
            tt = -1.0*pow(t, 2.0 / 3.0)*(1.0 - ((7.0 / 48.0)*pow(t, -2.0)) + ((35.0 / 288.0)*pow(t, -4.0)));
        };
        // Make the real Airy root complex
        ti = tt * ph;
        // t is now the solution for q = 0, The next step is the first Newton iteration
        ti = ti + q / ti;
    }
    else 
    {
        // Large q, use ak as the first guess (Ai'(ak) = 0)
        if (i <= 10) 
        {
            // The desired root is less than 10 so it is in the array above
            tt = ak[i - 1];
        }
        else 
        {
            // The desired root must be approximated from the first three terms of NIST DLMF 9.9.1.8
            // First find the argument (9.9.6) used in 9.9.1.8 for the ith negative root of Ai(ak).
            t = (3.0 / 8.0)*PI*(4.0*(i - 1) + 3.0);
            tt = -1.0*pow(t, 2.0 / 3.0)*(1.0 + ((5.0 / 48.0)*pow(t, -2.0)) - ((5.0 / 36.0)*pow(t, -4.0)));
        };
        ti = tt * ph;
        // t is now the solution for Z = infinity. Next step the first newton iteration
        ti = ti + 1.0 / q;
    };

    cnt = 0;        // Set the iteration counter
    eps = 0.5e-6;   // Set the error desired for the iteration

    // Now iterate by Newton's method

    //////////////////////////////////////////////////////////////////////
    // Note: We can use the following from
    // Berry and Christman "The Path Integrals of LF/VLF Wave Hop Theory"
    // Radio Science Vol. 69D, No. 11 , November 1965
    // Eqn (14) E(t)  = W2'(t) - q W2(t)
    // Eqn (39) E'(t) = t W2(t) - q W2'(t)
    //////////////////////////////////////////////////////////////////////

    do {

        // f(q) = Wi'(ti) - q*Wi(ti) 
        *Wi = Airy(ti, kind, scaling);
        // f'(q) = tw*Wi(ti) - q*Wi'(ti);
        *DWi = Airy(ti, dkind, scaling);
        // The Newton correction factor for iteration f(q)/f'(q)
        A = (*DWi - q * (*Wi)) / (ti*(*Wi) - q * (*DWi));
        ti = ti - A;    // New root guess ti
        cnt++;          // Increment the counter

    } while ((cnt <= 25) && ((abs((A / ti).real()) + (abs((A / ti).imag())) > eps)));

    // Check to see if there if the loop converged on an answer
    if (cnt == 26) // The cnt that fails is an arbitrary number most converge in ~5 tries
    { 
        // No Convergence return 0 + j*0 as the root as TW() did
        tw = complex<double>(0.0, 0.0);
    }
    else 
    {
        // Converged!
        tw = ti;
    };

    return tw;
};

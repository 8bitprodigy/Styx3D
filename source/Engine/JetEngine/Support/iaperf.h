// IAPERF.H

// Author: Michael Stoner
// Company: Intel
// Page: http://www.intel.com/cd/ids/developer/asmo-na/eng/microprocessors/itanium/19949.htm?page=3

#include <stdio.h>

// RDTSC functions
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux__
    #ifdef __ia64__
        #ifdef __INTEL_COMPILER

            #ifdef __cplusplus
                extern "C" { unsigned __int64 __getReg(int whichReg); }
            #else
                unsigned __int64 __getReg(int whichReg);
            #endif

            #pragma intrinsic(__getReg)

            #define INL_REGID_APITC 3116

            unsigned __int64 rdtsc()
            {
                volatile unsigned __int64 temp;

                temp = __getReg(INL_REGID_APITC);

                return temp;
            }

        #else // __INTEL_COMPILER

            // Assume 64-bit gcc
            unsigned long rdtsc()
            {
                volatile unsigned long temp;

                __asm__ __volatile__("mov %0=ar.itc" : "=r"(temp) :: memory");

                return temp;
            }

    #endif // __INTEL_COMPILER

    #else // __ia64__

        // Assume IA32, gcc or Intel Compiler

        unsigned long long rdtsc()
        {
            unsigned long long temp;

            __asm__ __volatile__ (
                    "cpuid\n\t"
                    "rdtsc\n\t"
                    "leal %0, %%ecx\n\t"
                    "movl %%eax, (%%ecx)\n\t"
                    "movl %%edx, 4(%%ecx)\n\t"
                    :
                    : "m" (temp)
                    : "%eax", "%ebx", "%ecx", "%edx");

            return temp;
        }
    #endif // __ia64__

#else // __linux__

    #ifdef WIN64
        // Assume Microsoft or Intel Compiler

        #ifdef __cplusplus
            extern "C" { unsigned __int64 __getReg(int whichReg); }
        #else
            unsigned __int64 __getReg(int whichReg);
        #endif

        #pragma intrinsic(__getReg)

        #define INL_REGID_APITC 3116

        unsigned __int64 rdtsc()
        {
            volatile unsigned __int64 temp;

            temp = __getReg(INL_REGID_APITC);

            return temp;
        }


    #else // WIN64
        // Assume WIN32 with Microsoft or Intel Compiler

        unsigned __int64 rdtsc()
        {
            volatile unsigned __int64 temp;

            _asm cpuid;
            _asm rdtsc;

            _asm lea ecx, temp;
            _asm mov [ecx], eax;
            _asm mov [ecx+4], edx;

            return temp;

        }
    #endif // WIN64
#endif // __linux__


// The IAperf Macros
#ifdef __linux__
    #define PERFINITMHZ(clkspd) \
        unsigned long long clocks; \
        double clockspeed = (unsigned long long)clkspd * 1000000;

    #define PERFREPORT \
        Log_Printf("time elapsed = %f sec\n", ((double)clocks)/clockspeed);

    #else
		#include "log.h"
        #define PERFINITMHZ(clkspd)\
            unsigned __int64 clocks; \
            double clockspeed = (double)1000000 * clkspd;


        #define PERFREPORT \
            Log_Printf("time elapsed = %f sec\n", ((double)clocks)/clockspeed);
#endif


#define PERFSTART clocks = rdtsc();
#define PERFSTOP clocks = rdtsc() - clocks;

//
// Created by artypoole on 18/07/24.
//

#include "CPUID.h"


#include <Terminal.h>
#include "types.h"
#include "logging.h"

u8 binaryNum[32];
u8 decimal[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};


cpuid_manufacturer_info_t cpu_manufacturer_info;
cpuid_ext_manufacturer_info_t cpu_ext_manufacturer_info;
cpuid_core_frequency_info_t cpu_frequency_info;


void decToBinary(u32 n)
{
    // counter for binary array
    int i = 0;
    while (n > 0)
    {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }
}


// In general, move a requested leaf into eax
// call cpuid
// read registers out into variables
// parse the info.

//todo: put this data into structs for easier inspection for each leaf.

cpuid_manufacturer_info_t* cpuid_get_manufacturer_info()
{
    asm volatile("mov $0x0, %eax\n\t");
    asm volatile("cpuid\n\t":"=a"(cpu_manufacturer_info.max_param), "=b" (cpu_manufacturer_info.ebx), "=d" (cpu_manufacturer_info.edx), "=c" (cpu_manufacturer_info.ecx));
    LOG("CPUID Manufacturer ID: ", cpu_manufacturer_info.manufacturer_ID);
    return &cpu_manufacturer_info;
}

cpuid_ext_manufacturer_info_t* cpuid_print_ext_manufacturer_info()
{
    asm volatile("mov $0x80000000, %eax");
    asm volatile("cpuid":"=a"(cpu_ext_manufacturer_info.max_ext_param));

    // brand string
    // todo: replace with other getter/setter methods.
    if (cpu_ext_manufacturer_info.max_ext_param - 0x80000000 >= 4)
    {
        u32 parts[12];
        asm volatile("mov $0x80000002, %eax");
        asm volatile("cpuid":"=a"(parts[0]), "=b" (parts[1]), "=c" (parts[2]), "=d" (parts[3]));
        asm volatile("mov $0x80000003, %eax");
        asm volatile("cpuid":"=a"(parts[4]), "=b" (parts[5]), "=c" (parts[6]), "=d" (parts[7]));
        asm volatile("mov $0x80000004, %eax");
        asm volatile("cpuid":"=a"(parts[8]), "=b" (parts[9]), "=c" (parts[10]), "=d" (parts[11]));
        WRITE("Brand string: ");
        for (size_t i = 0; i < 12; i++)
        {
            for (size_t c = 0; c < 32 / 8; c++)
            {
                char letter = static_cast<char>(parts[i] >> (c * 8));
                if (letter == 0) { break; }
                WRITE(letter);
            }
        }
        NEWLINE();
    }


    return &cpu_ext_manufacturer_info;
}


void cpuid_print_feature_info()
{
    u32 info[4]; // family info, addiitonal features, featureflag2, featureflag1
    __asm__("mov $0x1 , %eax");
    asm volatile("cpuid":"=a"(info[0]), "=b" (info[1]), "=c" (info[2]), "=d" (info[3]));

    for (size_t i = 0; i < 4; i++)
    {
        decToBinary(info[i]);
        LOG("Feature info ", i, ": ");
        for (size_t c = 0; c < 32; c++)
        {
            WRITE(decimal[binaryNum[c]]);
        }
        NEWLINE();
    }
}


cpuid_core_frequency_info_t* cpuid_get_frequency_info()
{
    if (cpu_manufacturer_info.max_param < 0x15)
    {
        LOG("ERROR: Cannot get timing info. CPUID feature not supported.");
        return 0;
    }
    asm volatile ("mov $0x15 , %eax");
    asm volatile("cpuid":"=a"(cpu_frequency_info.tsc_ratio_denom), "=b" (cpu_frequency_info.tsc_ratio_numer), "=c" (cpu_frequency_info.core_clock_freq_hz));
    LOG(
        "Raw freq info: tsc_ratio_denom",
        cpu_frequency_info.tsc_ratio_denom,
        " tsc_ratio_numer: ",
        cpu_frequency_info.tsc_ratio_numer,
        "core_clock_freq_hz",
        cpu_frequency_info.core_clock_freq_hz);
    asm volatile ("mov $0x16, %eax");
    asm volatile("cpuid":"=a"(cpu_frequency_info.cpu_base_freq_MHz), "=b" (cpu_frequency_info.cpu_max_freq_MHz), "=c" (cpu_frequency_info.bus_ref_freq_MHz));
    LOG(
        "Raw freq info: cpu_base_freq_MHz",
        cpu_frequency_info.cpu_base_freq_MHz,
        " cpu_max_freq_MHz: ",
        cpu_frequency_info.cpu_max_freq_MHz,
        "bus_ref_freq_MHz",
        cpu_frequency_info.bus_ref_freq_MHz);
    if (cpu_frequency_info.tsc_ratio_denom > 0 && cpu_frequency_info.core_clock_freq_hz > 0)
    {
        LOG("LEAF 0x15 EBX and ECX present. Calculating tsc freq.");
        cpu_frequency_info.tsc_freq = (cpu_frequency_info.core_clock_freq_hz * cpu_frequency_info.tsc_ratio_numer) / cpu_frequency_info.tsc_ratio_denom;
    }
    if (cpu_frequency_info.core_clock_freq_hz == 0 && cpu_frequency_info.tsc_ratio_denom > 0)
    {
        LOG("LEAF 0x15 not full. Calculating core_clock_freq.");
        cpu_frequency_info.tsc_freq = cpu_frequency_info.cpu_base_freq_MHz;
        cpu_frequency_info.core_clock_freq_hz = (cpu_frequency_info.cpu_base_freq_MHz * 10000000) * (cpu_frequency_info.tsc_ratio_denom * cpu_frequency_info.tsc_ratio_numer);
    }
    return &cpu_frequency_info;
}

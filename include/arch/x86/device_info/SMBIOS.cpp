//
// Created by artypoole on 19/07/24.
//

#include "SMBIOS.h"
#include "string.h"
// #include "Serial.h"

#include "logging.h"
#define LENGTH = 5;
#define MAJOR_VERSION = 6;
#define MINOR_VERSION = 7;


smbios_t* smbios = nullptr;
smbios_processor_info_t* cpu_info = nullptr;
u64 clock_rate = 0;


smbios_t search_for_SMBIOS()
{
    u8 checksum = 0;
    auto eps = reinterpret_cast<u8*>(0x000F0000);

    while (eps <= reinterpret_cast<u8*>(0x000FFFFF))
    {
        /* Check Anchor String (32-bit version) */
        if (const char magic[4] = {'_','S','M','_'}; !memcmp(eps, magic, 4))
        {
            const size_t length = eps[5];
            checksum = 0;

            /* Add all bytes */
            for (size_t i = 0; i < length; i++)
                checksum += eps[i];

            if (checksum == 0)
                /* Done! */
                break;
        }

        /* Next 16-byte-aligned address */
        eps += 16;
    }

    // LOG(reinterpret_cast<u32>(eps));
    smbios = reinterpret_cast<smbios_t*>(eps);
    return *smbios; // return a copy
}


size_t find_real_len(smbios_header_t* hd)
{
    // There are strings which occur after the data table. This searches for the end of those and returns the real length
    size_t i;
    const u8* strtab = reinterpret_cast<u8*>(hd) + hd->length;
    // Scan until we find a double zero byte
    for (i = 1; strtab[i - 1] != '\0' || strtab[i] != '\0'; i++);
    return hd->length + i + 1;
}

void SMBIOS_populate_cpu_info()
{

    if (smbios == NULL)
    {
        search_for_SMBIOS();
    }
    auto address = reinterpret_cast<u8*>(smbios->structure_table_address);
    for (size_t i = 0; i < smbios->n_structures; i++)
    {
        // Load header
        auto* header = reinterpret_cast<smbios_header_t*>(address);

        if (header->type == 4)
        {
            LOG("header found at: ", reinterpret_cast<u32>(header));
            auto* processor_info = reinterpret_cast<smbios_processor_info_t*>(header);
            if (processor_info->processor_type == 3) // cpu
            {
                cpu_info = processor_info;
            }
            else
            {
                LOG("other processor type detected: ", processor_info->processor_type);
            }
        }
        // Go to next header
        header = reinterpret_cast<smbios_header_t*>(address);
        const size_t real_len = find_real_len(header);
        address += real_len;
    }
}

u64 SMBIOS_get_CPU_clock_rate_hz()
{
    if (clock_rate!=0) return clock_rate;
    if (cpu_info == NULL)
    {
        SMBIOS_populate_cpu_info();
    }
    clock_rate = static_cast<u64>(cpu_info->current_speed) * 1000000;
    return clock_rate;
}





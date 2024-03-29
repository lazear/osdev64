#ifndef _SYSCONFIG_H
#define _SYSCONFIG_H


#define CONFIG_SSE 	 	 1
#define CONFIG_CPU_COUNT 2
#define CONFIG_X2APIC 	 3
#define CONFIG_PCI 	 	 4
#define CONFIG_HEAP 	 5
#define CONFIG_CPUID 	 6
#define CONFIG_BOOTTIME  7
#define CONFIG_MAX	128

int config_set(int config, size_t value);
int config_get(int config, size_t *value);

#endif
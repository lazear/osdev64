#ifndef _SYSCONFIG_H
#define _SYSCONFIG_H

#define CONFIG_SSE 	 	1
#define CONFIG_X2APIC 	2
#define CONFIG_PCI 	 	3
#define CONFIG_HEAP 	4
#define CONFIG_MAX	128

int config_set(int config, size_t value);
int config_get(int config, size_t *value);

#endif
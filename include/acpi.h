
#ifndef __ACPI__
#define __ACPI__

#include <stdint.h>

struct rsdp_header {
	char signature[8];
	uint8_t checksum;
	char oemid[6];
	uint8_t rev;
	uint32_t rsdt_ptr;		
	uint32_t length;
	uint64_t xsdt_ptr;
	uint8_t extchecksum;
	uint8_t pad[3];
}  __attribute__((packed));


struct acpi_header {
	char signature[4];
	uint32_t length;
	uint8_t rev;
	uint8_t checksum;
	char oemid[6];
	char oemtableid[8];
	uint32_t oemrev;
	uint32_t creatorid;
	uint32_t creatorrev;
}  __attribute__((packed));

struct rsdt_header {
	struct acpi_header h;
	uint32_t tableptrs[];
}  __attribute__((packed));


struct acpi_lapic {
	uint8_t type;			/* ==0; */
	uint8_t rec_len;
	uint8_t acpi_proc_id;
	uint8_t apic_id;
	uint32_t flags;
}  __attribute__((packed));

struct acpi_x2apic {
	uint8_t type;			/* ==9; */
	uint8_t rec_len;
	uint16_t reserved;
	uint32_t apic_id;
	uint32_t flags;
	uint32_t uid;
}  __attribute__((packed));

struct acpi_ioapic {
	uint8_t type;			/* ==1 */
	uint8_t rec_len;
	uint8_t ioapic_id;
	uint8_t res;
	uint32_t ioapic_addr;
	uint32_t gsib;			/* Global System Interrupt Bus */
}  __attribute__((packed));

struct acpi_iso {
	uint8_t type;		/* ==2 */
	uint8_t rec_len;	/* Record len*/
	uint8_t bus_src;	/*Bus source*/
	uint8_t irq_src;	/*IRQ source*/
	uint8_t gsi;		/* Global System Interrupt*/
	uint16_t flags;
}  __attribute__((packed));		/*Int. source override*/

struct  madt_header {
	struct acpi_header h;
	uint32_t lca;		/* local controller address */
	uint32_t flags;
}  __attribute__((packed));

struct fadt_header {
	struct acpi_header h;
	uint32_t firmware_ctrl;		/* physical memory address of FACS */
	uint32_t dsdt; 
	uint8_t res;
	uint8_t preferred_profile;	/* preferred power management profile */
	uint16_t sci_int;	/* system vector the SCI interrupt is wired to in 8259 mode */
	uint32_t smi_cmd;	/* system port address of SMI command port */
	uint8_t acpi_enable;	/* value to write to SMI_CMD */
	uint8_t acpi_disable;	/* value to write to SMI_CMD */

	/* more... */
};



extern int acpi_init(void);

#endif
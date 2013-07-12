
typedef struct 
{
	uchar reg:8;
	uchar func:3;
	uchar dev:5;
	uchar bus:8;
	uchar rsvd:7;
	uchar enable:1;
} confadd;

typedef struct 
{
	uint16 vendor_id;
	uint16 device_id;
	
	uint16 command;
	uint16 status;
	
	uint8 revision_id;
	uint8 interface;
	uint8 sub_class;
	uint8 base_class;
	
	uint8 cache_line_size;
	uint8 latency_timer;
	uint8 header_type;
	uint8 bist;	
} pci_cfg;



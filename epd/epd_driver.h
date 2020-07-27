#ifndef EPD_DRIVER_H
#define EPD_DRIVER_H


#include<utils/pgm.h>


/* SCSI Generic ----------------------------------------------------------------
*/
typedef unsigned char sg_command;
typedef unsigned char sg_data;


// SCSI operation codes (used as the first byte of an CommandDescriptorBlock).
enum sg_opcode
{
  SG_OP_INQUIRY = 0x12,
  SG_OP_CUSTOM = 0xFE
};


/* IT8951 EPD Driver -----------------------------------------------------------
*/
typedef struct
{
  int address;
  int update_mode;              // get from GetSysResponse.uiMode
  int x;
  int y;
  int width;
  int height;
  int wait_display_ready;       // set to 1 to enable
} epd_display_area_args_addr;


typedef struct
{
  int address;
  int x;
  int y;
  int width;
  int height;
  unsigned char pixels[];
} epd_load_image_args_addr;


// TODO: I believe this mapping is wrong (for my 9.7in Waveshare panel
// at least). I got the mapping from the following sources:
// - https://www.waveshare.net/wiki/File:E-paper-mode-declaration.pdf
// - https://github.com/waveshare/IT8951-ePaper/blob/de567d4fcae15925308483a49be7e93d9d9d29b9/lib/e-Paper/EPD_IT8951.c#L34 (partially)
// - https://github.com/GregDMeyer/IT8951/blob/435b6e18c29cc26318d375f236a3490a7794731f/IT8951/constants.py#L46
//
// Interestingly, these do seem match my experimental guesses from the
// Python prototype (find them by looking at epd_update_mode's
// definition in prior commits).
//
// I think I just need to test and find out. In particular, I want to
// find DU4 since this seems like the best set of trade-offs for my
// use cases: you've got white, light grey, dark grey and black
//            plus refreshes are 260 ms which is pretty ok,
//            and claim it to be non-flashy.
//
// A4 would be great as well (I can do a quick scan to check if the
// diff'd area is all black or white, then use A4).
//
// I believe there is a no-flash, 16-level mode which could be useful.

enum epd_update_mode
{
  EPD_UPD_RESET = 0,
  EPD_UPD_DU = 1,
  EPD_UPD_GC16 = 2,
  EPD_UPD_GL16 = 3,
  EPD_UPD_GLR16 = 4,
  EPD_UPD_GLD16 = 5,
  EPD_UPD_A2 = 6,
  EPD_UPD_DU4 = 7,
};

static const enum epd_update_mode EPD_ONE_BIT_MODES[] =
  { EPD_UPD_DU, EPD_UPD_A2 };
static const enum epd_update_mode EPD_ONE_BIT_LEVELS[] = { 0 * 8, 30 * 8 };

static const enum epd_update_mode EPD_TWO_BIT_MODES[] = { EPD_UPD_DU4 };
static const enum epd_update_mode EPD_TWO_BIT_LEVELS[] =
  { 10 * 8, 20 * 8, 30 * 8, 0 * 8 };

static const enum epd_update_mode EPD_FOUR_BIT_MODES[] =
  { EPD_UPD_GC16, EPD_UPD_GL16, EPD_UPD_GLR16, EPD_UPD_GLD16 };
static const enum epd_update_mode EPD_FOUR_BIT_MODES[] = {
  2 * 8, 4 * 8, 6 * 8, 8 * 8, 10 * 8, 12 * 8, 14 * 8, 16 * 8, 18 * 8, 20 * 8,
  22 * 8, 24 * 8, 26 * 8, 28 * 8, 30 * 8, 0 * 8
};

// Fill this code to CDB[6].
enum epd_opcode
{
  EPD_OP_GET_SYS = 0x80,
  EPD_OP_READ_MEM = 0x81,
  EPD_OP_WRITE_MEM = 0x82,
  EPD_OP_DPY_AREA = 0x94,
  EPD_OP_LD_IMG_AREA = 0xA2,
  EPD_OP_PMIC_CTRL = 0xA3,
  EPD_OP_FAST_WRITE_MEM = 0xA5,
  EPD_OP_AUTO_RESET = 0xA7,

  // Following are not listed in the docs, but found in the example
  // code. Danger?
  EPD_OP_USB_SPI_ERASE = 0x96,
  EPD_OP_USB_SPI_READ = 0x97,
  EPD_OP_USB_SPI_WRITE = 0x98,
  EPD_OP_IMG_CPY = 0xA4,        // Not used in Current Version (IT8961 Samp only)
  EPD_OP_READ_REG = 0x83,
  EPD_OP_WRITE_REG = 0x84,
  EPD_OP_FSET_TEMP = 0xA4,
};

// TODO: epd_info struct is wrong length
typedef struct
{
  unsigned int standard_cmd_number;
  unsigned int extend_cmd_number;
  unsigned int signature;
  unsigned int version;
  unsigned int width;
  unsigned int height;
  unsigned int update_buffer_address;
  unsigned int image_buffer_address;
  unsigned int temperature_segment_number;
  unsigned int display_modes_count;
  unsigned int display_mode_frame_counts[8];
  unsigned int image_buffers_count;
  unsigned int wbf_sfi_address;
  unsigned int reserved[9];
  void *cmd_info_data[1];
} epd_info;


enum epd_state
{ EPD_INIT, EPD_READY, EPD_BUSY };


typedef struct
{
  int fd;
  int state;
  unsigned int max_transfer;
  epd_info info;
} epd;


typedef struct
{
  unsigned char sg_op;
  unsigned char zero0;
  unsigned int address;
  unsigned char epd_op;
  unsigned short length;
  unsigned char zero1;
  unsigned char zero2;
  unsigned char zero3;
  unsigned char zero4;
  unsigned char zero5;
  unsigned char zero6;
  unsigned char zero7;
} __attribute__((__packed__)) epd_fast_write_command;


int epd_fast_write_mem(
  epd * display,
  pgm * image
);


int epd_transfer_image(
  epd * display,
  unsigned int x,
  unsigned int y,
  pgm * image
);


int epd_draw(
  epd * display,
  unsigned int x,
  unsigned int y,
  pgm * image,
  enum epd_update_mode update_mode
);


int epd_reset(
  epd * display
);


epd *epd_init(
  char path[]
);


#endif

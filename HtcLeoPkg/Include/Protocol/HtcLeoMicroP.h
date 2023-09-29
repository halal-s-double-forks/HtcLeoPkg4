#ifndef __HTCLEO_ROTOCOL_I2C_MICROP_H__
#define __HTCLEO_ROTOCOL_I2C_MICROP_H__


#define HTCLEO_MICROP_PROTOCOL_GUID                                             \
  {                                                                            \
    0x2c898318, 0x41c1, 0x4309,                                                \
    {                                                                          \
      0x89, 0x8a, 0x2f, 0x55, 0xc8, 0xcf, 0x0b, 0x86                           \
    }                                                                          \
  }

typedef struct _HTCLEO_MICROP_PROTOCOL HTCLEO_MICROP_PROTOCOL;

typedef INTN(*microp_i2c_write_t)(UINT8 addr, UINT8 *cmd, int lengt);


struct _HTCLEO_MICROP_PROTOCOL {
  microp_i2c_write_t  Write;
};

extern EFI_GUID gHtcLeoMicropProtocolGuid;

#endif
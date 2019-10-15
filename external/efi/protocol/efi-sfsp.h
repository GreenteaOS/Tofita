/*
 * efi-sfsp.h
 *
 * UEFI Simple File System Protocol.
 */

#ifndef __EFI_SFSP_H
#define __EFI_SFSP_H

#include <efi.h>
#include <protocol/efi-fp.h>


#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION    0x00010000

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID        {0x0964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}


struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef EFI_STATUS (*EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME)(struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,  EFI_FILE_PROTOCOL **Root);


typedef struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
    UINT64                                      Revision;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;


#endif /* __EFI_SFSP_H */

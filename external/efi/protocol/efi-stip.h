/*
 * efi-stip.h
 *
 * The UEFI Simple Text Input Protocol.
 */

#ifndef __EFI_STIP_H
#define __EFI_STIP_H

#include <efi.h>


#define EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID {0x387477c1, 0x69c7, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}


typedef struct EFI_INPUT_KEY {
    UINT16  ScanCode;
    UINT16  UnicodeChar;
} EFI_INPUT_KEY;

struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

typedef EFI_STATUS (*EFI_INPUT_RESET)(struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, BOOLEAN ExtendedVerification);
typedef EFI_STATUS (*EFI_INPUT_READ_KEY)(struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, EFI_INPUT_KEY *Key);

typedef struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    EFI_INPUT_RESET     Reset;
    EFI_INPUT_READ_KEY  ReadKeyStroke;
    EFI_EVENT           WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;


#endif /* __EFI_STIP_H */

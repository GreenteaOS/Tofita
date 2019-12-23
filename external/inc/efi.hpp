/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    efi.h

Abstract:

    Public EFI header files



Revision History

--*/


// Add a predefined macro to detect usage of the library
#ifndef _GNU_EFI
#define _GNU_EFI
#endif

//
// Build flags on input
//  EFI32
//  EFI_DEBUG               - Enable debugging code
//  EFI_NT_EMULATOR         - Building for running under NT
//


#ifndef _EFI_INCLUDE_
#define _EFI_INCLUDE_

#define EFI_FIRMWARE_VENDOR         L"INTEL"
#define EFI_FIRMWARE_MAJOR_REVISION 12
#define EFI_FIRMWARE_MINOR_REVISION 33
#define EFI_FIRMWARE_REVISION ((EFI_FIRMWARE_MAJOR_REVISION <<16) | (EFI_FIRMWARE_MINOR_REVISION))

#include "efibind.hpp"
#include "eficompiler.hpp"
#include "efidef.hpp"
#include "efidevp.hpp"
#include "efipciio.hpp"
#include "efiprot.hpp"
#include "eficon.hpp"
#include "efiser.hpp"
#include "efi_nii.hpp"
#include "efipxebc.hpp"
#include "efinet.hpp"
#include "efiapi.hpp"
#include "efifs.hpp"
#include "efierr.hpp"
#include "efiui.hpp"
#include "efiip.hpp"
#include "efiudp.hpp"
#include "efitcp.hpp"
#include "efipoint.hpp"
#include "efisetjmp.hpp"

#endif

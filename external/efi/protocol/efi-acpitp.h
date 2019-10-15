/*
 * efi-acpitp.h
 *
 * UEFI ACPI table protocol.
 */

#ifndef __EFI_ACPITP_H
#define __EFI_ACPITP_H

#include <efi.h>


#define EFI_ACPI_TABLE_PROTOCOL_GUID    {0xffe06bdd, 0x6107, 0x46a6, {0x7b, 0xb2, 0x5a, 0x9c, 0x7e, 0xc5, 0x28, 0x5c}}


struct EFI_ACPI_TABLE_PROTOCOL;

typedef EFI_STATUS (*EFI_ACPI_TABLE_INSTALL_ACPI_TABLE)(struct EFI_ACPI_TABLE_PROTOCOL *This, VOID *AcpiTableBuffer, UINTN AcpiTableBufferSize, UINTN *TableKey);
typedef EFI_STATUS (*EFI_ACPI_TABLE_UNINSTALL_ACPI_TABLE)(struct EFI_ACPI_TABLE_PROTOCOL *This, UINTN  TableKey);

typedef struct EFI_ACPI_TABLE_PROTOCOL {
    EFI_ACPI_TABLE_INSTALL_ACPI_TABLE   InstallAcpiTable;
    EFI_ACPI_TABLE_UNINSTALL_ACPI_TABLE UninstallAcpiTable;
} EFI_ACPI_TABLE_PROTOCOL;


#endif /* __EFI_ACPITP_H */

# efi

## UEFI headers

Assumes X64.

Requires <stdint.h>.

These are incomplete and you should probably use the official headers from [Tianocore](https://www.tianocore.org/).

You could build a "Hello World" application just by including <efi.h> and using the declarations there. Everything needed to reference the *Boot Services*, *Runtime Services*, *Configuration Table*, *System Table* and base types are included in <efi.h>. 

Using protocols requires including the appropriate protocol header. The naming convention for the include filename is to use the name of the structure, as it appears in the UEFI specification, and naming the file using the first letter of each component of the structure name. For example, the *EfiGraphicsOutputProtocol* is defined in *protocol/efi-gop.h*.

Please let me know of any mistakes. I'm also willing to add things as a favor. As it stands, I've only included components that I'm actually using.

<dl>
<dt><b>efi-bs.h</b></dt>
    <dd>boot services</dd>
<dt><b>efi-ct.h</b></dt>
    <dd>configuration table</dd>
<dt><b>efi.h</b></dt>
    <dd>base system</dd>
<dt><b>efi-rs.h</b></dt>
    <dd>runtime services</dd>
<dt><b>efi-st.h</b></dt>
    <dd>system table</dd>
<dt><b>efi-time.h</b></dt>
    <dd>time representation</dd>
<dt><b>protocol/</b></dt>
    <dd>efi protocols</dd>
<dt><b>protocol/efi-acpitp.h</b></dt>
    <dd>acpi table protocol</dd>
<dt><b>protocol/efi-dpp.h</b></dt>
    <dd>device path protocol</dd>
<dt><b>protocol/efi-fp.h</b></dt>
    <dd>file protocol</dd>
<dt><b>protocol/efi-gop.h</b></dt>
    <dd>graphics output protocol</dd>
<dt><b>protocol/efi-lidpp.h</b></dt>
    <dd>loaded image device path protocol</dd>
<dt><b>protocol/efi-lip.h</b></dt>
    <dd>loaded image protocol</dd>
<dt><b>protocol/efi-sfsp.h</b></dt>
    <dd>simple file system protocol</dd>
<dt><b>protocol/efi-stip.h</b></dt>
    <dd>simple text input protocol</dd>
<dt><b>protocol/efi-stop.h</b></dt>
    <dd>simple text output protocol</dd>
</dl>

---
layout: wiki
title: Introduction
into: intro
---

GreenteaOS features own kernel [Tofita](https://github.com/GreenteaOS/Tofita): a non-portable, strictly x86-64 desktop-oriented monolithic design with UEFI bootloader.

GreenteaFS is also considered to be used as a primary file system. With hybrid approach of CoW and non-CoW structure to achieve maximal performance on consumer desktop PCs (in comparison to most other fs's designed for server use). Sources of GreenteaFS are not yet available due to active experimentation. Somewhat influenced by TFS of Redox. Intended to be very compatible with NTFS in API terms (like streams support and unicode file names).

GUI is, of course, intended to be hardware accelerated in the future, with GPU drivers as parts of the kernel (i.e. non-userspace). Only support for AMD GCN1+ and DX12-capable Intel is planned, with Nvidia being still closed source and excluded from support. Support for the Windows NT GPU drivers is not planned.

Safety to be provided with user (personal) space and apps isolation. Think of a portable user folders and apps in wine-prefix manner.

Kernel will be rewritten in Hexa language when that will be mature enough.

---
layout: wiki
title: Notes on CPU compatibility
---

List of required features. Note: `wide support` means *both* AMD and Intel.

- Greentea OS targets CPUs **at least** from 2009 and newer
- `SSE2` is mandatory on AMD64
- `NX` bit should be present on all AMD64, but may be disabled, does not affect system working
- `SSE3` wide support since 2005
- `CMPXCHG16B` wide support since 2006
- `PrefetchW` wide support since 2006
- `LAHF/SAHF` wide support since 2005
- `POPCNT` wide support since 2008
- `2 MB` huge pages seems to be mandatory on AMD64

Not required features:

- `SSE4.1` wide support since 2011 and **not used by Greentea**
- `SSSE3` wide support since 2011 and **not used by Greentea**
- `SSE4a` seems like AMD-only and **not used by Greentea**
- `1 GB` huge pages (aka `PDPE1GB`) wide support since 2010 and **not used by Greentea**

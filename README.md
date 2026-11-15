<div align="center">

<img src="packaging/icons/256x256/lgl-eacv2-checker.png" width="128" alt="LGL EAC v2 Checker">

# LGL EAC v2 Checker

**Check detectable Fedora boot security requirements associated with Easy Anti-Cheat Linux v2.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/license/mit)
[![Fedora](https://img.shields.io/badge/Fedora-45-blue?logo=fedora&logoColor=white)](https://fedoraproject.org)
[![GTK](https://img.shields.io/badge/GTK-4-blue?logo=gtk&logoColor=white)](https://www.gtk.org)
[![C++](https://img.shields.io/badge/C++-20-blue?logo=cplusplus&logoColor=white)](https://isocpp.org)
[![COPR](https://img.shields.io/badge/COPR-lgl--toolkit-blue?logo=fedora&logoColor=white)](https://copr.fedorainfracloud.org/coprs/linuxgamerlife/lgl-toolkit/)

<p align="center">
  <a href="https://ko-fi.com/G2G3V70LW">
    <img src="https://storage.ko-fi.com/cdn/kofi6.png?v=6" height="36" alt="Support on Ko-fi" />
  </a>
</p>
</div>

---

## Important beta warning

LGL EAC v2 Checker is beta software. Its checks are based on the
[CachyOS Easy Anti-Cheat documentation](https://wiki.cachyos.org/configuration/easy_anti_cheat/)
and have been adapted for Fedora.

No Easy Anti-Cheat Linux v2 games are currently available to validate these
results against. A passing result does not prove that a game will launch, that a
publisher supports Fedora, or that Epic trusts the current boot chain.

Some suggested changes affect firmware, Secure Boot, the bootloader, kernel
arguments, kernel modules, and Linux security features. A mistake may prevent
Fedora from starting. Read every instruction before copying a command. Do not
make a change unless you understand it and are prepared to recover the system
from a TTY, rescue environment, or live USB.

The application displays this warning at every launch. Checks begin only after
the user selects **Continue at My Own Risk**.

---

## Overview

LGL EAC v2 Checker is a native GTK 4 diagnostic application for Fedora. It
examines selected properties of the current boot and explains whether each
detectable requirement passes, fails, needs attention, or cannot be determined.

The application is a checker, not an installer or configuration tool:

- It reads the minimum system information needed for each check.
- It runs as the normal desktop user.
- It never changes firmware, boot settings, kernel arguments, packages, or
  system files automatically.
- It never invokes commands with administrator privileges.
- It provides complete, copyable commands only as optional guidance.
- It explains what a command does and how to interpret the result.
- It treats missing or inaccessible information as unknown, never as a pass.
- It does not send telemetry or system information over the network.

The current application version is **0.1.0 beta**.

---

## Install

LGL EAC v2 Checker is published through the
[LGL Toolkit COPR](https://copr.fedorainfracloud.org/coprs/linuxgamerlife/lgl-toolkit/):

```bash
sudo dnf copr enable linuxgamerlife/lgl-toolkit
sudo dnf install lgl-eacv2-checker
```

Launch **LGL EAC v2 Checker** from the application menu or run:

```bash
lgl-eacv2-checker
```

The source code and issue tracker are available at
[github.com/linuxgamerlife/lgl-eacv2-checker](https://github.com/linuxgamerlife/lgl-eacv2-checker).

---

## Current checks

| Check | What it examines |
|---|---|
| **Fedora platform** | Reads `/etc/os-release` and confirms that the current system identifies itself as Fedora with a version |
| **Fedora kernel** | Confirms that the running kernel image belongs to Fedora's standard `kernel-core` package and that RPM reports no changed or missing packaged files |
| **UEFI boot** | Confirms that the current session was started through UEFI by checking `/sys/firmware/efi` |
| **Kernel lockdown** | Confirms that the running kernel selected lockdown integrity mode |
| **Kexec loading** | Confirms that `kernel.kexec_load_disabled` is set to `1` |
| **IOMMU** | Checks whether the current boot exposes active IOMMU groups |
| **Linux Security Modules** | Confirms that `landlock`, `lockdown`, `yama`, and `bpf` are active, with integrity support reported as either `integrity` or Fedora's `ima` and `evm` combination |

Each result includes:

- A clear status and short explanation.
- The evidence used to reach the result.
- Plain language steps for failures and unknown results.
- Labelled commands with one-click copy buttons where a command is useful.
- Safety notes before changes that may affect startup or recovery.

### Result meanings

| Status | Meaning |
|---|---|
| **Pass** | The implemented check found the expected state |
| **Warning** | The state may need attention, a restart, or further verification |
| **Fail** | The implemented check found evidence that the expected state is not active |
| **Unknown** | The application could not collect enough reliable evidence |

The overall **Ready** result currently means that every implemented check
passed. It does not mean that all planned EAC v2 requirements have been
implemented or independently confirmed.

---

## Safety and privacy

**Read-only operation.** The application reads selected files under `/etc`,
`/proc`, `/sys`, and `/usr/lib/modules`, and uses RPM only to query and verify
the installed kernel package. It does not edit files, install packages, change
boot settings, or run a privileged helper.

**User-controlled commands.** Remediation commands are displayed as text. The
application never runs them. Commands that use `sudo` execute only if the user
copies them into a terminal and explicitly approves the password prompt.

**No network access.** The checker does not contact Epic, a game publisher,
CachyOS, LinuxGamerLife, or any analytics service. Documentation and project
links open only when the user chooses them.

**No certification claim.** LinuxGamerLife is not affiliated with Epic Games,
Easy Anti-Cheat, CachyOS, or Fedora. Product names are used only to describe the
systems and compatibility properties being checked.

**Recovery risk.** Firmware, Secure Boot, bootloader, kernel, and module changes
can make a system unbootable or stop hardware drivers from loading. Keep current
backups and Fedora recovery media before making those changes.

---

## Known limitations

- The reference procedure was written for CachyOS, not Fedora.
- Fedora does not provide CachyOS packages such as `eac-setup` or `eac-status`.
- Fedora's kernel, bootloader, signing chain, and LSM names may differ from the
  CachyOS examples.
- Fedora kernel package provenance does not prove that Epic trusts the kernel.
- No EAC Linux v2 title is currently available for an end-to-end launch test.
- The application cannot determine Epic's private trust decisions.
- Firmware interfaces and kernel evidence vary across hardware.
- Some information may be unavailable to an unprivileged desktop process.
- A result can become outdated after a kernel update, firmware change, or
  restart. Run the checks again after changing the system.

Please report incorrect checks, unclear guidance, and hardware-specific results
through the
[GitHub issue tracker](https://github.com/linuxgamerlife/lgl-eacv2-checker/issues).
Include the check name and its sanitized evidence. Do not post serial numbers,
usernames, hostnames, private keys, or unrelated kernel command-line secrets.

---

## Build from source

Build from a checkout on Fedora:

```bash
git clone https://github.com/linuxgamerlife/lgl-eacv2-checker.git
cd lgl-eacv2-checker
sudo dnf install cmake gcc-c++ ninja-build pkgconf-pkg-config \
  gtk4-devel gtkmm4.0-devel libadwaita-devel glib2-devel glibmm2.68-devel \
  desktop-file-utils appstream cppcheck
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
./build/lgl-eacv2-checker
```

The project requires:

- C++20
- CMake 3.20 or newer
- gtkmm 4.20 or newer
- libadwaita 1.6 or newer
- GLib and giomm

### Sanitizer build

Build and run the tests with AddressSanitizer and
UndefinedBehaviorSanitizer:

```bash
cmake -S . -B build-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON -DEAC_ENABLE_SANITIZERS=ON
cmake --build build-sanitize -j"$(nproc)"
ctest --test-dir build-sanitize --output-on-failure
```

The normal build uses `-Wall`, `-Wextra`, `-Wpedantic`, and `-Werror`.
Core parsing and settings logic are tested without a display. The test suite
also includes a GTK lifecycle smoke test under Xvfb when `xvfb-run` is
available.

---

## Project layout

```text
CMakeLists.txt        Root build entry point
DESIGN.md             Security design, trust boundaries, and failure model
plan.md               Development plan and milestones
gtk/
  CMakeLists.txt      GTK application, core library, resources, and tests
  data/               GResource manifest and application stylesheet
  src/core/           Widget-free checks, parsing, settings, and result model
  src/ui/             GTK window, check rows, and theme integration
  tests/              Core tests and GTK lifecycle smoke test
packaging/            Desktop entry, AppStream metadata, icons, and RPM files
```

The application follows the LinuxGamerLife GTK desktop standard. Core checking
code is kept independent from widgets so it can be tested using controlled
fixtures rather than the developer's live system.

---

## Contributing

Bug reports and tested hardware results are welcome through
[GitHub Issues](https://github.com/linuxgamerlife/lgl-eacv2-checker/issues).

When reporting a false pass or false failure:

1. Name the affected check.
2. Copy the evidence shown by the application.
3. Include the Fedora version and running kernel version.
4. Explain the expected result and why.
5. Remove hostnames, usernames, serial numbers, keys, tokens, and other private
   information before posting.

Changes to security checks should include fixture-based tests for passing,
failing, missing-file, and malformed-input cases.

---

## Reference

The original compatibility procedure is documented by CachyOS:

[Easy Anti-Cheat](https://wiki.cachyos.org/configuration/easy_anti_cheat/)

The Fedora application independently checks equivalent properties where
possible. It does not assume that CachyOS packages or signing arrangements
exist on Fedora.

---

## License

MIT. This software is provided as-is, without warranty of any kind. See the
[MIT License](https://opensource.org/license/mit) for the full terms.

Epic Games, Easy Anti-Cheat, CachyOS, Fedora, and other product names and
trademarks belong to their respective owners.

---

<div align="center">
Made for <a href="https://fedoraproject.org">Fedora</a> · by
<a href="https://www.youtube.com/@linuxgamerlife">LinuxGamerLife</a>
</div>

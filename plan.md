# Fedora EAC v2 Readiness Checker

## Objective

Build a small GTK application that checks whether the currently booted Fedora
system meets the system and boot-security requirements associated with Epic
Easy Anti-Cheat Linux v2.

The application is a diagnostic tool, not an installer. A passing result means
that the detectable prerequisites are satisfied; it cannot guarantee that a
particular game or publisher supports the system.

Reference:
<https://wiki.cachyos.org/configuration/easy_anti_cheat/>

## Technology

- C++20
- GTK 4 through gtkmm 4
- Libadwaita for initialisation and theme integration only (no Adwaita widgets)
- CMake
- RPM packaging

This follows the LinuxGamerLife GTK Desktop Application Standard in
`/development/projects/docs/gtk-desktop-application-standard.md`. That standard
supersedes the original Rust recommendation for this project.

An RPM is preferable to a Flatpak for the initial release because the
application needs read access to host information under `/proc`, `/sys`,
`/boot`, and the EFI variables.

## User interface

Use one small adaptive window containing:

- An overall readiness result:
  - Ready
  - Restart or boot configuration required
  - Firmware changes required
  - Not ready
  - Unable to determine
- A list of individual checks with pass, warning, fail, or unknown states
- Expandable technical details and suggested remediation for each check
- A button to run the checks again
- Actions to copy or save a sanitized diagnostic report
- A link to relevant documentation

## System checks

### Platform and firmware

- Detect Fedora and its version using `/etc/os-release`.
- Confirm that the system booted in UEFI mode by checking
  `/sys/firmware/efi`.
- Check whether Secure Boot is enabled.
- Check that UEFI Setup Mode is disabled.

### TPM and measured boot

- Confirm that a TPM is present.
- Confirm that it is TPM 2.0.
- Check whether the TPM measured-boot event log is available at
  `/sys/kernel/security/tpm0/binary_bios_measurements`.
- Treat detailed event-log and certificate validation as a later milestone.

### Running kernel

- Confirm that kernel lockdown is in integrity mode using
  `/sys/kernel/security/lockdown`.
- Check that module-signature enforcement is enabled.
- Check loaded kernel modules for missing signatures when that information is
  available.
- Confirm that kexec loading is disabled using
  `/proc/sys/kernel/kexec_load_disabled`.
- Record the running kernel version and its Fedora package.
- Inspect the kernel command line for required or conflicting options.

### IOMMU and Linux Security Modules

- Confirm that IOMMU is active using `/sys/kernel/iommu_groups` and available
  kernel information.
- Confirm that the BPF LSM is active using `/sys/kernel/security/lsm`.
- Report whether the expected LSMs are present:
  `landlock`, `lockdown`, `yama`, `integrity`, and `bpf`.

### Hardware and external modules

- Detect the installed GPU.
- Warn about NVIDIA or other out-of-tree modules whose signatures cannot be
  verified.
- Detect relevant loaded DKMS modules and explain that unsigned modules may be
  incompatible with the required boot state.
- Do not apply CachyOS-specific NVIDIA package requirements directly to
  Fedora.

## Result model

Keep the checking engine independent from the GTK interface.

```cpp
enum CheckStatus {
    Pass,
    Warning,
    Fail,
    Unknown,
}

struct CheckResult {
    std::string id;
    std::string title;
    CheckStatus status;
    std::string summary;
    std::string details;
    std::optional<std::string> remediation;
}
```

Each check should:

1. Read only the minimum required system information.
2. Handle missing or inaccessible files without crashing.
3. Return evidence explaining how its result was determined.
4. Provide a safe remediation suggestion for warning and failure states.

## Privilege and safety model

- Perform checks without root wherever possible.
- Do not automatically modify Secure Boot keys, firmware settings,
  bootloaders, kernel arguments, or installed packages.
- Do not invoke arbitrary commands through `sudo`.
- If privileged inspection is needed later, create a narrowly scoped polkit
  helper with a fixed interface.
- Keep exported reports free of hostnames, usernames, serial numbers, and
  unrelated kernel command-line secrets.

## Proposed project structure

```text
lgl-eacv2-checker/
├── CMakeLists.txt
├── plan.md
├── gtk/
│   ├── CMakeLists.txt
│   ├── data/
│   │   ├── ec.gresource.xml
│   │   └── style.css
│   ├── src/
│   │   ├── main.cpp
│   │   ├── core/
│   │   └── ui/
│   └── tests/
└── packaging/
    ├── com.linuxgamerlife.lgl-eacv2-checker.desktop
    ├── com.linuxgamerlife.lgl-eacv2-checker.metainfo.xml
    └── lgl-eacv2-checker.spec
```

## Milestones

### 1. Application skeleton

- Create the C++20, gtkmm 4, and Libadwaita-initialised project.
- Implement the main window and reusable check-row component.
- Add asynchronous check execution so the interface remains responsive.
- Add application metadata and a placeholder icon.

### 2. Minimum viable checks

- Fedora version
- UEFI boot
- Secure Boot and Setup Mode
- TPM 2.0
- Measured-boot log availability
- Kernel lockdown
- Module-signature enforcement
- IOMMU
- Kexec state
- BPF and expected LSMs

### 3. Diagnostics and reporting

- Add detailed evidence and remediation text.
- Add running-kernel and loaded-module inspection.
- Add GPU and DKMS warnings.
- Add copy-report and save-report actions.
- Sanitize exported reports.

### 4. Boot-chain confidence

- Parse the TPM event log.
- Identify the bootloader, kernel signatures, and certificates involved in the
  current boot.
- Clearly separate verified findings from checks that remain inconclusive.
- Avoid claiming that a Fedora signing chain is trusted by Epic unless this can
  be verified against authoritative information.

### 5. Testing and distribution

- Unit-test parsers using fixture files.
- Test pass, fail, missing-file, permission-denied, and unsupported-system
  cases.
- Test on current supported Fedora releases and representative Intel, AMD, and
  NVIDIA systems.
- Add accessibility labels and keyboard navigation.
- Create an RPM spec and optionally publish through Fedora COPR.

## MVP acceptance criteria

The MVP is complete when it:

- Starts successfully on supported Fedora releases without root.
- Reports an independent status for every core prerequisite.
- Explains each warning or failure and suggests a safe next step.
- Distinguishes a firmware capability from the state of the current boot.
- Handles unavailable information as unknown rather than silently passing it.
- Exports a useful, sanitized text report.
- Never promises that a game will launch or describes the system as
  Epic-certified.

## Important Fedora limitation

The referenced procedure is designed for CachyOS and relies on its dedicated
kernel, signed systemd-boot build, signing certificate, `eac-setup`, and
`eac-status`. Those components must not be assumed to exist or work on Fedora.
The Fedora application should independently inspect equivalent security
properties and clearly report when Epic's trust decision cannot be verified.

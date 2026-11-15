# Changelog

## 0.1.0 - 2026-11-15

- Add the initial GTK 4 application and read-only checking engine.
- Check Fedora identity, UEFI boot, kernel lockdown, kexec, IOMMU groups, and
  expected Linux Security Modules.
- Add evidence, plain language remediation, labelled copyable commands, and a
  startup beta warning.
- Add fixture-based core tests and a GTK lifecycle smoke test.
- Check that the running kernel is an unmodified Fedora `kernel-core` package.
- Add Fedora RPM and COPR packaging.

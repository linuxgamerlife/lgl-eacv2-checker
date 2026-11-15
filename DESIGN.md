# Secure design notes

## Feature

The application performs read-only checks of the current Fedora boot and
summarises whether detectable Easy Anti-Cheat v2 prerequisites are present. It
does not configure the machine and does not claim certification or game
compatibility.

## Assets and trust boundaries

- Untrusted system input: selected files below `/etc`, `/proc`, and `/sys`.
- UI boundary: check results are immutable values handed from a worker thread
  to the GTK main thread.
- User data: one boolean preference under
  `$XDG_CONFIG_HOME/lgl-eacv2-checker/settings.ini`.
- No network connection, privileged helper, package operation, or shell command
  is part of the initial design.

System files may disappear, be inaccessible, be unusually large, or contain
malformed data. Reads are size-capped, parsing is defensive, and unavailable or
ambiguous evidence produces `Unknown`, never `Pass`.

## Thread and ownership model

The window owns its widgets, dispatcher, result buffer, and worker thread.
Only the worker accesses system files. It stores complete value objects behind
a mutex and signals `Glib::Dispatcher`; only the GTK thread updates widgets.
The worker is joined before the window's members are destroyed.

## Misuse and failure behavior

- Repeated checks are generation-tagged; stale results are discarded.
- No check writes to system state or requests elevation.
- Diagnostic text must not include hostnames, usernames, serial numbers, or the
  complete kernel command line.
- Missing files, permission failures, oversized files, and malformed values are
  reported as unknown with bounded evidence.

## Initial review decision

Least privilege is satisfied by an unprivileged, read-only process. Any future
privileged operation requires a new design review and a narrowly scoped helper;
the GUI must never run as root.

Name:           lgl-eacv2-checker
Version:        0.1.0
Release:        1%{?dist}
Summary:        Fedora EAC v2 boot security readiness checker

License:        MIT
URL:            https://github.com/linuxgamerlife/lgl-eacv2-checker
Source0:        %{url}/archive/refs/tags/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.20
BuildRequires:  cmake-rpm-macros
BuildRequires:  gcc-c++
BuildRequires:  ninja-build
BuildRequires:  pkgconfig(gtkmm-4.0) >= 4.20
BuildRequires:  pkgconfig(libadwaita-1) >= 1.6
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gio-2.0)
BuildRequires:  desktop-file-utils
BuildRequires:  appstream
BuildRequires:  xorg-x11-server-Xvfb

Requires:       hicolor-icon-theme

%description
LGL EAC v2 Checker is a read-only GTK diagnostic application for Fedora. It
examines detectable boot security properties associated with Easy Anti-Cheat
Linux v2 and provides evidence and user-controlled remediation guidance. It
does not change firmware, boot settings, kernel arguments, or system files.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -DBUILD_TESTING=ON
%cmake_build

%install
%cmake_install

%check
%ctest
desktop-file-validate %{buildroot}%{_datadir}/applications/com.linuxgamerlife.%{name}.desktop
appstreamcli validate --no-net %{buildroot}%{_datadir}/metainfo/com.linuxgamerlife.%{name}.metainfo.xml

%files
%license LICENSE
%doc README.md CHANGELOG.md DESIGN.md
%{_bindir}/%{name}
%{_datadir}/applications/com.linuxgamerlife.%{name}.desktop
%{_datadir}/metainfo/com.linuxgamerlife.%{name}.metainfo.xml
%{_datadir}/icons/hicolor/48x48/apps/%{name}.png
%{_datadir}/icons/hicolor/64x64/apps/%{name}.png
%{_datadir}/icons/hicolor/128x128/apps/%{name}.png
%{_datadir}/icons/hicolor/256x256/apps/%{name}.png

%changelog
* Sun Nov 15 2026 LinuxGamerLife - 0.1.0-1
- Initial beta release
- Add read-only Fedora, UEFI, lockdown, kexec, IOMMU, and LSM checks
- Add guided remediation, startup risk acknowledgement, and GTK interface

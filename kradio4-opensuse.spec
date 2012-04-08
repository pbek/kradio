#
# spec file for package KRadio4
#
# Copyright (c) 2012 Ernst Martin Witte
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://sourceforge.net/tracker/?group_id=45668
#
Name:          kradio4
Summary:       AM/FM/INternet Radio Application for KDE 4.x
Version:       4.0.5
Release:       1.opensuse
Group:         Hardware/Radio
License:       GPL-2.0+
URL:           http://kradio.sourceforge.net
Source:        http://sourceforge.net/projects/kradio/files/kradio/%{version}/%name-%{version}.tar.gz
BuildRequires: fdupes
BuildRequires: libkde4-devel >= 4.2.0
BuildRequires: libsndfile-devel
BuildRequires: libvorbis-devel
BuildRequires: lirc-devel
BuildRequires: libffmpeg-devel
BuildRequires: libmms-devel
Provides:      kde4-kradio = 4.0.5
Obsoletes:     kde4-kradio < 4.0.5
BuildRoot:     %{_tmppath}/%{name}-%{version}-build
%kde4_runtime_requires
Vendor:        Martin Witte <emw-kradio@nocabal.de>
Packager:      Martin Witte <emw-kradio@nocabal.de>

%description 
Comfortable AM/FM/Internet-Radio Application for KDE4

KRadio currently provides:

* V4L/V4L2 Radio support
* Internet Radio support (libffmpeg)
* DBus support
* RDS support
* PVR support
* Remote Control support (LIRC)
* Alarms, Sleep Countdown
* Several GUI Controls (Docking Menu, Station Quickbar, Radio Display, Shortcuts)
* Timeshifter Capability
* Recording Capabilities (mp3, ogg/vorbis, wav, ...)
* Extendable Plugin Architecture

This Package also includes a growing collection of station preset
files for many cities around the world contributed by KRadio Users.

As KRadio is based on an extendable plugin architecture, contributions
of new plugins (e.g. Internet Radio Streams, new cool GUIs) are welcome.

Authors:
--------
    Ernst Martin Witte <emw-kradio@nocabal.de>
    Marcus Camen <mcamen@mcamen.de>
    Klas Kalass <klas.kalass@gmx.de>
    Frank Schwanz <schwanz@fh-brandenburg.de>


%files -f %name.lang
%defattr(-,root,root,-)
%{_datadir}/pixmaps/kradio4.png
%{_docdir}/%{name}
%exclude %{_docdir}/%{name}/INSTALL
%{_kde4_bindir}/*
%{_kde4_libdir}/kradio4
%{_kde4_appsdir}/kradio4
%{_kde4_iconsdir}/*
%{_kde4_applicationsdir}/kradio4.desktop

%prep
%setup -q 

%build 
%cmake_kde4
%make_jobs

%install
%make_install
mkdir %{buildroot}/%{_docdir}
mv %{buildroot}/%{_docdir}/../%{name} %{buildroot}/%{_docdir}/%{name}
%suse_update_desktop_file -G "KDE Radio Tuner" kradio4 AudioVideo Tuner
%kde_post_install
%fdupes %{buildroot}
%find_lang kradio4 --all-name

%post   -p /sbin/ldconfig
%postun -p /sbin/ldconfig


#
# spec file for package kradio
#

Name:      kradio
License:   GPL
Summary:   V4L/V4L2-Radio Application for KDE
Version:   <place version here>
Release:   <place release here, e.g. suse/mandrake>
Vendor:    Martin Witte <witte@kawo1.rwth-aachen.de>
Packager:  Martin Witte <witte@kawo1.rwth-aachen.de>
Url:       http://sourceforge.net/projects/kradio
Group:     kde3
Source:    kradio-%version.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot

%description
Comfortable V4L/V4L2-Radio Application for KDE

KRadio is a comfortable radio application for KDE with support for 
V4L and V4L2 radio cards drivers.

KRadio currently provides:

* V4L/V4L2 Radio support
* Remote Control support (LIRC)
* Alarms, Sleep Countdown
* Several GUI Controls (Docking Menu, Station Quickbar, Radio Display)
* Timeshifter Capability
* Recording Capabilities (mp3, ogg/vorbis, wav, ...)
* Extendable Plugin Architecture

This Package also includes a growing collection of station preset
files for many cities around the world contributed by KRadio Users.

As KRadio is based on an extendable plugin architecture, contributions
of new plugins (e.g. Internet Radio Streams, new cool GUIs) are welcome.

Authors:
--------
    Ernst Martin Witte <witte@kawo1.rwth-aachen.de>
    Marcus Camen <mcamen@mcamen.de>
    Klas Kalass <klas.kalass@gmx.de>
    Frank Schwanz <schwanz@fh-brandenburg.de>

%prep
%setup -q
. /etc/opt/kde3/common_options
update_admin --no-final

%build
. /etc/opt/kde3/common_options
./configure $configkde --without-gl
  
make

%install
. /etc/opt/kde3/common_options
make DESTDIR=$RPM_BUILD_ROOT $INSTALL_TARGET
%find_lang %name-%version

%files -f %name-%version.lang
%defattr(-,root,root)
/opt/kde3/bin/*
/opt/kde3/share/appl*/*/*.desktop
/opt/kde3/share/icons/*
/opt/kde3/lib/kradio
/opt/kde3/share/apps/kradio

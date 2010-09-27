Name:          kradio4
Summary:       AM/FM/INternet Radio Application for KDE 4.x
Version: 4.0.1_rc2
Group:         kde4
License:       GPL
URL:           http://kradio.sourceforge.net
Release:       1_suse11.1
Source:        %name-%{version}.tar.gz
BuildRequires: kde4-filesystem
BuildRequires: cmake
BuildRequires: libkde4-devel
BuildRequires: libffmpeg-devel
BuildRequires: libmp3lame-devel
BuildRequires: lirc
BuildRequires: libogg-devel
BuildRequires: libvorbis-devel
%kde4_runtime_requires
Vendor:        Martin Witte <emw-kradio@nocabal.de>
Packager:      Martin Witte <emw-kradio@nocabal.de>
BuildRoot:     %{_tmppath}/%{name}-%{version}-build

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
%_kde_prefix/bin/*
%_kde_libdir/kradio4/*
%_kde_share_dir/apps/kradio4/*
%_kde_prefix/share/icons/*
%_kde_prefix/share/locale/*
%_kde_prefix/share/doc/*
%_kde_prefix/share/applications/kde4/*

%prep
%setup -q 

%build 
%cmake_kde4
%make_jobs

%install
%makeinstall
%kde_post_install 
%find_lang %name

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf filelists

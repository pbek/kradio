# This spec file was generated using Kpp
# If you find any problems with this spec file please report
# the error to ian geiser <geiseri@msoe.edu>
Summary:   V4L/V4L2-Radio Application for KDE 3.x
Name:      KRadio
Version:   0.3.0-alpha2
Release:   1.suse
Copyright: GPL
Vendor:    Martin Witte <witte@kawo1.rwth-aachen.de>
Url:       http://sourceforge.net/projects/kradio
Packager:  Martin Witte <witte@kawo1.rwth-aachen.de>
Group:     kde3
Source:    /usr/src/kradio-cvs/kradio/kradio-0.3.0-alpha2.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot

%description
V4L-Radio Application for KDE 3.x

KRadio provides:

V4L/V4L2 Radio support
Remote Control support (LIRC)
Alarms, Sleep Countdown
Several GUI Controls (Docking Menu, Station Quickbar, Radio Display)
Recording Capabilities
extendable Plugin Architecture

This Package also incluse a Station Preset File Database contributed
by KRadio Users around the world.

%prep
%setup
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure \
                 --build=i386-linux --host=i386-linux --target=i386-linux  \
		 --prefix=/opt/kde3 --program-prefix="" \
                $LOCALFLAGS
%build
# Setup for parallel builds
numprocs=`egrep -c ^cpu[0-9]+ /proc/stat || :`
if [ "$numprocs" = "0" ]; then
  numprocs=1
fi

make -j$numprocs

%install
make install-strip DESTDIR=$RPM_BUILD_ROOT

cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.\(.*\)$,\%attr(-\,root\,root) \%dir "\1",' > $RPM_BUILD_DIR/file.list.kradio
find . -type f | sed 's,^\.\(.*\)$,\%attr(-\,root\,root) "\1",' >> $RPM_BUILD_DIR/file.list.kradio
find . -type l | sed 's,^\.\(.*\)$,\%attr(-\,root\,root) "\1",' >> $RPM_BUILD_DIR/file.list.kradio

%clean
rm -rf $RPM_BUILD_ROOT/*
rm -rf $RPM_BUILD_DIR/kradio
rm -rf ../file.list.kradio


%files -f ../file.list.kradio

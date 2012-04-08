%bcond_without ffmpeg

Version:   4.0.5
%define    rel2 release
%define    rel %{version}-%{rel2}
Name:      kradio4
License:   GPLv2
Summary:   V4L/V4L2-Radio Application for KDE4
Release:   %{rel}%{?dist}
Url:       http://kradio.sourceforge.net/
Group:     Applications/Multimedia
Source0:   http://downloads.sourceforge.net/%{name}/%{name}-%{rel}.tar.bz2
Source1:   Doxyfile.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{release}-root-%(%{__id_u}-n)
Buildrequires: chrpath 
Buildrequires: cmake >= 2.6.2 
Buildrequires: alsa-lib-devel
Buildrequires: gcc-c++ gettext flex doxygen graphviz dbus-devel
%if %{with ffmpeg}
Buildrequires: ffmpeg-devel
Buildrequires: libmms-devel
%endif
Buildrequires: kdelibs-devel >= 4.2
Buildrequires: lirc-devel 
Buildrequires: libsndfile-devel
Buildrequires: libogg-devel 
Buildrequires: libvorbis-devel 
Buildrequires: qt-devel >= 4.4 
Buildrequires: strigi-devel 
Buildrequires: libboost-devel

%description
KRadio is a comfortable radio application for KDE4 with support for 
V4L and V4L2 radio card drivers.

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

%prep
%setup -q -a 1 -n %{name}-%{rel}

%build
unset QTINC QTLIB QTPATH_LRELEASE QMAKESPEC
export QT4DIR=%{_libdir}/qt4
export QTDIR=$QT4DIR
PATH=$QT4DIR/bin:$PATH ; export PATH

%cmake -DCMAKE_BUILD_TYPE=release ../%{name}-%{rel}
make %{?_smp_mflags}
doxygen
  
%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}
find %{_builddir} -type f -name '*.map' -a -size 0 -exec rm -f {} \;
find %{_builddir} -type f -name installdox -exec rm -f {} \;

mkdir -p %{buildroot}%{_datadir}/pixmaps
mkdir -p %{buildroot}%{_datadir}/doc/HTML/en/%{name}
mkdir -p %{buildroot}%{_datadir}/applications/kde4
cp %{buildroot}%{_datadir}/icons/hicolor/48x48/apps/%{name}.png \
   %{buildroot}%{_datadir}/pixmaps/%{name}.png
cp -R %{_builddir}/%{name}-%{rel}/man/ \
      %{buildroot}%{_datadir}/
cp -R %{_builddir}/%{name}-%{rel}/html/* \
      %{buildroot}%{_datadir}/doc/HTML/en/%{name}

%find_lang %{name}.\*

# Create a desktop entry
%{__cat} << EOF > %{buildroot}%{_datadir}/applications/kde4/%{name}.desktop
[Desktop Entry]
Type=Application
Exec=%{name} -caption "%c" %i %m  
Icon=%{name}.png
X-DocPath=%{name}/index.html
Comment=V4L/V4L2-Radio Application for KDE4
Terminal=false
Name=%{name}
Categories=Qt;KDE;AudioVideo;
EOF

%clean
rm -rf %{buildroot}

%files -f %{name}.\*.lang
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog FAQ INSTALL dot-lircrc.example 
%doc README README.kde4 README.PVR REQUIREMENTS TODO TODO.kdetestscripts
%{_bindir}/%{name}
%{_libdir}/%{name}
%{_datadir}/pixmaps/%{name}.png
%{_datadir}/icons/hicolor/*/*/*
%{_datadir}/icons/locolor/*/*/*
%{_datadir}/kde4/apps/%{name}
%{_datadir}/applications/kde4/%{name}.desktop
%{_datadir}/doc/HTML/en/%{name}
# conflicts with previous kradio
%exclude %{_datadir}/man/man3/*
%exclude %{_datadir}/doc/%{name}

%changelog

* Tue Apr 14 2009 Paulo Roma <roma@lcg.ufrj.br> - 4.0.0-0.7.r829.20090411
- Updated to snapshot 829.
- Using %%cmake. Rpaths removed automatically.
- Changed Source URL.

* Fri Apr 11 2009 Paulo Roma <roma@lcg.ufrj.br> - 4.0.0-0.6.r829.20090411
- Updated to snapshot 829.
- Removed already applied mute patch.

* Fri Apr 10 2009 Paulo Roma <roma@lcg.ufrj.br> - 4.0.0-0.5.r827.20090410
- Updated to snapshot 827.
  svn co https://kradio.svn.sourceforge.net/svnroot/kradio/trunk kradio
- Removed obsolete flag.patch
- Created mute patch for unmuting after power off/power on.

* Fri Apr 10 2009 Paulo Roma <roma@lcg.ufrj.br> - 4.0.0-0.4.rc2
- Updated to 4.0.0-rc2

* Thu Apr 02 2009 Paulo Roma <roma@lcg.ufrj.br> - 4.0.0-0.3.rc1
- Added BR strigi-devel and libmms-devel for internet radio plugin.
- Updated to 4.0.0-rc1

* Fri Mar 31 2009 Paulo Roma <roma@lcg.ufrj.br> - 4.0.0-0.1.r778.20090322
- Added BR alsa-lib-devel.

* Fri Mar 27 2009 Paulo Roma <roma@lcg.ufrj.br> - 4.0-0.1.r778.20090322
- Updated to kradio4-snapshot-2009-03-22-r778.
- Rewritten spec file.

* Wed Jun 11 2008 Paulo Roma <roma@lcg.ufrj.br> - r497.20061112-4
- Changed BRs qt-devel for qt3-devel and kdelibs-devel for kdelibs3-devel.
- Patched for lirc support in F9.

* Fri May 09 2008 Paulo Roma <roma@lcg.ufrj.br> - r497.20061112-3
- Removed unneeded Requires.
- Using find_lang.
- Disabled rpath.
- Removed empty files.

* Sun Feb 04 2007 Paulo Roma <roma@lcg.ufrj.br> - r497.20061112-2.1
- Created Doxyfile for generating html docs and man pages.
- Added missing BRs.

* Sat Feb 03 2007 Paulo Roma <roma@lcg.ufrj.br> - r497.20061112-2
- Rebuilt for Fedora 6.
- Using unsermake.
- Updated to r497.20061112

* Wed Feb 08 2006 Paulo Roma <roma@lcg.ufrj.br> - r497.20061112-1
- Initial spec file.


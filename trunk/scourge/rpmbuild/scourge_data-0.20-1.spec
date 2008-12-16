Summary: Rogue-like RPG
Summary(de): Rogue-artiges Rollenspiel
Name: scourge_data
Version: 0.20
Group: Amusements/Games
Release: 1
License: GPL
Source0: http://internap.dl.sourceforge.net/sourceforge/scourge/scourge-%{version}.data.tar.gz
Source1: scourge.svg
Source2: scourge.desktop
BuildRoot: /var/tmp/%{name}-buildroot
Requires:scourge = %{version}
Buildarch:noarch

%description
S.C.O.U.R.G.E. is a rogue-like game in the fine tradition of NetHack and Moria It sports a graphical front-end, similar to glHack or the Falcon's eye. I tried to design the 3D UI as a best of both worlds from old to new: It lets you rotate the view, zoom in/out, view special effects, etc. On the other hand I've always liked the old-school isometric games like Exult or Woodward.

%description -l de
S.C.O.U.R.G.E. ist ein rogue-artiges Rollenspiel in der edlen Tradition von Nethack und Moria. Es verfügt über ein grafisches Frontend, ähnlich glHack oder Falcon's Eye. Ich habe versucht, in der 3D-Schnittstelle das Beste beider Welten, alt wie neu, zu vereinen: Es ist möglich, die Ansicht zu drehen, zu zoomen, Spezialeffekte zu betrachten usw. Andererseits habe ich stets althergebrachte isometrische Spiele wie Exult oder Woodward geschätzt.

%prep
cd /usr/src/redhat/BUILD
rm -rf %{name}
gzip -dc /usr/src/redhat/SOURCES/scourge-%{version}.data.tar.gz | tar -xvvf -
if [ $? -ne 0 ]; then
  exit $?
fi
cd %{name}
cd /usr/src/redhat/BUILD/%{name}
chown -R root.root .
chmod -R a+rX,g-w,o-w .

%install
cd /usr/src/redhat/BUILD/%{name}
/bin/mkdir -p $RPM_BUILD_ROOT/usr/local/share/scourge
/bin/cp -r /usr/src/redhat/BUILD/%{name}/* $RPM_BUILD_ROOT/usr/local/share/scourge
/bin/mkdir -p $RPM_BUILD_ROOT/usr/share/pixmaps
/bin/mkdir -p $RPM_BUILD_ROOT/usr/share/applications
/bin/cp /usr/src/redhat/SOURCES/scourge.svg $RPM_BUILD_ROOT/usr/share/pixmaps
/bin/cp /usr/src/redhat/SOURCES/scourge.desktop $RPM_BUILD_ROOT/usr/share/applications

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/local/share/scourge
/usr/share/pixmaps/scourge.svg
/usr/share/applications/scourge.desktop


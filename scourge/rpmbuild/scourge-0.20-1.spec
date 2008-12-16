Summary: Rogue-like RPG
Summary(de): Rogue-artiges Rollenspiel
Name: scourge
Version: 0.20
Group: Amusements/Games
Release: 1
License: GPL
Source: http://internap.dl.sourceforge.net/sourceforge/scourge/scourge-%{version}.src.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot
Requires:SDL_net >= 1.2, SDL >= 1.2, SDL_ttf >= 1.2, SDL_mixer >= 1.0, freetype >= 2, mesa-libGL >= 6.5, SDL_image >= 1.2, scourge_data = %{version}
BuildPrereq:SDL_net-devel >= 1.2, SDL-devel >= 1.2, SDL_ttf-devel >= 1.2, SDL_mixer-devel >= 1.2, SDL_image-devel >= 1.2, freetype-devel >= 2, mesa-libGL-devel >= 6.5, automake >= 1, libstdc++-devel >= 4.1, scons >=  0.98

%description
S.C.O.U.R.G.E. is a rogue-like game in the fine tradition of NetHack and Moria It sports a graphical front-end, similar to glHack or the Falcon's eye. I tried to design the 3D UI as a best of both worlds from old to new: It lets you rotate the view, zoom in/out, view special effects, etc. On the other hand I've always liked the old-school isometric games like Exult or Woodward.

%description -l de
S.C.O.U.R.G.E. ist ein rogue-artiges Rollenspiel in der edlen Tradition von Nethack und Moria. Es verfügt über ein grafisches Frontend, ähnlich glHack oder Falcon's Eye. Ich habe versucht, in der 3D-Schnittstelle das Beste beider Welten, alt wie neu, zu vereinen: Es ist möglich, die Ansicht zu drehen, zu zoomen, Spezialeffekte zu betrachten usw. Andererseits habe ich stets althergebrachte isometrische Spiele wie Exult oder Woodward geschätzt.

%prep
cd /usr/src/redhat/BUILD
rm -rf %{name}
gzip -dc /usr/src/redhat/SOURCES/%{name}-%{version}.src.tar.gz | tar -xvvf -
if [ $? -ne 0 ]; then
  exit $?
fi
cd %{name}
cd /usr/src/redhat/BUILD/%{name}
chown -R root.root .
chmod -R a+rX,g-w,o-w .

%build
cd /usr/src/redhat/BUILD/%{name}
autoreconf -i
%configure
make

%install
cd /usr/src/redhat/BUILD/%{name}
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/bin/scourge
/usr/share/locale/de/LC_MESSAGES/scourge.mo
/usr/share/locale/en/LC_MESSAGES/scourge.mo
/usr/share/locale/es/LC_MESSAGES/scourge.mo
/usr/share/locale/fr/LC_MESSAGES/scourge.mo
/usr/share/locale/hu/LC_MESSAGES/scourge.mo
/usr/share/locale/it/LC_MESSAGES/scourge.mo
/usr/share/locale/pl/LC_MESSAGES/scourge.mo
/usr/share/locale/pt/LC_MESSAGES/scourge.mo
/usr/share/locale/pt_BR/LC_MESSAGES/scourge.mo
/usr/share/locale/ru/LC_MESSAGES/scourge.mo
/usr/share/locale/sv/LC_MESSAGES/scourge.mo

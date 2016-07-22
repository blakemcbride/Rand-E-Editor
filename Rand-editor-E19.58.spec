# Fabien Perriollat, CERN, Jan 2002
#   <Fabien.Perriollat@cern.ch>

# Relocation directory
# --------------------
# To build outside the default /usr/local directory :
#   'userhomedir' and 'buildroot' macro values can be defined
#    in ~/.rpmmacros, /etc/rpm/macros or /etc/rpm/i686-linux/macros
# or by rpm options (see rpm man and rpm macros doc file in
#   /usr/share/doc/rpm-4.0.2)
#   rpm --define "userhomedir somewhere"
#   rpm --buildroot somewhere
# _prefix is a defined macro of rpm package in /usr/lib/rpm/macros file
#   rpm --showrc to see the values which are set.

# The build root directory must be chossen such a way that it must be empty
#   before install of the package. The various 'brp-strip' rpm shell
#   scripts which are executed just before building the package files
#   try to strip evry executable files in the build_root directory.
#   see : /usr/lib/rpm/brp-redhat and the other /usr/lib/rpm/brp-* files.

# The rpm macros 'dump' and 'trace' can be used to produce a verbose
#   output, with many info to debug building process.
#   look at /usr/share/doc/rpm-4.0.2/macro

%define defaultbuildroot /tmp/rpm_buildroot
%define targdir %{_prefix}/local
Prefix: %{targdir}
BuildRoot: %{defaultbuildroot}


Summary: Rand full screen text Editor for Linux
Name: Rand
Version: E19
Release: 58
Copyright: Copyright abandoned, 1983, The Rand Corporation
Group: Applications/Editors
Source: ftp://home.cern.ch/perrioll/Rand_Editor/%{name}-%{version}.%{release}.tgz
Packager: Fabien PERRIOLLAT <Fabien.Perriollat@cern.ch>
URL: http://home.cern.ch/perrioll/Rand_Editor
Requires: XFree86
# Exclusivearch: i386
Exclusiveos: Linux
Serial: 4

%define pkgname %{name}

%description
The %{name} editor is one of several editors running on UNIX systems. It is
a full screen text editor initialy developped by Rand Corporation,
(copyright abandoned, 1983, by the Rand Corporation). The Rand Corporation
initial version was called NED.
The linux version is an improved version of the E editor provided
at CERN on the PRIAM Unix service by Gordon Lee and Peter Villemoes.
It was enriched with more one-line help, new commands and key functions.
A default keyboard configuration for ANSI like terminals
(linux console, xterm family, vt200 family ..).
It has the capability to handle Unix and DOS text files.
.
The source (.src.rpm file) and binary for linux (.rpm) can be get from :
    %{url}
The current version is : %{name}-%{version}.%{release}

# ----------------------------------------------------------------------------
# to rebuild the package (bin and source files) :
#   cd /usr/src/redhat/SPECS
#   rpm -ba --clean Rand-editor-E19.58.spec
#
#   To load somewhere else than the default build in : /usr/local
#   rpm -ivh --prefix <somewhere> Rand-E19-58.i386.rpm
#   rpm -ivh --relocate /usr/local=<somewhere> Rand-E19-58.i386.rpm
#       to prevent creation of links in the doc system directory use
#           --excludedocs rpm option flag
#
# NB : the version and revision must be coherent with Rand-E19/e19/NewVersion file
#
# Various usefull rpm commands
#   rpm --showrc    to see the macros and values which are set.
#   rpm -q  Rand    display the current installed version of Rand package
#   rpm -qi Rand    display info on installed Rand package
#   rpm -ql Rand    display files list of the installed Rand package
#   rpm -qc Rand    display list of configuration files in Rand package
#   rpm -qd Rand    display list of documentation files in Rand package
#   rpm -V  Rand    check the installed Rand package
#
# To Erase the Rand package :
#   rpm --erase Rand
#
# ----------------------------------------------------------------------------

# directory where to install the Rand editor
%define pkgdir %{pkgname}
%define cfgdir %{targdir}/etc
%define docdir %{targdir}/doc/%{pkgname}
%define pkgtargdir %{targdir}/%{pkgdir}
%define pkgcfgdir %{cfgdir}/%{pkgdir}
%define bindir %{targdir}/bin
%define genericprgname %{bindir}/e

# keyboard definition files
%define KbDir kbfiles
%define RefKbFile vt200kbn
%define KbFiles universalkb linuxkb xtermkb nxtermkb
%define KbscDir ./help/%{KbDir}

# directory used by rpm to build the packages
%define RpmDirs BUILD SOURCES SPECS SRPMS
%define RpmsDir RPMS
%define RpmsDirs athlon i386 i486 i586 i686 noarch

# BUILD directory is needed before starting %prep
%(mkdir -p %{_topdir}/BUILD)

%prep
    set +x
    echo    "|-------------------------------------------------------"
    echo -n "| $USER : "; date
    echo    "|     Compile the %{name} package in %{_topdir}"
    echo    "|-------------------------------------------------------"
    set -x

    for d in %{RpmDirs} ; do
	mkdir -p %{_topdir}/$d
    done;
    for d in %{RpmsDirs} ; do
	mkdir -p %{_topdir}/%{RpmsDir}/$d
    done;

%setup -n %{pkgname}-%{version}.%{release}

%build
    ./local.sh
    make depend
    make VERSION=%{version} RELEASE=%{release} TARGETDIR=%{targdir} PKGDIR=%{pkgdir}

%install
    set +x
    echo    "|-------------------------------------------------------"
    echo -n "| $USER : "; date
    echo    "|     Install the %{name} package in $RPM_BUILD_ROOT%{targdir}"
    echo    "|     BuildRoot  is %{?buildroot:%{buildroot}}"
    echo    "|     Relocation is %{targdir}"
    echo    "|     _topdir    is %{_topdir}"
    echo    "|     targdir    is %{targdir}"
    echo    "|-------------------------------------------------------"
    set -x

make preinstall TARGETDIR=$RPM_BUILD_ROOT%{targdir} PKGDIR=%{pkgdir}
make install TARGETDIR=$RPM_BUILD_ROOT%{targdir} PKGDIR=%{pkgdir}

# link the executable to the generic name
    mkdir -p $RPM_BUILD_ROOT%{bindir}
    rm -f $RPM_BUILD_ROOT%{genericprgname}
    (cd $RPM_BUILD_ROOT%{bindir}; ln -s ../%{pkgname}/e19.%{release} ./e)

# install system wide keyboard configuration files
    mkdir -p $RPM_BUILD_ROOT%{pkgcfgdir}/%{KbDir}
    install -m 644 %{KbscDir}/%{RefKbFile} $RPM_BUILD_ROOT%{pkgcfgdir}/%{KbDir}
    for i in %{KbFiles} ; do
	if [ -e %{KbscDir}/$i -a ! -L %{KbscDir}/$i ]; then
	    rm -f $RPM_BUILD_ROOT%{pkgcfgdir}/%{KbDir}/$i
	    install -m 644 %{KbscDir}/$i $RPM_BUILD_ROOT%{pkgcfgdir}/%{KbDir}
	else
	    ( cd $RPM_BUILD_ROOT%{pkgcfgdir}/%{KbDir}; \
		rm -f ./$i; \
		ln -s ./%{RefKbFile} ./$i; \
	    )
	fi
    done;

# doc file to be installed into the package doc directory
    mkdir -p $RPM_BUILD_ROOT%{pkgtargdir}/doc
    install -m 644 ./doc/Introduction_to_Rand.txt $RPM_BUILD_ROOT%{pkgtargdir}/doc
    install -m 644 ./doc/Rand-E19_man.html $RPM_BUILD_ROOT%{pkgtargdir}/doc
    install -m 644 ./README $RPM_BUILD_ROOT%{pkgtargdir}
    rm -f $RPM_BUILD_ROOT%{docdir}/Introduction_to_Rand.txt $RPM_BUILD_ROOT%{docdir}/README $RPM_BUILD_ROOT%{docdir}/Rand-E19_man.html

# doc file to be linked into the default doc directory
    mkdir -p $RPM_BUILD_ROOT%{docdir}
    ( cd $RPM_BUILD_ROOT%{docdir}; \
	ln -s ../../%{pkgname}/doc/Introduction_to_Rand.txt ./; \
	ln -s ../../%{pkgname}/doc/Rand-E19_man.html ./; \
	ln -s ../../%{pkgname}/README ./; \
    )

%clean
    if [ $RPM_BUILD_ROOT = %{defaultbuildroot} ]; then
	rm -rf $RPM_BUILD_ROOT%{targdir}
    fi

%postun
    if [ -d %{docdir} ]; then
	rmdir --ignore-fail-on-non-empty %{docdir}
    fi
    if [ -d %{pkgcfgdir} ]; then
	rmdir --ignore-fail-on-non-empty %{pkgcfgdir}/%{KbDir}
	rmdir --ignore-fail-on-non-empty %{pkgcfgdir}
    fi

%files
%defattr(-,root,root)
%{targdir}/man/man1/e.1
%doc %{docdir}/README
%doc %{docdir}/Introduction_to_Rand.txt
%doc %{docdir}/Rand-E19_man.html
# %dir %{pkgcfgdir}
# %dir %{pkgcfgdir}/%{KbDir}
%config(missingok) %{pkgcfgdir}/%{KbDir}/vt200kbn
%config(missingok) %{pkgcfgdir}/%{KbDir}/universalkb
%config(missingok) %{pkgcfgdir}/%{KbDir}/linuxkb
%config(missingok) %{pkgcfgdir}/%{KbDir}/xtermkb
%config(missingok) %{pkgcfgdir}/%{KbDir}/nxtermkb
%dir %{pkgtargdir}
%{pkgtargdir}/Crashdoc
%{pkgtargdir}/errmsg
%{pkgtargdir}/helpkey
%{pkgtargdir}/recovermsg
%{pkgtargdir}/e19
%{pkgtargdir}/e19.%{release}
%{pkgtargdir}/center
%{pkgtargdir}/fill
%{pkgtargdir}/just
%{pkgtargdir}/run
%dir %{pkgtargdir}/%{KbDir}
%{pkgtargdir}/%{KbDir}/vt200kbn
%{pkgtargdir}/%{KbDir}/universalkb
%{pkgtargdir}/%{KbDir}/linuxkb
%{pkgtargdir}/%{KbDir}/xtermkb
%{pkgtargdir}/%{KbDir}/nxtermkb
%{pkgtargdir}/README
%dir %{pkgtargdir}/doc
%{pkgtargdir}/doc/Introduction_to_Rand.txt
%{pkgtargdir}/doc/Rand-E19_man.html
%{genericprgname}

%changelog
* Tue Jan 14 2002 by Fabien Perriollat <Fabien.Perriollat@cern.ch>
- revision 58
- IMPORTANT NOTICE : this version is not backward compatible with the
  previous one for the key stroke file content (used in replay facility).
- Support for the ISO 8859 (8 bits) character set.
- "cchar" key function (CTRLQUOTE = quoted character) key can enter any
  character by its code value in octal, decimal or hexadecimal.
- Window edges characters set can be dynamicaly controled by set command.
- "root" user is not any more a very special case for the directory in which
  are build the working files for the editing session. The working files
  are build in the current directory either if the owner or the group
  of this directory is "root", else they will be in /tmp directory.
- During file (or directory) name expansion (with the tab key for commands
  which support this feature) a second tab key stroke will display
  the list of entries which match the patern.
- Editor command can be expended with the tab key. In case of ambiguous
  command a second tab key stroke display the list of all amgiguous command.
- Editor command followed by a single "!" display some info on the command
  parameters. On command widow a single "!" display the list of all commands
  and there aliases.
- The help key is context sensitive (editing window or command window).
- The "help <cmd>" display not only the on line documentation for the
  given editor command, but also for keyboard function (if 'cmd' is a
  keyboard function.
- New option in "help" command : "charset" and "graphset" to display the
  character set in use to display the text and the window edges. New set
  of keyword for the keyboard definition file to control this character sets.
- Addition of new command "build-kbfile" to generate a keyboard definition
  file from the current setting, to be used for cutomization.
- New directive "#!Comment" and "#!-Comment" for keyboard definition file.
  Look at the file generated by "build-kbfile" for the available set of
  keywords and directives.
- New option for "set" command : "set info n" define what to display on
  the information line.
- Modification of the spec file to be able to build the package from
  a user defined root directory. The .rpmmacros file is provide, and
  can be copied into $HOME/ to simplify package building.
- A POSIX lock file is used on the key stroke working file to allow muiltiple
  sessions by the same user in the same directory, (see the session number
  in the working files .es1, .ec1 .ek1 .ek1b for sessions '1' ... '9').
- Various bugs and inconsistancy in startup corrected, handling exit
  of a -notracks session changed : the new state file (.esX) is saved.
- New aliases for the 'exit' command : 'quit', 'q'
- The empty option parameter '-' can be used to end the option list
  and edit a file for which the name start with a '-' character.
- Cursor current and previous positions, and file tick mark are save into
  status file, and restaured.
- Transiant tick mark is generated on major window motions (look at the
  'help keyf tick' output for descrition of transient tick and key tick
  navigation facility.
- The command "edit -" can be used to remove a file from the current list
  of edited file (if it is not modified, nor referenced).
- New syntax for the -debug option : -debug=[debug output file]:[debug level]
  The "set debuglevel 0" suspend the debugging printout, and close the
  output file. "set debuglevel <no zero level>" resume the debugging.
- The new "set preferences" command save the user preferenes into preferences
  file : $HOME/.Rand/preferences (see on line help for set command).
  "set preferences ?"  Display the content of preferences file for the
  current terminal.

* Wed Jun 13 2001 by Fabien Perriollat <Fabien.Perriollat@cern.ch>
- revision 57
- Correct handling of very large file system.
- Customisation of editing window boder characters set.
- Better interaction with the edited files list.
- Enter in command history navigation on <CMD> <CMD> <ALT> keys stroke.
- Addition of a "contributed" directory in the sources with include
  various usefull file for better local customisation.

* Fri Feb 02 2001 by Fabien Perriollat <Fabien.Perriollat@cern.ch>
- revision 56
- New command "flipbkarrow" to flip between "DEL" and "DELCH" the Back Arrow
    key (in facte the key which generate the ascii del character (code 0177)
- New switch "-noX11" to start the Rand editor without any call to X11 library.
- The call to XOpenDisplay is protected by a time out, and can be ended
    with the "INT" key (normally <Ctrl C> key, ref to stty -all).
- The interactive keyboard help display the escape sequence generated by
    the key pushed (when a key function is allocated to this escape sequence).
- New conditional compilation flags :
    NOX11 in order to not include anything from X11 library (which is normaly
      used to get the best result for keyboard mapping.
    NOXKB if the X11 library does not provide X11 keyboard extension
* Mon Mar 13 2000 by Fabien Perriollat <Fabien.Perriollat@cern.ch>
- revision 55
- Support for long files (more than 32767 lines).
- Version available for AIX (IBM Unix) on RS6000 family
- New command : 'files' to display the list of currently edited files
  (eqivalent to the command 'edit ?').
- New key function 'fnavigate' to navigate in the currently edited files list.
  This key is assigned to the key pad '-' key in the provided kbfile
  for xterm family and linux console (see vt200kbn).
- save and restaure in state file (.es1) the full edited file list.
* Mon Mar 06 2000 Fabien Perriollat <Fabien.Perriollat@cern.ch>
- revision 54
  Support for very long files (more than 32767 lines). Beta version
  Not available, use the revision 55
* Fri Mar 03 2000 Fabien Perriollat <Fabien.Perriollat@cern.ch>
- revision 53
- Resize of terminal screen supported (with the restricion
  that it can be done only when a single editing window is used).
- Navigation in the command line history.
- Navigation in the edited file list.
* Mon Jan 31 2000 Fabien Perriollat <Fabien.Perriollat@cern.ch>
- revision 52
- keyboard definition file can use "#include <file_name>" directive.
- Try to use X11 Keyboard Extention for better understanding of key mapping.
- better installation of system wide keyboard configuration files.
* Fri Oct 16 1999 Fabien Perriollat <Fabien.Perriollat@cern.ch>
- revision 51
- package build with relocation capability
- all version and directory dependent Rand internal sysmbols are
    built from the variables defined in this specification file.
    (pacakage name, version, release #, default installation directory)
* Thu Jan  7 1999 Fabien Perriollat <Fabien.Perriollat@cern.ch>
- initial version of the packaging specification.

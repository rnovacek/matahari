%global __strip %{_mingw32_strip}
%global __objdump %{_mingw32_objdump}


%global specversion 5
%global upstream_version e1d0d9a

# Keep around for when/if required
%global alphatag %{upstream_version}.git

%global mh_release %{?alphatag:0.}%{specversion}%{?alphatag:.%{alphatag}}%{?dist}

Name:		mingw32-matahari
Version:	0.4.0
Release:	%{mh_release}
Summary:	Matahari QMF Agents for Windows guests

Group:		Applications/System
License:	GPLv2
URL:		http://fedorahosted.org/matahari
Source0:	matahari-%{version}.tbz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildArch:      noarch

BuildRequires:  redhat-rpm-config cmake make qmf-devel
BuildRequires:  mingw32-filesystem >= 57
BuildRequires:  mingw32-gcc-c++ mingw32-nsis genisoimage
BuildRequires:  mingw32-pcre mingw32-qpid-cpp mingw32-libvirt mingw32-srvany mingw32-netcf

%description

Matahari provides a QMF Agent that can be used to control and manage
various pieces of functionality for an ovirt node, using the AMQP protocol.

The Advanced Message Queuing Protocol (AMQP) is an open standard application
layer protocol providing reliable transport of messages.

QMF provides a modeling framework layer on top of qpid (which implements
AMQP).  This interface allows you to manage a host and its various components
as a set of objects with properties and methods.

MinGW cross-compiled Windows application.

%{_mingw32_debug_package}

%prep
%setup -q -n matahari-%{version}

%build
PATH=%{_mingw32_bindir}:$PATH

ls -al /usr/i686-pc-mingw32/sys-root/mingw/lib/pkgconfig
#%{_mingw32_cmake} -DCMAKE_BUILD_TYPE=RelWithDebInfo
%{_mingw32_cmake} --debug-output -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=on
make VERBOSE=1 %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT

make VERBOSE=1 %{?_smp_mflags} package
genisoimage -o matahari-%{version}-win32.iso matahari-%{version}-win32.exe src/windows/autorun.inf

%{__install} -d $RPM_BUILD_ROOT/%{_mingw32_datadir}/matahari
%{__install} matahari-%{version}-win32.iso $RPM_BUILD_ROOT/%{_mingw32_datadir}/matahari

%files
%defattr(-,root,root,-)
%{_mingw32_datadir}/matahari

%doc AUTHORS COPYING

%changelog
* Tue Oct 12 2010 Andrew Beekhof <andrew@beekhof.net> - matahari-0.4.0-0.3.e1d0d9a.git
- Initial build.

* Fri Sep 10 2010 Andrew Beekhof <andrew@beekhof.net> - matahari-0.4.0-0.1
- Initial build.

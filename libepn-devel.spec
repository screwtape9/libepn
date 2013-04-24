%define major 1
%define minor 3
%define patchlevel 1

%define __os_install_post %{nil}

Name: libepn-devel
Version: %{major}.%{minor}.%{patchlevel}
Release: 1%{?dist}
Summary: An edge-triggered epoll-driven TCP socket server.
Group: System Environment/Libraries

Vendor: Joe Smith
URL: http://virtualmofo.com/~j_smith/libepn
License: BSD

Source0: %{name}-%{version}.tar.gz
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

%description
This package provides the header files and a static debug library for the libepn library.

%prep
%setup -q

%build
cd lib
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
cd lib
make install-devel DESTDIR=%{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
/usr/lib/libepn.%{major}.%{minor}.%{patchlevel}.a
/usr/include/epn

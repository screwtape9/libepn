%define major 1
%define minor 3
%define patchlevel 1

Name: libepn
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
See summary...

%prep
%setup -q

%build
cd lib
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
cd lib
make install DESTDIR=%{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
/usr/lib/libepn.so.%{major}.%{minor}.%{patchlevel}
/usr/lib/libepn.so.%{major}.%{minor}
/usr/lib/libepn.so.%{major}
/usr/lib/libepn.so

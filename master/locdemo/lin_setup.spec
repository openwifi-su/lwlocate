Summary: LocDemo libwlocate demo application
Name: LocDemo
Version: 0.4
Release: 1
License: GPL
Packager: http://www.openwlanmap.org
Group: Applications/Internet
Requires: wxGTK
%description
A WLAN geolocation demonstration application that makes use of libwlocate and the data of project OpenWLANMap. It is recommended to execute this application with root privileges to get more detailed WLAN information and to get a more exact position information.
%files
/etc/init.d/*
/usr/*
%pre
if [ -x /sbin/LocDemo ]; then
   rm -f /sbin/LocDemo
fi
if [ -x /etc/init.d/wlocd ]; then
   /etc/init.d/wlocd stop
fi
%preun
/etc/init.d/wlocd stop
rm /etc/rc5.d/S99wlocd
rm /etc/rc5.d/K01wlocd
rm /etc/rc3.d/S99wlocd
rm /etc/rc3.d/K01wlocd
%post
/etc/init.d/wlocd start 1>/dev/null 2>/dev/null
ln -s /etc/init.d/wlocd /etc/rc5.d/S99wlocd
ln -s /etc/init.d/wlocd /etc/rc5.d/K01wlocd
ln -s /etc/init.d/wlocd /etc/rc3.d/S99wlocd
ln -s /etc/init.d/wlocd /etc/rc3.d/K01wlocd
ldconfig

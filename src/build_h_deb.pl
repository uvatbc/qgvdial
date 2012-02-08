my $cmd;
my $pwd=`cd .. ; pwd`;
chomp $pwd;
my $sedpwd = $pwd;
$sedpwd =~ s/\//\\\//g;

system("rm -rf debian");
system("rm -f ../build-files/qgvdial.Call.service");
system("rm -f ../build-files/qgvdial.Text.service");
system("rm -f ../build-files/qgvdial.desktop");

system("dh_make -p qgvdial_2 --createorig --single -e yuvraaj\@gmail.com -c lgpl");

# Add a post install file to add the executable bit after installation on the device
system("cp ../build-files/postinst.harmattan-qgvdial debian/postinst");
system("cp ../build-files/prerm.harmattan-qgvdial debian/prerm");
# Fix the control file
system("cp ../build-files/control.harmattan-qgvdial debian/control");
# Add the Aegis manifest file
system("cp ../build-files/aegis.harmattan.qgvdial debian/qgvdial.aegis");

system("echo 'override_dh_auto_configure:\n' >> debian/rules");

# Fix the dbus service file name. The same files as maemo can be used
system("cp ../build-files/qgvdial.Call.service.maemo ../build-files/qgvdial.Call.service");
system("cp ../build-files/qgvdial.Text.service.maemo ../build-files/qgvdial.Text.service");
# Change the name of the desktop file so that it can be directly used in the compilation
system("cp ../build-files/qgvdial.desktop.harmattan ../build-files/qgvdial.desktop");

# Replace hard coded current directory with relative directory - pass #1.
$cmd="sed 's/\$\(INSTALL_ROOT\)//g' Makefile | sed 's/$sedpwd\\/src\\/..\\/debian/debian/g' | sed 's/$sedpwd\\/debian/debian/g' >Makefile1 ; mv Makefile1 Makefile";
print "$cmd\n";
system($cmd);
system("touch Makefile");

system("dpkg-buildpackage -rfakeroot -nc -b -us -uc");


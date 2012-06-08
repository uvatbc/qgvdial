my $cmd;
my $pwd=`cd .. ; pwd`;
chomp $pwd;
my $sedpwd = $pwd;
$sedpwd =~ s/\//\\\//g;

system("rm -rf debian");

system("dh_make -n -p qgvdial_2 --createorig --single -e yuvraaj\@gmail.com -c lgpl2");

# Put all the debianization files into the debian folder
system("cd ../build-files/qgvdial/harmattan/ ; cp postinst prerm control qgvdial.aegis ../../../src/debian/");

# Replace hard coded current directory with relative directory - pass #1.
$cmd="sed 's/\$\(INSTALL_ROOT\)//g' Makefile | sed 's/$sedpwd\\/src\\/..\\/debian/debian/g' | sed 's/$sedpwd\\/debian/debian/g' >Makefile1 ; mv Makefile1 Makefile";
print "$cmd\n";
system($cmd);
system("touch Makefile");

system("dpkg-buildpackage -rfakeroot -nc -b -us -uc");


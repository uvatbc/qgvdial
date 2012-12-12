my $cmd;
my $pwd=`cd .. ; pwd`;
chomp $pwd;
my $sedpwd = $pwd;
$sedpwd =~ s/\//\\\//g;

system("rm -rf debian");
system("dh_make -n -p qgvtp_2 --createorig --single -e yuvraaj\@gmail.com -c lgpl2");

# Put all the debianization files into the debian folder
system("cd ../build-files/qgvtp/harmattan/ ; cp postinst prerm control qgvtp.aegis ../../../qgv-tp/debian/");
system("dpkg-buildpackage -rfakeroot -nc -b -us -uc");


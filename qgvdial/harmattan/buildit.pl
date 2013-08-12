system("rm -rf debian ; rm ../qgvdial*armel*");
system("echo y | dh_make -n -p qgvdial_2 --createorig --single -e yuvraaj\@gmail.com -c lgpl2");

# Put all the debianization files into the debian folder
system("cd ./qtc_packaging/debian_harmattan/ ; cp changelog compat control copyright qgvdial.aegis README rules ../../debian/");
# Speed up the make
system("make -j2 >op.make.txt");
system("dpkg-buildpackage -rfakeroot -nc -b -us -uc");


my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $cmd;

# Delete any existing version file
system("rm ver.cfg");
# Get the latest version file from the repository
$cmd = "svn export $repo/build-files/ver.cfg";
system($cmd);

# Pull out the version from the file
open(QVARFILE, "ver.cfg") or die;
my $qver = <QVARFILE>;
close QVARFILE;
chomp $qver;

# Get the subversion checkin version
my $svnver = `svn log $repo --limit=1 | grep \"^r\"`;
# Parse out the version number from the output we pulled out
$svnver =~ m/^r(\d+)*/;
$svnver = $1;
# Create the version suffix
$qver = "$qver.$svnver";

# Get the full path of the base diretory
my $basedir = `pwd`;
chomp $basedir;
$basedir = "$basedir/qgvdial-$qver";

# Delete any previous checkout directories
system("rm -rf qgvdial-* qgvtp-* qgvdial_* qgvtp_*");

$cmd = "svn export $repo $basedir";
print "$cmd\n";
system($cmd);

# Append the version to the pro file
open(PRO_FILE, ">>$basedir/qgvdial/qt-not-qml/desktop_linux.pro") || die "Cannot open pro file";
print PRO_FILE "VERSION=__QGVDIAL_VERSION__\n";
close PRO_FILE;

# Version replacement
$cmd = "perl $basedir/build-files/version.pl __QGVDIAL_VERSION__ $qver $basedir";
print "$cmd\n";
system($cmd);

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd $basedir/qgvdial/qt-not-qml && echo y | dh_make --createorig --single -e yuvraaj\@gmail.com -c lgpl";
print "$cmd\n";
system($cmd);

# Put all the debianization files into the debian folder
system("cd $basedir/build-files/qgvdial/ubuntu ; mv postinst prerm control rules $basedir/qgvdial/qt-not-qml/debian/");

# Fix the changelog and put it into the correct location
system("head -1 $basedir/qgvdial/qt-not-qml/debian/changelog >dest.txt && cat $basedir/build-files/qgvdial/changelog >>dest.txt && tail -2 $basedir/qgvdial/qt-not-qml/debian/changelog | sed 's/unknown/Yuvraaj Kelkar/g' >>dest.txt && mv dest.txt $basedir/qgvdial/qt-not-qml/debian/changelog");

# Built it all!
$cmd = "cd $basedir/qgvdial/qt-not-qml && dpkg-buildpackage -rfakeroot -nc -uc -us";
system($cmd);

exit(0);


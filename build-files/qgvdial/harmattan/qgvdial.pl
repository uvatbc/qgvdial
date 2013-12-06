my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $cmd;

# Delete any existing version file
system("rm -f ver.cfg");
# Get the latest version file from the repository
$cmd = "ssh uv\@userver \"cd harmattan/export/qgvdial ; svn export $repo/build-files/ver.cfg\"";
system($cmd);

# Pull out the version from the file
open(QVARFILE, "ver.cfg") or die;
my $qver = <QVARFILE>;
close QVARFILE;
chomp $qver;

# Get the subversion checkin version
my $svnver = `ssh uv\@userver "svn log $repo --limit=1 | grep \"^r\""`;
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

$cmd = "ssh uv\@userver \"cd ~/harmattan/export/qgvdial ; svn export $repo qgvdial-$qver\"";
system($cmd);

# Append the version to the pro file
open(PRO_FILE, ">>$basedir/qgvdial/harmattan/harmattan.pro") || die "Cannot open pro file";
print PRO_FILE "VERSION=__QGVDIAL_VERSION__\n";
close PRO_FILE;

# Version replacement
$cmd = "perl $basedir/build-files/version.pl __QGVDIAL_VERSION__ $qver $basedir";
print "$cmd\n";
system($cmd);

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd $basedir/qgvdial ; mv harmattan qgvdial-$qver";
print "$cmd\n";
system($cmd);

$basedir = "$basedir/qgvdial/qgvdial-$qver";
$cmd = "cd $basedir && echo y | dh_make -n --createorig --single -e yuvraaj\@gmail.com -c lgpl2";
print "$cmd\n";
system($cmd);

# Put all the debianization files into the debian folder
system("cd $basedir && cd ./qtc_packaging/debian_harmattan/ ; cp changelog compat control copyright qgvdial.aegis README rules ../../debian/");
# Speed up the make
system("cd $basedir ; pushd ../features/dbus_api/gen/ ; ./create_ifaces.sh  ; popd ; qmake ; make -j4");
system("cd $basedir ; dpkg-buildpackage -rfakeroot -nc -b -us -uc");
system("cd $basedir ; cp ../qgvdial*deb ../../..");


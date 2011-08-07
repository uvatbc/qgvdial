my $machine = `uname -m`;
my $mad = '';
my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $cmd;
my $line;
my $qtdir = $ENV{'QTDIR'};

# Delete any existing version file
# Delete any existing version file
system("rm ver.cfg");
# Get the latest version file from the repository
$cmd = "svn export $repo/build-files/ver.cfg";
system($cmd);

# Pull out the version from the file
open(QVARFILE, "ver.cfg") or die;
my $qver = <QVARFILE>;
close QVARFILE;

# Get the subversion checkin version
my $svnver = `svn log $repo --limit=1 | grep \"^r\"`;
# Parse out the version number from the output we pulled out
$svnver =~ m/^r(\d+)*/;
$svnver = $1;
# Create the version suffix
$qver = "$qver.$svnver";
my $basedir = "./qgvdial-$qver";

# Delete any previous checkout directories
system("rm -rf qgvdial* qgvtp*");

$cmd = "svn export $repo $basedir";
system($cmd);
system("cp $basedir/icons/qgv64.png $basedir/src/qgvdial.png");

# Append the version to the pro file
open(PRO_FILE, ">>$basedir/src/src.pro") || die "Cannot open pro file";
print PRO_FILE "VERSION=__QGVDIAL_VERSION__\n";
close PRO_FILE;

# Version replacement
$cmd = "perl $basedir/build-files/version.pl __QGVDIAL_VERSION__ $qver $basedir";
print "$cmd\n";
system($cmd);

# Copy the correct pro file
system("cp $basedir/build-files/pro.qgvdial $basedir/qgvdial.pro");

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd $basedir && $mad qmake && $mad dh_make --createorig --single -e yuvraaj\@gmail.com -c lgpl";
print "$cmd\n";
system($cmd);

# Add a post install file to add the executable bit after installation on the device
system("mv $basedir/build-files/postinst.linux $basedir/debian/postinst");
system("mv $basedir/build-files/prerm.linux $basedir/debian/prerm");
# Fix the control file
system("mv $basedir/build-files/control.linux $basedir/debian/control");
# Fix the dbus service file name
system("mv $basedir/build-files/qgvdial.Call.service.linux $basedir/build-files/qgvdial.Call.service");
system("mv $basedir/build-files/qgvdial.Text.service.linux $basedir/build-files/qgvdial.Text.service");
system("mv $basedir/qgv-tp/data/org.freedesktop.Telepathy.ConnectionManager.qgvtp.service.linux $basedir/qgv-tp/data/org.freedesktop.Telepathy.ConnectionManager.qgvtp.service");

# Fix the changelog and put it into the correct location
system("cp $basedir/debian/changelog ./saved-changelog");
$cmd = "head -1 $basedir/debian/changelog >dest.txt && cat $basedir/build-files/changelog.qgvdial >>dest.txt && tail -2 $basedir/debian/changelog >>dest.txt && mv dest.txt $basedir/debian/changelog";
print("$cmd\n");
system($cmd);

# Make sure all make files are present before mucking with them.
system("cd $basedir && make src/Makefile");

# Replace hard coded current directory with relative directory.

# Execute the rest of the build command
$cmd = "cd $basedir && $mad dpkg-buildpackage && $mad remote -r org.maemo.qgvdial send ../qgvdial_$qver-1_$machine.deb && $mad remote -r org.maemo.qgvdial install qgvdial_$qver-1_$machine.deb";
system($cmd);

exit();

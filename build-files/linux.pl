my $machine = `uname -m`;
my $mad = '';
my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $cmd;
my $line;

# Delete any existing version file
if (-f ver.cfg)
{
    unlink(ver.cfg);
}
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

# Delete any previous checkout directories
system("rm -rf qgvdial*");
$cmd = "svn export $repo qgvdial-$qver";
system($cmd);
system("cp qgvdial-$qver/icons/Google.png qgvdial-$qver/src/qgvdial.png");

# Version replacement
$cmd = "cd qgvdial-$qver ; perl ../version.pl __QGVDIAL_VERSION__ $qver";
print "$cmd\n";
system($cmd);

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd qgvdial-$qver ; $mad qmake && $mad dh_make --createorig --single -e yuvraaj\@gmail.com -c lgpl && $mad qmake";
system($cmd);

# Add a post install file to add the executable bit after installation on the device
system("mv qgvdial-$qver/build-files/postinst.linux qgvdial-$qver/debian/postinst");
# Fix the control file
system("mv qgvdial-$qver/build-files/control.linux qgvdial-$qver/debian/control");
# Fix the dbu service file name
system("mv qgvdial-$qver/build-files/qgvdial.service.linux qgvdial-$qver/build-files/qgvdial.service");

# Execute the rest of the build command
$cmd = "cd qgvdial-$qver && $mad dpkg-buildpackage && $mad remote -r org.maemo.qgvdial send ../qgvdial_$qver-1_$machine.deb && $mad remote -r org.maemo.qgvdial install qgvdial_$qver-1_$machine.deb";
system($cmd);

exit();


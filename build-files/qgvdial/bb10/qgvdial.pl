my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $cmd;

# Delete any existing version file
system("rm -f ver.cfg");
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
system("rm -rf qgvdial-* qgvtp-* qgvdial_* qgvtp_* qgvdial.bar");

$cmd = "svn export $repo qgvdial-$qver";
system($cmd);

# Version replacement
$cmd = "perl $basedir/build-files/version.pl __QGVDIAL_VERSION__ $qver $basedir";
print "$cmd\n";
system($cmd);

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd $basedir/qgvdial/bb10-qml ; qmake ; make -j4 ; ./run-package.sh";
print "$cmd\n";
system($cmd);


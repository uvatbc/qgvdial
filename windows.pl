my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $nsis = "& 'C:\\Program Files (x86)\\NSIS\\makensis.exe'";
my $cmd;
my $line;

# Delete any existing version file
if (-f ver.cfg)
{
    unlink(ver.cfg);
}
# Get the latest version file from the repository
$cmd = "svn export $repo/ver.cfg";
system($cmd);

# Pull out the version from the file
open(QVARFILE, "ver.cfg") or die;
my $qver = <QVARFILE>;
close QVARFILE;

# Get the subversion checkin version
system("svn log $repo --limit=1 | grep \"^r\" > svnlog.txt");
open(QVARFILE, "svnlog.txt") or die;
my $svnver = <QVARFILE>;
close QVARFILE;
unlink "svnlog.txt";

# Parse out the version number from the output we pulled out
$svnver =~ m/^r(\d+)*/;
$svnver = $1;
# Create the version suffix
$qver = "$qver.$svnver";

system("rm -rf qgvdial*");
$cmd = "svn export $repo qgvdial-$qver";
system($cmd);
system("cp qgvdial-$qver/icons/Google.png qgvdial-$qver/src/qgvdial.png");

$cmd = "cd qgvdial-$qver ; perl ../version.pl __QGVDIAL_VERSION__ $qver";
print "$cmd\n";
system($cmd);

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd qgvdial-$qver & qmake & make all";
system($cmd);

$cmd = "copy qgvdial-$qver/src/release/qgvdial.exe I:/Uv/releases/qgvdial/win-install/qgvdial/bin";
system($cmd);

$cmd = "$nsis qgvdial-$qver/src/setup.nsi";
system($cmd);

my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $nsis = "\"C:\\Program Files (x86)\\NSIS\\makensis.exe\"";
my $wixbase='C:/Program Files (x86)/Windows Installer XML v3/bin';
my $cmd;
my $line;

# Delete any existing version file
if (-f ver.cfg) { unlink(ver.cfg); }
# Get the latest version file from the repository
system("svn export $repo/build-files/ver.cfg");

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

system("powershell Remove-Item -Recurse -Force qgvdial*");
system("svn export $repo qgvdial-$qver");
system("copy qgvdial-$qver\\icons\\Google.png qgvdial-$qver\\src\\qgvdial.png");
system("move qgvdial-$qver\\build-files\\qt.conf.win qgvdial-$qver\\build-files\\qt.conf");

system("cd qgvdial-$qver & perl build-files/version.pl __QGVDIAL_VERSION__ $qver");
$cmd = `echo %QTDIR%`;
$cmd =~ s/\\/\\\\/g;
$cmd = "cd qgvdial-$qver & perl build-files/version.pl __QTDIR__ $cmd";
system($cmd);

# Compile it!
$cmd = "cd qgvdial-$qver & qmake & make release";
system($cmd);

$cmd = "copy qgvdial-$qver\\src\\release\\qgvdial.exe I:\\Uv\\releases\\qgvdial\\win-install\\qgvdial\\bin";
system($cmd);

# Old setup
#system("$nsis qgvdial-$qver\\src\\setup.nsi");

# New setup: Create MSI.
system("cd qgvdial-$qver/build-files & \"$wixbase/candle.exe\" qgvdial.wxs");
system("cd qgvdial-$qver/build-files & \"$wixbase/light.exe\" qgvdial.wixobj -o ..\\qgvdial-$qver.msi");

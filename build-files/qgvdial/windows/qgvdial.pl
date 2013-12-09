my $repo = "https://qgvdial.googlecode.com/svn/trunk";
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
chomp $qver;

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

system("powershell Remove-Item -Recurse -Force qgvdial-*");
system("svn export $repo qgvdial-$qver");
system("move qgvdial-$qver\\build-files\\qt.conf.win qgvdial-$qver\\build-files\\qt.conf");

# Append the version to the pro file
open(PRO_FILE, ">>qgvdial-$qver/qgvdial/qt-not-qml/desktop_windows.pro") || die "Cannot open pro file";
print PRO_FILE "VERSION=__QGVDIAL_VERSION__\n";
close PRO_FILE;

# Version replacement
system("cd qgvdial-$qver & perl build-files/version.pl __QGVDIAL_VERSION__ $qver");

# Replace the QTDIR variable
$cmd = `echo %QTDIR%`;
$cmd =~ s/\\/\\\\/g;
$cmd = "cd qgvdial-$qver & perl build-files/version.pl __QTDIR__ $cmd";
system($cmd);

# Compile it!
$cmd = "cd qgvdial-$qver/qgvdial/qt-not-qml/desktop_windows & qmake ../desktop_windows.pro & nmake -nologo release";
system($cmd);

# this is required for the old installer
$cmd = "copy qgvdial-$qver\\qgvdial\\qt-not-qml\\desktop_windows\\release\\qgvdial.exe J:\\releases\\qgvdial\\win-install\\qgvdial\\bin";
system($cmd);

# New setup: Create MSI.
system("cd qgvdial-$qver/build-files/qgvdial/windows & \"$wixbase/candle.exe\" qgvdial.wxs");
system("cd qgvdial-$qver/build-files/qgvdial/windows & \"$wixbase/light.exe\" -cultures:en-US -ext WixUIExtension qgvdial.wixobj -o ..\\qgvdial-$qver.msi");

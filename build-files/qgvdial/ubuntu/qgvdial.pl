#!/usr/bin/perl

use DateTime;
my $dt = DateTime->now;
my $dtstr = sprintf("%s, %d %s %d %s", $dt->day_abbr(), $dt->day(), $dt->month_abbr(), $dt->year(), $dt->hms());

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

# Date replacement
$cmd = "perl $basedir/build-files/version.pl __CHANGELOG_DATETIME__ $dtstr $basedir";
print "$cmd\n";
system($cmd);

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "mv $basedir/qgvdial/qt-not-qml $basedir/qgvdial/qgvdial-$qver";
print "$cmd\n";
system($cmd);
$cmd = "cd $basedir/qgvdial/qgvdial-$qver && echo y | dh_make --createorig --single -e yuvraaj\@gmail.com -c lgpl";
print "$cmd\n";
system($cmd);

# Put all the debianization files into the debian folder
$cmd = "cd $basedir/build-files/qgvdial/ubuntu ; mv postinst prerm control rules changelog $basedir/qgvdial/qgvdial-$qver/debian/";
print "$cmd\n";
system("$cmd");
$cmd = "cd $basedir/build-files/qgvdial ; cp changelog.txt $basedir/qgvdial/qgvdial-$qver/debian/changelog";
print "$cmd\n";
system("$cmd");

# Built it all!
$cmd = "cd $basedir/qgvdial/qgvdial-$qver && dpkg-buildpackage -rfakeroot -nc -uc -us";
print "$cmd\n";
system($cmd);

$cmd = "cp $basedir/qgvdial/qgvdial*deb .";
print "$cmd\n";
system($cmd);

exit(0);


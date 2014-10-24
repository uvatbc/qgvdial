#!/usr/bin/perl

my @months = qw( Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec );
my @days = qw(Sun Mon Tue Wed Thu Fri Sat Sun);
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime();
my $dtstr = sprintf("%s, %d %s %d %02d:%02d:%02d", $days[$wday], $mday, $months[$mon], $year+1900, $hour, $min, $sec);

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
$cmd = "perl version.pl __QGVDIAL_VERSION__ $qver $basedir";
print "$cmd\n";
system($cmd);

# Date replacement
$cmd = "perl version.pl __CHANGELOG_DATETIME__ '$dtstr' $basedir";
print "$cmd\n";
system($cmd);

# Cipher replacement
open my $qgvcipfile, '<', "cipher_qgvdial";
my $cipher = <$qgvcipfile>;
close $qgvcipfile;
chomp $cipher;
$cmd = "perl version.pl __THIS_IS_MY_EXTREMELY_LONG_KEY_ '$cipher' $basedir";
print "$cmd\n";
system($cmd);

# Mixpanel replacement
open my $mixpanel_token_file, '<', "mixpanel.token";
my $mixpanel_token = <$mixpanel_token_file>;
close $mixpanel_token_file;
chomp $mixpanel_token;
$cmd = "perl version.pl __MY_MIXPANEL_TOKEN__ '$mixpanel_token' $basedir";
print "$cmd\n";
system($cmd);

# Copy the client secret file to the api directory
$cmd = "cd $basedir/api ; cp ../../client_secret_284024172505-2go4p60orvjs7hdmcqpbblh4pr5thu79.apps.googleusercontent.com.json .";
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
$cmd = "cd $basedir/build-files/qgvdial/ubuntu ; mv control rules changelog $basedir/qgvdial/qgvdial-$qver/debian/";
print "$cmd\n";
system("$cmd");
$cmd = "cd $basedir/build-files/qgvdial ; cp changelog.txt $basedir/qgvdial/qgvdial-$qver/debian/changelog";
print "$cmd\n";
system("$cmd");

# Speed up the make
system("cd $basedir/qgvdial/qgvdial-$qver ; ../features/dbus_api/gen/create_ifaces.sh  ; qmake ; make -j4");

# Built it all!
$cmd = "cd $basedir/qgvdial/qgvdial-$qver && dpkg-buildpackage -rfakeroot -nc -uc -us";
print "$cmd\n";
system($cmd);

$cmd = "cp $basedir/qgvdial/qgvdial*deb .";
print "$cmd\n";
system($cmd);

exit(0);


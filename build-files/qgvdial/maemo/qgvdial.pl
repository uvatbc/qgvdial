#!/usr/bin/perl

my @months = qw( Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec );
my @days = qw(Sun Mon Tue Wed Thu Fri Sat Sun);
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime();
my $dtstr = sprintf("%s, %d %s %d %02d:%02d:%02d", $days[$wday], $mday, $months[$mon], $year+1900, $hour, $min, $sec);

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

# Get the full path of the base directory
my $basedir = `pwd`;
chomp $basedir;
$basedir = "$basedir/qgvdial-$qver";

# Delete any previous checkout directories
system("rm -rf qgvdial-* qgvtp-* qgvdial_* qgvtp_* version.pl");

system("svn export $repo $basedir");
system("cp $basedir/build-files/version.pl .");

# Append the version to the pro file
open(PRO_FILE, ">>$basedir/qgvdial/maemo/maemo.pro") || die "Cannot open pro file";
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

# NO Cipher replacement for Maemo: The source is publicly available.

# Mixpanel replacement
open my $mixpanel_token_file, '<', "mixpanel.token";
my $mixpanel_token = <$mixpanel_token_file>;
close $mixpanel_token_file;
chomp $mixpanel_token;
$cmd = "perl version.pl __MY_MIXPANEL_TOKEN__ '$mixpanel_token' $basedir";
print "$cmd\n";
system($cmd);

# Copy the client secret file to the api directory :(
$cmd = "cd $basedir/api ; cp ../../client_secret_284024172505-2go4p60orvjs7hdmcqpbblh4pr5thu79.apps.googleusercontent.com.json .";
print "$cmd\n";
system($cmd);

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd $basedir && echo y | dh_make -n --createorig --single -e yuvraaj\@gmail.com -c lgpl2";
print "$cmd\n";
system($cmd);

# Put all the debianization files into the debian folder
system("cd $basedir/qgvdial/maemo/qtc_packaging/debian_fremantle/ ; cp compat control copyright README rules ../../../../debian/");
system("cd $basedir/debian ; cp ../build-files/qgvdial/changelog.txt changelog");

# Copy the top level pro file
system("cp $basedir/build-files/qgvdial/maemo/maemo.pro $basedir");

# Reverse the order of these two lines for a complete build 
$cmd = "cd $basedir && dpkg-buildpackage -rfakeroot";
$cmd = "cd $basedir && dpkg-buildpackage -rfakeroot -sa -S -uc -us";
# Execute the rest of the build command
system($cmd);

system("cd $basedir/.. ; dput -f fremantle-upload qgvdial*.changes");

exit(0);


#!/usr/bin/perl

my @months = qw( Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec );
my @days = qw(Sun Mon Tue Wed Thu Fri Sat Sun);
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime();
my $dtstr = sprintf("%s, %d %s %d %02d:%02d:%02d", $days[$wday], $mday, $months[$mon], $year+1900, $hour, $min, $sec);

my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $cmd;

# Delete any existing version file
system("rm ver.cfg version.pl");
# Get the latest version file from the repository
$cmd = "svn export $repo/build-files/ver.cfg";
system($cmd);
$cmd = "svn export $repo/build-files/version.pl";
system($cmd);

# Pull out the version from the file
open(QVARFILE, "ver.cfg") or die;
my $qver = <QVARFILE>;
close QVARFILE;
chomp $qver;
my $qverbase = $qver;

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
system("rm -rf qgvdial-* qgvtp-* qgvdial_* qgvtp_* *rpm");

$cmd = "svn export $repo $basedir";
print "$cmd\n";
system($cmd);

# Append the version to the pro file
open(PRO_FILE, ">>$basedir/qgvdial/sailfish/sailfish.pro") || die "Cannot open pro file";
print PRO_FILE "VERSION=__QGVDIAL_VERSION__\n";
close PRO_FILE;

# Version replacement
$cmd = "perl version.pl __QGVDIAL_VERSION__ $qver $basedir";
print "$cmd\n";
system($cmd);
$cmd = "perl version.pl __QVER_BASE__ $qverbase $basedir";
print "$cmd\n";
system($cmd);
$cmd = "perl version.pl __QVER_SVN__ $svnver $basedir";
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

# Copy the client secret file to the api directory
$cmd = "cd $basedir/api ; cp ../../client_secret_284024172505-2go4p60orvjs7hdmcqpbblh4pr5thu79.apps.googleusercontent.com.json .";
print "$cmd\n";
system($cmd);

# Put the yaml file into the correct folder
$cmd = "cp $basedir/build-files/qgvdial/sailfish/sailfish.yaml $basedir/qgvdial/sailfish/rpm/sailfish.yaml";
print "$cmd\n";
system("$cmd");

# Start mersdk if it isn't already on.
$cmd = "/usr/bin/VBoxManage -nologo list runningvms | grep MerSDK | wc -l";
print "$cmd\n";
my $was_on = `$cmd`;

if ($was_on == 0) {
    $cmd = "/usr/bin/VBoxManage startvm MerSDK --type headless";
    print "$cmd\n";
    system("$cmd");
}

# Wait for localhost ssh port
my $count = 30;
while ($count > 0) {
    $cmd = `nmap -p 2222 localhost | grep 2222 | grep open`;
    if (length($cmd) != 0) {
        print "\nIt is on!\n";
        last;
    }
    print "$count... ";
    $count = $count - 1;
}

# Wait for 10 seconds even after it is on...
print "Waiting for 10 more seconds\n";
system("sleep 10");

# Create the dbus interface
$cmd = "cd $basedir/qgvdial/features/dbus_api/gen/ ; ./create_ifaces.sh";
print "$cmd\n";
system("$cmd");

my $basessh = "ssh -p 2222 -i ~/bin/SailfishOS/vmshare/ssh/private_keys/engine/mersdk mersdk\@localhost";
my $merbasedir = "/home/mersdk/share/code/exports/sailfish/qgvdial/qgvdial-$qver";

$cmd = "$basessh 'cd $merbasedir/qgvdial/sailfish ; mb2 -t SailfishOS-armv7hl qmake sailfish.pro ; mb2 -t SailfishOS-armv7hl make -j2 ; mb2 -t SailfishOS-armv7hl rpm'";
print "$cmd\n";
system("$cmd");

# Stop mersdk
if ($was_on == 0) {
    $cmd = "/usr/bin/VBoxManage controlvm MerSDK acpipowerbutton";
    print "$cmd\n";
    system("$cmd");
}

# Find the rpm
$cmd = "find . | grep RPMS | grep rpm\$ | grep -v debug";
print "$cmd\n";
$cmd = `$cmd`;
chomp $cmd; chomp $cmd;

if ($cmd =~ /qgvdial-$qver.armv7hl.rpm/) {
    print("Successfully built!\n");
} else {
    print "Failed to create rpm. Die.\n";
    die;
}

system("cp $cmd ./qgvdial-$qver.sailfishos.armv7hl.rpm");

exit(0);


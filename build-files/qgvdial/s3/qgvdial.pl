my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $s_make = "make.exe";
my $cmd;
my $dest;

my ($sec,$min,$hour,$day,$month,$yr19,@rest) = localtime(time);
$yr19 += 1900;
$month++;
my $suffix = sprintf("%04d%02d%02d", $yr19, $month, $day);
$dest = sprintf("J:/releases/qgvdial/%04d-%02d-%02d", $yr19, $month, $day);
if ((!(-e $dest)) or (!(-d $dest))) {
    $cmd = "powershell mkdir $dest";
    print("$cmd\n");
    system($cmd);
}

# Get the latest version file from the repository
$cmd = "del ver.cfg & svn export $repo/build-files/ver.cfg";
system($cmd);

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

$cmd = "svn export $repo qgvdial-$qver";
print("$cmd\n");
system($cmd);

# Append the version to the pro file
open(PRO_FILE, ">>qgvdial-$qver/qgvdial/symbian/symbian.pro") || die "Cannot open pro file";
print PRO_FILE "VERSION=__QGVDIAL_VERSION__\n";
close PRO_FILE;

# Version replacement
$cmd = "cd qgvdial-$qver & perl ./build-files/version.pl __QGVDIAL_VERSION__ $qver";
print("$cmd\n");
system($cmd);

# Cipher replacement
open my $qgvcipfile, '<', "../cipher_qgvdial";
my $cipher = <$qgvcipfile>;
close $qgvcipfile;
chomp $cipher;
$cmd = "perl qgvdial-$qver/build-files/version.pl __THIS_IS_MY_EXTREMELY_LONG_KEY_ \"$cipher\" qgvdial-$qver";
print "$cmd\n";
system($cmd);

# Mixpanel replacement
open my $mixpanel_token_file, '<', "../mixpanel.token";
my $mixpanel_token = <$mixpanel_token_file>;
close $mixpanel_token_file;
chomp $mixpanel_token;
$cmd = "perl qgvdial-$qver/build-files/version.pl __MY_MIXPANEL_TOKEN__ '$mixpanel_token' $basedir";
print "$cmd\n";
system($cmd);

# Copy the client secret file to the api directory
$cmd = "powershell cp ../client_secret_284024172505-2go4p60orvjs7hdmcqpbblh4pr5thu79.apps.googleusercontent.com.json qgvdial-$qver/api";
print "$cmd\n";
system($cmd);

# Mosquitto is merged straight into the build
$cmd = "cd qgvdial-$qver/qgvdial/symbian & prep_s3.bat";
print "$cmd\n";
system($cmd);

# Symbian 3 (to including Belle) should not have qt-components
$cmd = "cd qgvdial-$qver/src & echo something>no-qt-components";
print "$cmd\n";
system($cmd);

# This is the way to enter a directory and setup the remainder variables
my $enterdir = "cd qgvdial-$qver/qgvdial/symbian & set BUILDDIR=%CD% & set SOURCEDIR=%CD%";

# qmake, make release-gcce, make installer_sis
$cmd = "$enterdir & qmake.exe symbian.pro -r -spec symbian-sbsv2 CONFIG+=release -after OBJECTS_DIR=obj MOC_DIR=moc UI_DIR=ui RCC_DIR=rcc & $s_make release-gcce -w & $s_make installer_sis -w QT_SIS_CERTIFICATE=%QGV_CERT% QT_SIS_KEY=%QGV_KEY%";
print("$cmd\n");
system($cmd);

$cmd = "$enterdir & signsis.exe -u qgvdial.sis unsigned_qgvdial.sis";
print("$cmd\n");
system($cmd);

# Copy the sis files to the outer directory
$cmd = "$enterdir & powershell cp qgvdial.sis $dest/qgvdial_s3_$qver.sis & powershell cp unsigned_qgvdial.sis $dest/unsigned_qgvdial_s3_$qver.sis & powershell cp qgvdial_installer.sis $dest/qgvdial_installer_s3_$qver.sis";
print("$cmd\n");
system($cmd);

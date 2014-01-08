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
$cmd = "perl version.pl __QGVDIAL_VERSION__ $qver $basedir";
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

# Prepare the bar-descriptor
$cmd = "cd $basedir/qgvdial/bb10-qml ; mv bar-descriptor-deploy.xml bar-descriptor.xml";
print "$cmd\n";
system($cmd);

$cmd = "cd $basedir/qgvdial/bb10-qml ; qmake ; make -j4 ; blackberry-nativepackager -package qgvdial.bar bar-descriptor.xml -debugToken /media/drobo/uv/work/bb_dev_cert/z10-dbgtoken.bar ; cp qgvdial.bar ../../..";
print "$cmd\n";
system($cmd);

$cmd = "cd $basedir ; mkdir -p bundle/qgvdial_10.1.0.4200 ; cp qgvdial.bar bundle/qgvdial_10.1.0.4200 ; cp build-files/qgvdial/bb10/release.xml bundle ; mv bundle ..";
print "$cmd\n";
system($cmd);


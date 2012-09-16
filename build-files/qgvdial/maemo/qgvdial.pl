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
system("rm -rf qgvdial-* qgvtp-* qgvdial_* qgvtp_*");

system("svn export $repo $basedir");
system("cp $basedir/icons/64/qgvdial.png $basedir/src/qgvdial.png");

# Append the version to the pro file
open(PRO_FILE, ">>$basedir/src/src.pro") || die "Cannot open pro file";
print PRO_FILE "VERSION=__QGVDIAL_VERSION__\n";
close PRO_FILE;

# Version replacement
$cmd = "perl $basedir/build-files/version.pl __QGVDIAL_VERSION__ $qver $basedir";
print "$cmd\n";
system($cmd);

# Copy the correct pro file
system("cp $basedir/build-files/qgvdial/pro.qgvdial $basedir/qgvdial.pro");

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd $basedir && echo y | dh_make --createorig --single -e yuvraaj\@gmail.com -c lgpl";
print "$cmd\n";
system($cmd);

# Put all the debianization files into the debian folder
system("cd $basedir/build-files/qgvdial/maemo ; mv postinst prerm control rules $basedir/debian/");

# Fix the changelog and put it into the correct location
system("head -1 $basedir/debian/changelog >dest.txt && cat $basedir/build-files/qgvdial/changelog >>dest.txt && tail -2 $basedir/debian/changelog | sed 's/unknown/Yuvraaj Kelkar/g' >>dest.txt && mv dest.txt $basedir/debian/changelog");

# Reverse the order of these two lines for a complete build 
$cmd = "cd $basedir && dpkg-buildpackage -rfakeroot";
$cmd = "cd $basedir && dpkg-buildpackage -rfakeroot -sa -S -uc -us";
# Execute the rest of the build command
system($cmd);

$cmd = "dput -f fremantle-upload qgvdial*.changes";
system($cmd);

exit(0);


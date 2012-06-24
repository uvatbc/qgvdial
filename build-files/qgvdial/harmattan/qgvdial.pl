my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $cmd;

# Delete any existing version file
system("rm -f ver.cfg");
# Get the latest version file from the repository
$cmd = "ssh uv\@userver_x86 \"cd harmattan/export/qgvdial ; svn export $repo/build-files/ver.cfg\"";
system($cmd);

# Pull out the version from the file
open(QVARFILE, "ver.cfg") or die;
my $qver = <QVARFILE>;
close QVARFILE;
chomp $qver;

# Get the subversion checkin version
my $svnver = `ssh uv\@userver_x86 "svn log $repo --limit=1 | grep \"^r\""`;
# Parse out the version number from the output we pulled out
$svnver =~ m/^r(\d+)*/;
$svnver = $1;
# Create the version suffix
$qver = "$qver.$svnver";

# Get the full path of the base diretory
my $basedir = `pwd`;
chomp $basedir;
$basedir = "$basedir/qgvnotify-$qver";

# Delete any previous checkout directories
system("rm -rf qgvdial-* qgvtp-* qgvdial_* qgvtp_*");

$cmd = "ssh uv\@userver_x86 \"cd harmattan/export/qgvdial ; svn export $repo $basedir\"";
system($cmd);
system("cp $basedir/icons/qgv.png $basedir/src/qgvdial.png");

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

# For the moment, forcibly include mqlib
$cmd = "cd $basedir/src && touch mqlib-build";
print "$cmd\n";
system($cmd);

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd $basedir && echo y | dh_make -n --createorig --single -e yuvraaj\@gmail.com -c lgpl2";
print "$cmd\n";
system($cmd);

# Put all the debianization files into the debian folder
system("cd $basedir/build-files/qgvdial/harmattan ; mv postinst prerm control qgvdial.aegis rules $basedir/debian/");

# Fix the changelog and put it into the correct location
system("head -1 $basedir/debian/changelog >dest.txt && cat $basedir/build-files/qgvdial/changelog >>dest.txt && tail -2 $basedir/debian/changelog | sed 's/unknown/Yuvraaj Kelkar/g' >>dest.txt && mv dest.txt $basedir/debian/changelog");

# Built it all!
$cmd = "cd $basedir && dpkg-buildpackage -rfakeroot -nc -uc -us";
system($cmd);

exit(0);


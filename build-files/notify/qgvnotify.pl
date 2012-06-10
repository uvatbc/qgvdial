my $target = $ARGV[0];
if (($target eq "") || (($target ne "fremantle") && ($target ne "diablo"))) {
    print "Need target: Either maemo or diablo\n";
    exit();
}

my $build_file_dir = $target;
if ($target eq "fremantle") {
    $build_file_dir = "maemo";
}

my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $cmd;
my $line;

# Delete any existing version file
if (-f ver.cfg) {
    unlink(ver.cfg);
}
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
$basedir = "$basedir/qgvnotify-$qver";

# Delete any previous checkout directories
system("rm -rf qgvnotify-* qgvnotify_*");
$cmd = "svn export $repo $basedir";
system($cmd);
system("cp $basedir/icons/qgv.png $basedir/src/qgvdial.png");

# Append the version to the pro file
open(PRO_FILE, ">>$basedir/notify/notify.pro") || die "Cannot open pro file";
print PRO_FILE "VERSION=__QGVDIAL_VERSION__\n";
close PRO_FILE;

# Version replacement
$cmd = "cd $basedir ; perl ./build-files/version.pl __QGVDIAL_VERSION__ $qver";
print "$cmd\n";
system($cmd);

# Remove all bak files
$cmd = "for i in `find . | grep bak` ; do echo $i ; rm $i; done";
print "$cmd\n";
system($cmd);

# Copy the correct pro file
system("cp $basedir/build-files/notify/pro.qgvnotify $basedir/qgvdial.pro");

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd $basedir ; qmake && echo y | dh_make --createorig --single -e yuvraaj\@gmail.com -c lgpl && qmake";
system($cmd);

# Put all the debianization files into the debian folder
$cmd = "cd $basedir/build-files/notify/$build_file_dir ; mv postinst prerm control rules $basedir/debian/";
print "$cmd\n";
system($cmd);

# Fix the changelog and put it into the correct location
system("head -1 $basedir/debian/changelog >dest.txt && cat $basedir/build-files/notify/changelog >>dest.txt && tail -2 $basedir/debian/changelog | sed 's/unknown/Yuvraaj Kelkar/g' >>dest.txt && mv dest.txt $basedir/debian/changelog");

# Reverse the order of these two lines for a complete build 
$cmd = "cd $basedir && dpkg-buildpackage -rfakeroot";
$cmd = "cd $basedir && dpkg-buildpackage -rfakeroot -sa -S";
# Execute the rest of the build command
system($cmd);

$cmd = "dput -f $target-extras-builder qgvnotify*.changes";
print "$cmd\n";
system($cmd);

exit();


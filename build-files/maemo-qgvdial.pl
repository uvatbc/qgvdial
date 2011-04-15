my $machine = `uname -m`;
chomp $machine;
my $mad;
my $asroot;
if ($machine ne "arm") {
    $mad = '/home/uv/apps/mad';
} else {
    $asroot = "fakeroot";
}

my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $cmd;
my $line;

# Delete any existing version file
if (-f ver.cfg)
{
    unlink(ver.cfg);
}
# Get the latest version file from the repository
$cmd = "svn export $repo/build-files/ver.cfg";
system($cmd);

# Pull out the version from the file
open(QVARFILE, "ver.cfg") or die;
my $qver = <QVARFILE>;
close QVARFILE;

# Get the subversion checkin version
my $svnver = `svn log $repo --limit=1 | grep \"^r\"`;
# Parse out the version number from the output we pulled out
$svnver =~ m/^r(\d+)*/;
$svnver = $1;
# Create the version suffix
$qver = "$qver.$svnver";
my $basedir = "./qgvdial-$qver";

# Delete any previous checkout directories
system("rm -rf qgvdial*");
$cmd = "svn export $repo $basedir";
system($cmd);
system("cp $basedir/icons/Google.png $basedir/src/qgvdial.png");

# Append the version to the pro file
open(PRO_FILE, ">>$basedir/src/src.pro") || die "Cannot open pro file";
print PRO_FILE "VERSION=__QGVDIAL_VERSION__\n";
close PRO_FILE;

# Version replacement
$cmd = "cd $basedir ; perl ./build-files/version.pl __QGVDIAL_VERSION__ $qver";
print "$cmd\n";
system($cmd);

# Fix the QML files
$cmd = "cd $basedir ; perl ./build-files/fixqml.pl ./qml";
print "$cmd\n";
system($cmd);

# Copy the correct pro file
system("cp $basedir/build-files/pro.qgvdial $basedir/qgvdial.pro");

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd $basedir ; $mad qmake && echo y | $mad dh_make --createorig --single -e yuvraaj\@gmail.com -c lgpl && $mad qmake";
system($cmd);

# Add a post install file to add the executable bit after installation on the device
system("mv $basedir/build-files/postinst.maemo-qgvdial $basedir/debian/postinst");
system("mv $basedir/build-files/prerm.maemo-qgvdial $basedir/debian/prerm");
# Fix the control file
system("mv $basedir/build-files/control.maemo-qgvdial $basedir/debian/control");
# Fix the dbus service file name
system("mv $basedir/build-files/qgvdial.Call.service.maemo $basedir/build-files/qgvdial.Call.service");
system("mv $basedir/build-files/qgvdial.Text.service.maemo $basedir/build-files/qgvdial.Text.service");
# Change the name of the desktop file so that it can be directly used in the compilation
system("mv $basedir/build-files/qgvdial.desktop.maemo $basedir/build-files/qgvdial.desktop");

system("head -1 $basedir/debian/changelog >dest.txt ; cat $basedir/build-files/changelog >>dest.txt ; tail -2 $basedir/debian/changelog | head -1 | sed 's/unknown/Yuvraaj Kelkar/g' >>dest.txt ; mv dest.txt $basedir/debian/changelog");

if ($machine eq "arm") {
    # Make sure all make files are present before mucking with them.
    system("cd $basedir ; make src/Makefile qgv-tp/Makefile qgv-util/Makefile");

    # Strip out "/targets/FREMANTLE_ARMEL/"
    $cmd="sed 's/\\/targets\\/FREMANTLE_ARMEL//g' $basedir/Makefile >$basedir/Makefile1 ; mv $basedir/Makefile1 $basedir/Makefile ; sed 's/\\/targets\\/FREMANTLE_ARMEL//g' $basedir/qgv-tp/Makefile >$basedir/qgv-tp/Makefile1 ; mv $basedir/qgv-tp/Makefile1 $basedir/qgv-tp/Makefile ; sed 's/\\/targets\\/FREMANTLE_ARMEL//g' $basedir/qgv-util/Makefile >$basedir/qgv-util/Makefile1 ; mv $basedir/qgv-util/Makefile1 $basedir/qgv-util/Makefile ; sed 's/\\/targets\\/FREMANTLE_ARMEL//g' $basedir/src/Makefile >$basedir/src/Makefile1 ; mv $basedir/src/Makefile1 $basedir/src/Makefile";
    print "$cmd\n";
    system($cmd);

    $cmd=`pwd`;
    chomp $cmd;
    $cmd =~ s/\//\\\//g;

    # Replace hard coded current directory with relative directory.
    $cmd="sed 's/$cmd\\/qgvdial-$qver/../g' $basedir/Makefile >$basedir/Makefile1 ; mv $basedir/Makefile1 $basedir/Makefile ; sed 's/$cmd\\/qgvdial-$qver/../g' $basedir/qgv-tp/Makefile >$basedir/qgv-tp/Makefile1 ; mv $basedir/qgv-tp/Makefile1 $basedir/qgv-tp/Makefile ; sed 's/$cmd\\/qgvdial-$qver/../g' $basedir/qgv-util/Makefile >$basedir/qgv-util/Makefile1 ; mv $basedir/qgv-util/Makefile1 $basedir/qgv-util/Makefile ; sed 's/$cmd\\/qgvdial-$qver/../g' $basedir/src/Makefile >$basedir/src/Makefile1 ; mv $basedir/src/Makefile1 $basedir/src/Makefile";
    print "$cmd\n";
    system($cmd);

    # Remove the GLESv2 dependency
    $cmd="sed 's/ -lGLESv2//g' $basedir/Makefile >$basedir/Makefile1 ; mv $basedir/Makefile1 $basedir/Makefile ; sed 's/ -lGLESv2//g' $basedir/qgv-tp/Makefile >$basedir/qgv-tp/Makefile1 ; mv $basedir/qgv-tp/Makefile1 $basedir/qgv-tp/Makefile ; sed 's/ -lGLESv2//g' $basedir/qgv-util/Makefile >$basedir/qgv-util/Makefile1 ; mv $basedir/qgv-util/Makefile1 $basedir/qgv-util/Makefile ; sed 's/ -lGLESv2//g' $basedir/src/Makefile >$basedir/src/Makefile1 ; mv $basedir/src/Makefile1 $basedir/src/Makefile";
    print "$cmd\n";
    system($cmd);

    # Reverse the order of these two lines for a complete build 
    $cmd = "cd $basedir && dpkg-buildpackage -rfakeroot";
    $cmd = "cd $basedir && dpkg-buildpackage -rfakeroot -sa -S";
} else {
    $cmd = "cd $basedir && $asroot $mad dpkg-buildpackage -rfakeroot";
}
# Execute the rest of the build command
system($cmd);

if ($machine ne "arm") {
    $cmd = "dput -f fremantle-upload qgvdial*.changes";
} else {
    $cmd = "dput -f fremantle-extras-builder qgvdial*.changes";
}
system($cmd);

exit();


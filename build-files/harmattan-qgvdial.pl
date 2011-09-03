use Cwd;

my $machine = `uname -m`; chomp $machine;
my $mad;
my $asroot;
my $qtsdk;
my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $cmd;
my $line;
my $pathreplace;
my $curdir = getcwd();

if ($machine ne "arm") {
    $qtsdk = $ENV{'QTSDK'};
    if ($qtsdk eq undef) {
        die "Need to specify QTSDK environment variable";
    }
    
    if (`hostname` =~ m/win/) { # This is the only way I know to differentiate the build machine as windows
        $pathreplace = "$qtsdk/Maemo/4.6.2/sysroots/fremantle-arm-sysroot-20.2010.36-2-slim";
        $curdir =~ s/^\/c/C\:/i;
    }

    $mad = "mad"; 
} else {
    $asroot = "fakeroot";
    $pathreplace = "/targets/FREMANTLE_ARMEL";
}
$pathreplace =~ s/\//\\\//g;
$curdir =~ s/\//\\\//g;

# Delete any existing version file
system("rm -f ver.cfg");
# Get the latest version file from the repository
$cmd = "ssh uv\@userver_x86 \"cd harmattan/export ; svn export $repo/build-files/ver.cfg\"";
system($cmd);

# Pull out the version from the file
open(QVARFILE, "ver.cfg") or die;
my $qver = <QVARFILE>;
close QVARFILE;

# Get the subversion checkin version
my $svnver = `ssh uv\@userver_x86 "svn log $repo --limit=1 | grep \"^r\""`;
# Parse out the version number from the output we pulled out
$svnver =~ m/^r(\d+)*/;
$svnver = $1;
# Create the version suffix
$qver = "$qver.$svnver";
my $basedir = "./qgvdial-$qver";

# Delete any previous checkout directories
system("rm -rf qgvdial*");
system("rm -rf qgvtp*");

$cmd = "ssh uv\@userver_x86 \"cd harmattan/export ; svn export $repo $basedir\"";
system($cmd);
system("cp $basedir/icons/qgv64.png $basedir/src/qgvdial.png");

# Append the version to the pro file
open(PRO_FILE, ">>$basedir/src/src.pro") || die "Cannot open pro file";
print PRO_FILE "VERSION=__QGVDIAL_VERSION__\n";
close PRO_FILE;

# Version replacement
$cmd = "perl $basedir/build-files/version.pl __QGVDIAL_VERSION__ $qver $basedir";
print "$cmd\n";
system($cmd);

# Copy the correct pro file
system("cp $basedir/build-files/pro.qgvdial $basedir/qgvdial.pro");

# For the moment, forcibly include mqlib
$cmd = "cd $basedir/src && touch mqlib-build";
print "$cmd\n";
system($cmd);

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd $basedir && $mad qmake && $mad dh_make --createorig --single -e yuvraaj\@gmail.com -c lgpl";
print "$cmd\n";
system($cmd);

# Add a post install file to add the executable bit after installation on the device
system("mv $basedir/build-files/postinst.harmattan-qgvdial $basedir/debian/postinst");
system("mv $basedir/build-files/prerm.harmattan-qgvdial $basedir/debian/prerm");
# Fix the control file
system("mv $basedir/build-files/control.harmattan-qgvdial $basedir/debian/control");
# Add the Aegis manifest file
system("mv $basedir/build-files/aegis.harmattan.qgvdial $basedir/debian/qgvdial.aegis");

# Fix the dbus service file name. The same files as maemo can be used
system("mv $basedir/build-files/qgvdial.Call.service.maemo $basedir/build-files/qgvdial.Call.service");
system("mv $basedir/build-files/qgvdial.Text.service.maemo $basedir/build-files/qgvdial.Text.service");
# Change the name of the desktop file so that it can be directly used in the compilation
system("mv $basedir/build-files/qgvdial.desktop.harmattan $basedir/build-files/qgvdial.desktop");

# Fix the changelog and put it into the correct location
system("head -1 $basedir/debian/changelog >dest.txt && cat $basedir/build-files/changelog.qgvdial >>dest.txt && tail -2 $basedir/debian/changelog | sed 's/unknown/Yuvraaj Kelkar/g' >>dest.txt && mv dest.txt $basedir/debian/changelog");

# Make sure all make files are present before mucking with them.
system("cd $basedir && make src/Makefile");

# Strip out "/targets/FREMANTLE_ARMEL/" or "$qtsdk/Maemo/4.6.2/sysroots/fremantle-arm-sysroot-20.2010.36-2-slim"
$cmd="sed \"s/$pathreplace//ig\" $basedir/Makefile >$basedir/Makefile1 && mv $basedir/Makefile1 $basedir/Makefile && sed \"s/$pathreplace//ig\" $basedir/src/Makefile >$basedir/src/Makefile1 && mv $basedir/src/Makefile1 $basedir/src/Makefile";
print "$cmd\n";
system($cmd);

# Replace hard coded current directory with relative directory.
$cmd="sed 's/$curdir\\/qgvdial-$qver/../g' $basedir/Makefile >$basedir/Makefile1 ; mv $basedir/Makefile1 $basedir/Makefile ; sed 's/$curdir\\/qgvdial-$qver/../g' $basedir/src/Makefile >$basedir/src/Makefile1 ; mv $basedir/src/Makefile1 $basedir/src/Makefile";
print "$cmd\n";
system($cmd);

# Remove the GLESv2 dependency
$cmd="sed 's/ -lGLESv2//ig' $basedir/Makefile >$basedir/Makefile1 ; mv $basedir/Makefile1 $basedir/Makefile ; sed 's/ -lGLESv2//ig' $basedir/src/Makefile >$basedir/src/Makefile1 ; mv $basedir/src/Makefile1 $basedir/src/Makefile";
print "$cmd\n";
system($cmd);

if ($machine eq "arm") {
    # Reverse the order of these two lines for a source build 
    $cmd = "cd $basedir && $mad dpkg-buildpackage -rfakeroot -sa -S -uc -us";
    $cmd = "cd $basedir && $mad dpkg-buildpackage -rfakeroot -nc";
} else {
    # Reverse the order of these two lines for a source build 
    $cmd = "cd $basedir && $mad dpkg-buildpackage -sa -S -uc -us";
    $cmd = "cd $basedir && $mad dpkg-buildpackage -nc";
}
# Execute the rest of the build command
system($cmd);

exit(0);

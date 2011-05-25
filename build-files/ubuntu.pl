my $machine = `uname -m`;
my $mad = '';
my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $cmd;
my $line;
my $qtdir = $ENV{'QTDIR'};

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
system("cp $basedir/icons/qgv.png $basedir/src/qgvdial.png");

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

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd $basedir ; $mad qmake && echo y | $mad dh_make --createorig --single -e yuvraaj\@gmail.com -c lgpl && $mad qmake";
system($cmd);

# Add a post install file to add the executable bit after installation on the device
system("mv $basedir/build-files/postinst.linux $basedir/debian/postinst");
system("mv $basedir/build-files/prerm.linux $basedir/debian/prerm");
# Fix the control file
system("mv $basedir/build-files/control.linux $basedir/debian/control");
# Fix the dbus service file name
system("mv $basedir/build-files/qgvdial.Call.service.linux $basedir/build-files/qgvdial.Call.service");
system("mv $basedir/build-files/qgvdial.Text.service.linux $basedir/build-files/qgvdial.Text.service");
system("mv $basedir/qgv-tp/data/org.freedesktop.Telepathy.ConnectionManager.qgvtp.service.linux $basedir/qgv-tp/data/org.freedesktop.Telepathy.ConnectionManager.qgvtp.service");

system("head -1 $basedir/debian/changelog >dest.txt ; cat $basedir/build-files/changelog >>dest.txt ; tail -2 $basedir/debian/changelog | head -1 | sed 's/unknown/Yuvraaj Kelkar/g' >>dest.txt ; mv dest.txt $basedir/debian/changelog");

my $qt_target = "$basedir/debian/qgvdial/usr/share/qgvdial/qt-4.7.1";
# Copy the relevent Qt shared libraries
system("mkdir -p $qt_target");
system("cp $qtdir/lib/libQtMultimediaKit.so.1 $qt_target");
system("cp $qtdir/lib/libQtDeclarative.so.4 $qt_target");
system("cp $qtdir/lib/libQtSvg.so.4 $qt_target");
system("cp $qtdir/lib/libQtWebKit.so.4 $qt_target");
system("cp $qtdir/lib/libQtDBus.so.4 $qt_target");
system("cp $qtdir/lib/libQtScript.so.4 $qt_target");
system("cp $qtdir/lib/libQtSql.so.4 $qt_target");
system("cp $qtdir/lib/libQtXmlPatterns.so.4 $qt_target");
system("cp $qtdir/lib/libQtXml.so.4 $qt_target");
system("cp $qtdir/lib/libQtOpenGL.so.4 $qt_target");
system("cp $qtdir/lib/libQtGui.so.4 $qt_target");
system("cp $qtdir/lib/libQtNetwork.so.4 $qt_target");
system("cp $qtdir/lib/libQtCore.so.4 $qt_target");
system("cp $qtdir/lib/libphonon.so.4 $qt_target");

# Execute the rest of the build command
$cmd = "cd $basedir && $mad dpkg-buildpackage && $mad remote -r org.maemo.qgvdial send ../qgvdial_$qver-1_$machine.deb && $mad remote -r org.maemo.qgvdial install qgvdial_$qver-1_$machine.deb";
system($cmd);

exit();

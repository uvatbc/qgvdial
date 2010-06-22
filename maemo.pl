my $mad = '/home/uv/apps/mad';
my $repo = "https://qgvdial.googlecode.com/svn/trunk";
my $cmd;

# Delete any existing version file
if (-f ver.cfg)
{
    unlink(ver.cfg);
}
# Get the latest version file from the repository
$cmd = "svn export $repo/ver.cfg";
system($cmd);

# Pull out the version from the file
open(QVARFILE, "ver.cfg") or die;
my $qver = <QVARFILE>;
close QVARFILE;

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

system("rm -rf qgvdial*");
$cmd = "svn export $repo qgvdial-$qver";
system($cmd);
system("cp qgvdial-$qver/icons/Google.png qgvdial-$qver/src/qgvdial.png");

# Append the maemo.pro.suffix to the src.pro
open(PRO, ">>qgvdial-$qver/src/src.pro") or die "Cannot open source pro file";
open(PRO_SUFFIX, "<qgvdial-$qver/src/maemo.pro.suffix") or die "Cannot suffix file";
my $holdTerminator = $/;
undef $/;
my $buf = <PRO_SUFFIX>;
$/ = $holdTerminator;
print(PRO "$buf");
close(PRO_SUFFIX);
close(PRO);

$cmd = "cd qgvdial-$qver ; perl ../version.pl __QGVDIAL_VERSION__ $qver";
print "$cmd\n";
system($cmd);

# Do everything upto the preparation of the debian directory. Code is still not compiled.
$cmd = "cd qgvdial-$qver ; $mad qmake && $mad dh_make --createorig --single -e yuvraaj\@gmail.com -c lgpl && $mad qmake";
system($cmd);

# Add a post install file to add the executable bit after installation on the device
open(POSTINST, ">qgvdial-$qver/debian/postinst") or die "Cannot create postinst";
print(POSTINST "#!/bin/bash\n");
print(POSTINST "chmod +x /opt/qgvdial/bin/qgvdial\n");
close(POSTINST);
system("chmod +x qgvdial-$qver/debian/postinst");

# Execute the rest of the build command
$cmd = "cd qgvdial-$qver && $mad dpkg-buildpackage && $mad remote -r org.maemo.qgvdial send ../qgvdial_$qver-1_armel.deb && $mad remote -r org.maemo.qgvdial install qgvdial_$qver-1_armel.deb";
system($cmd);

exit();


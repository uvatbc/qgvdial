#!/usr/bin/perl
use File::Copy;

sub fix_one_file {
	my $entry = shift;
	my $dir_name = shift;
    
    # Remove trailing slash if present
    $dir_name =~ s/\/$//;
    
    my $filename = "$dir_name/$entry";
    print "Cleaning $filename\n";
    
    open(QMLFILE, $filename) or die;
    open(TEMPFILE, ">>$filename.tmp") or die;
    while (<QMLFILE>) {
        chomp; chomp;
        $line = $_;
        
        # Remove all leading spaces.
        $line =~ s/^\s*//;
        
        #Remove all comments
        if ($line =~ m/(.*)\/\//) {
            $line = $1;
        }
        
        # If by now the line is NOT empty, use it.
        if (length $line != 0) {
            print TEMPFILE "$line\n";
        }
    }
    close TEMPFILE;
    close QMLFILE;
    
    move("$filename", "$filename.bak");
    move("$filename.tmp", "$filename");
}

######## main ########
my $start_dir = $ARGV[0];

my $error_str = "Cannot open directory : \"$start_dir\"\n";
if	(not defined $start_dir)
{
	$start_dir = '.';
	$error_str = "Cannot open current directory\n";
}

opendir ($dir, $start_dir) || die $error_str;

$entry = readdir($dir);
while (defined $entry) {
    # Ignore . and ..
    if ( ($entry eq '.') || ($entry eq '..') ) {
        $entry = readdir($dir);
        next;
    }
    
    # Ignore everything other than qml and js
    if (($entry =~ m/.qml$/i) || ($entry =~ m/.js$/i)) {
        fix_one_file ($entry, $start_dir);
    }

    $entry = readdir($dir);
}

closedir $dir;

#!/usr/bin/perl

use Cwd 'abs_path';
my $path1 = abs_path();
my $path2 = abs_path("../..");

my $cmd = "sed 's $path1/$path2 $path2 g' Makefile > m2.txt ; mv m2.txt Makefile";
print("$cmd\n");
system($cmd);

# The last bullshit doesn't work: so fuck it out of the Makefile
$cmd = "sed 's|Copying application data.*\$|Copying application data|g' Makefile > m2.txt ; mv m2.txt Makefile";
print("$cmd\n");
system($cmd);

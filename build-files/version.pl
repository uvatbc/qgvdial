if (!defined $ARGV[0]) {
    die "Must provide source subst";
}

if (!defined $ARGV[1]) {
    die "Must provide destination subst";
}

my $srcsubst = $ARGV[0];
my $qver = $ARGV[1];
print("$srcsubst = $qver\n");

if (list_dir(".") == 0)
{
    print $error_str;
}

exit();

sub list_dir
{
    my $dir_name = shift;
    my $dir;
    opendir ($dir, $dir_name) || return 0;

    dir_init();

    my $entry;
    $entry = readdir($dir);
    while (defined $entry)
    {
        if ( ($entry eq '.') || ($entry eq '..') )
        {
            $entry = readdir($dir);
            next;
        }

        entry_callback ($entry, $dir_name);

        $entry = readdir($dir);
    }

    dir_exit();

    closedir $dir;

    return    1;
}

my $tab_count = 0;
sub dir_init
{
    $tab_count++;
}

sub dir_exit
{
    $tab_count--;
}

sub entry_callback
{
    my $entry = shift;
    my $dir_name = shift;

    #####
    # Here is where you would do something to the entry.
    #####
    action_on_entry($entry, $dir_name);

    #####
    # Recurse if possible
    #####
    my $dir = "$dir_name/$entry";
    if (-d $dir)
    {
        list_dir($dir);
    }
}

sub action_on_entry
{
    my $entry = shift;
    my $dir_name = shift;

    my $file = "$dir_name/$entry";
    if (-f $file)
    {
        $_ = $file;
        if ( (m/.cpp$/i) || (m/.c$/i) || (m/.h$/i) || (m/.rc$/i) || (m/.iss$/i) || (m/.txt$/i) || (m/.desktop$/i) || (m/.qml$/i) || (m/.wxs$/i) || (m/.pro$/i))
        {
            print "Working on $file\n";
            my $subst = "s/$srcsubst/$qver/g";
            system("perl -pi.bak -l -e $subst $file");
        }
    }
}


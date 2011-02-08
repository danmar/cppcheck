#!/usr/bin/perl
# warn if there are tabs in string constants, it can have surprising effects.
# example usage:
#     scripts/tabs.pl lib/checkstl.cpp

sub checkfile
{
    my $filename = $_[0];

    # parse file
    open(FILE, $filename);
    my @lines = <FILE>;
    close(FILE);

    # check comments..
    my $linenr = 0;
    foreach $line (@lines)
    {
        $linenr = $linenr + 1;

        # is there a tab in a string
        if ($line =~ /".*\t.*"/)
        {
                print "[$filename:$linenr] tab inside string constant\n";
        }
    }
}


foreach $filename (@ARGV)
{
    checkfile($filename)
}



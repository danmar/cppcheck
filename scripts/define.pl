#!/usr/bin/perl
# warn if there are #define in the code. it is often preferred with "const"
# usage:
#     scripts/define.pl lib/checkstl.cpp

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

        # is there a define?
        if ($line =~ /^#define\s+[A-Za-z0-9_]+\s+[^\s]/)
        {
            print "[$filename:$linenr] found #define\n";
        }
    }
}


foreach $filename (@ARGV)
{
    checkfile($filename)
}



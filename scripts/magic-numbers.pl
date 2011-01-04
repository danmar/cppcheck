#!/usr/bin/perl
# warn if there are magic numbers in the code.
# usage:
#     scripts/magic-numbers.pl lib/checkstl.cpp

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

        # is there a magic number?
        if (($line =~ /[^a-zA-Z_][0-9]{3,}/) && 
            (!($line =~ /define|const|(\/\/)/)))
        {
                print "[$filename:$linenr] Magic number\n";
        }
    }
}


foreach $filename (@ARGV)
{
    checkfile($filename)
}



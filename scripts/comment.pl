#!/usr/bin/perl
# Missing comments (it is a good idea to write a comment before a variable declaration)
# usage:
#     scripts/comment.pl lib/checkstl.cpp
sub checkfile
{
    my $filename = $_[0];

    # parse file
    open(FILE, $filename);
    my @lines = <FILE>;
    close(FILE);

    # check comments..
    my $comment = false;
    my $linenr = 0;
    foreach $line (@lines)
    {
        $linenr = $linenr + 1;

        # missing comment before variable declaration?
        if (($comment == 0) && 
            ($line =~ /^\s+([a-z]+)? [a-z]+(\s)+[a-z][a-z0-9]*\s*[;=]/) && 
            (!($line =~ /return|delete|operator/)))
        {
                print "[$filename:$linenr] No comment before variable declaration\n";
        }

        # set comment variable
        if (($line =~ /\/\//) || ($line =~ /\/\*/))
        {
            $comment = 1;
        }
        else
        {
            $comment = 0;
        }
    }
}


foreach $filename (@ARGV)
{
    checkfile($filename)
}



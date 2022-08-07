#!/usr/bin/env perl
use strict;
use warnings;
use CGI;
my $cgi = CGI->new();
print $cgi->header(
    -type            => 'text/html',
    -cost            => 'Three smackers',
    -annoyance_level => 'high',
    -complaints_to   => 'bit bucket',
);

print $cgi->header(
    -type       => 'image/gif',
    -nph        => 1,
    -status     => '402 Payment required',
    -expires    => '+3d',
    -charset    => 'utf-8',
    -attachment => 'foo.gif',
    -Cost       => '$2.00'
);

print <<'END';
<!doctype html>
<html> HTML Goes Here </html>
END

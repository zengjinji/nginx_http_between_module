#!/usr/bin/perl

# (C) Zengjinji

# Tests for between.

###############################################################################

use warnings;
use strict;

use Test::More;

BEGIN { use FindBin; chdir($FindBin::Bin); }

use lib 'lib';
use Test::Nginx;

###############################################################################

select STDERR; $| = 1;
select STDOUT; $| = 1;

my $t = Test::Nginx->new()->has(qw/http between/)->plan(10);

my $conf = <<'EOF';

%%TEST_GLOBALS%%

daemon off;

events {
}

http {
    %%TEST_GLOBALS_HTTP%%

    server {
        listen       127.0.0.1:8080;
        server_name  localhost;

        location /time0 {
            return 200 between_time:$is_between_time;
        }

        location /time1 {
            between_time  %%between_time1%%;
            return 200 between_time:$is_between_time;
        }

        location /time2 {
            between_time  %%between_time2%%;
            return 200 between_time:$is_between_time;
        }

        location /time3 {
            between_time  %%between_time3%%;
            return 200 between_time:$is_between_time;
        }

        location /time4 {
            between_time  %%between_time4%%;
            return 200 between_time:$is_between_time;
        }

        location /time5 {
            between_time  %%between_time2%%  %%between_time3%%;
            return 200 between_time:$is_between_time;
        }

        location /time6 {
            between_time  %%between_time2%%  %%between_time4%%;
            return 200 between_time:$is_between_time;
        }

        location /percent0 {
            return 200 between_percent:$is_between_percent;
        }

        location /percent1 {
            between_percent 100;
            return 200 between_percent:$is_between_percent;
        }

        location /percent2 {
            between_percent 0;
            return 200 between_percent:$is_between_percent;
        }
    }
}

EOF

my ($ori_ss, $ori_mm, $ori_hh) = localtime();
my ($begin_hh, $begin_mm, $end_hh, $end_mm);

my $hh = int($ori_hh);
my $mm = int($ori_mm);

if ($hh == 0) {
    $begin_hh = 0;
    $begin_mm = 0;
    $end_hh = $hh + 1;
    $end_mm = 0;

} elsif ($hh == 23) {
    $begin_hh = $hh - 1;
    $begin_mm = 0;
    $end_hh = $hh;
    $end_mm = 59;

} else {
    $begin_hh = $hh - 1;
    $begin_mm = 0;
    $end_hh = $hh + 1;
    $end_mm = 0;
}
my $between_time1 = get_time_interval($begin_hh, $begin_mm, $end_hh, $end_mm);


if ($hh < 2) {
    $begin_hh = 22;
    $begin_mm = 0;
    $end_hh = 23;
    $end_mm = 0;
} else {
    $begin_hh = $hh-2;
    $begin_mm = 0;
    $end_hh = $hh-1;
    $end_mm = 0;
}
my $between_time2 = get_time_interval($begin_hh, $begin_mm, $end_hh, $end_mm);

$begin_hh = $hh;
$begin_mm = $mm;
$end_hh = $hh;
$end_mm = $mm;
if ($mm == 59) {
    if ($hh == 23) {
        $end_hh = 23;
        $end_mm = 59;
    } else {
        $end_hh = $hh + 1;
        $end_mm = 0;
    }  
} else {
    $end_mm = $mm + 1;
}
my $between_time3 = get_time_interval($begin_hh, $begin_mm, $end_hh, $end_mm);

$begin_hh = $hh;
$end_hh = $hh;
if ($mm > 2) {
    $begin_mm = $mm - 2;
    $end_mm = $mm - 1;
} else {
    $begin_mm = 58;
    $end_mm = 59;
}

my $between_time4 = get_time_interval($begin_hh, $begin_mm, $end_hh, $end_mm);

$conf =~ s/%%between_time1%%/$between_time1/gmse;
$conf =~ s/%%between_time2%%/$between_time2/gmse;
$conf =~ s/%%between_time3%%/$between_time3/gmse;
$conf =~ s/%%between_time4%%/$between_time4/gmse;

$t->write_file_expand('nginx.conf', $conf);

$t->run();

################################################################################

like(http_get('/time0'), qr/between_time:1/, 'between_time is 1, default');
like(http_get('/time1'), qr/between_time:1/, 'between hour');
like(http_get('/time2'), qr/between_time:0/, 'no between hour');
like(http_get('/time3'), qr/between_time:1/, 'between minute');
like(http_get('/time4'), qr/between_time:0/, 'no between minute');
like(http_get('/time5'), qr/between_time:1/, 'between time array');
like(http_get('/time6'), qr/between_time:0/, 'no between time array');

like(http_get('/percent0'), qr/between_percent:1/, 'between_percent is 1, default');
like(http_get('/percent1'), qr/between_percent:1/, 'between percent');
like(http_get('/percent2'), qr/between_percent:0/, 'no between percent');


################################################################################

sub get_time_interval {
    my ($begin_hh, $begin_mm, $end_hh, $end_mm) = @_;

    $begin_hh = "$begin_hh";
    $begin_mm = "$begin_mm";
    $end_hh = "$end_hh";
    $end_mm = "$end_mm";

    if (length($begin_hh) == 1) {
        $begin_hh = "0$begin_hh"
    }

    if (length($begin_mm) == 1) {
        $begin_mm = "0$begin_mm"
    }

    if (length($end_hh) == 1) {
        $end_hh = "0$end_hh"
    }

    if (length($end_mm) == 1) {
        $end_mm = "0$end_mm"
    }

    return  "$begin_hh:$begin_mm-$end_hh:$end_mm";
}

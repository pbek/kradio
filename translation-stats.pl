#!/usr/bin/perl -w

use strict;
use Data::Dumper;

# Status: 0 == excelent (no translations required)
#         1 == good (some translations required)
#         2 == not so good (quite some translations required)
#         3 == bad (many translations required)
my $STATE_EXCELENT  = 0;
my $STATE_GOOD      = 1;
my $STATE_NOTSOGOOD = 2;
my $STATE_BAD       = 3;


open ALLPO, "find -iname \"*.po\" |" or die "cannot run find -iname \"*.po\"\n: $!";
my @allpo = <ALLPO>;
close ALLPO;

chomp @allpo;

my %langs = ();

foreach my $po(@allpo) {
    my $lang = $po;
    $lang =~ s:^.*/::;
    $lang =~ s:\.po$::;
    my %res = analyze_po($po);
    $langs{$lang} = { %{ $langs{$lang} || {} }, %res };
}


my %stats = ();

foreach my $lang(sort keys %langs) {
    my %records        = %{$langs{$lang}};
    my $total          = 0 + keys %records;
    my $fuzzy          = 0 + grep { exists $$_{fuzzy}   && $$_{fuzzy}        } values %records;
    my $not_translated = 0 + grep { !exists $$_{msgstr} || $$_{msgstr} eq "" } values %records;
    my $translated     = $total - $fuzzy - $not_translated;
    $stats{$lang} = {  total           => $total,
                       fuzzy           => $fuzzy,
                       translated      => $translated,
                       not_translated  => $not_translated,
                       tobe_translated => $fuzzy + $not_translated,
                    };
}

# standard text output

foreach my $lang(sort keys %stats) {
    my $s = $stats{$lang};
    printf "%-5s:   total:%5i     translated: %5i (%3i%%)    fuzzy: %5i (%3i%%)     untranslated: %5i (%3i%%)\n",
           $lang,
           $$s{total},
           $$s{translated},     $$s{translated}     * 100 / $$s{total},
           $$s{fuzzy},          $$s{fuzzy}          * 100 / $$s{total},
           $$s{not_translated}, $$s{not_translated} * 100 / $$s{total};
}

# colorful html output

foreach my $lang(sort keys %stats) {
    my $s = $stats{$lang};
    print "<tr>";
    print colored_html_td($lang,                                     get_lang_color($s));
    print colored_html_td($$s{total},                                "#000000");
    print colored_html_td(get_percent_string($s, "translated"),     get_lang_color($s));
    print colored_html_td(get_percent_string($s, "fuzzy"),          get_color(get_status($s, 'fuzzy')));
    print colored_html_td(get_percent_string($s, "not_translated"), get_color(get_status($s, 'not_translated')));
    print "</tr>\n";
}




sub get_percent_string
{
    my $lang = shift;
    my $key  = shift;
    return "$$lang{$key} (".int($$lang{$key} * 100 / $$lang{total})."%)";
}

sub colored_html_td
{
    my $text  = shift;
    my $color = shift;
    return "<td ALIGN=center><FONT COLOR=\"$color\">$text</FONT></TD>";
}

sub get_lang_color
{
    my $lang   = shift;
    my $status = get_lang_status($lang);
    my $color  = get_color($status);
}


sub get_color
{
    my $status = shift;
    if    ($status == $STATE_EXCELENT)  { return "#00C000"; }
    elsif ($status == $STATE_GOOD)      { return "#0000C0"; }
    elsif ($status == $STATE_NOTSOGOOD) { return "#FFA000"; }
    elsif ($status == $STATE_BAD)       { return "#FF0000"; }
    else                                { return "#FF0000"; }
}

# Status: 0 == excelent (no translations required)
#         1 == good (some translations required)
#         2 == not so good (quite some translations required)
#         3 == bad (many translations required)
sub get_lang_status
{
    my $lang = shift;
    return get_status($lang, 'tobe_translated');
}


# Status: 0 == excelent (no translations required)
#         1 == good (some translations required)
#         2 == not so good (quite some translations required)
#         3 == bad (many translations required)
sub get_status
{
    my $lang = shift;
    my $key  = shift;

    my $percent = $$lang{$key} * 100 / $$lang{total};
    if ($percent == 0) {
        return $STATE_EXCELENT;
    }
    elsif ($percent <= 8) {
        return $STATE_GOOD;
    }
    elsif ($percent <= 25) {
        return $STATE_NOTSOGOOD;
    }
    else {
        return $STATE_BAD;
    }
}


# my %de = keys %{$langs{de}};
# my %es = keys %{$langs{es}};
#
# print map "es: $_\n", sort keys %es;
# print map "de: $_\n", sort keys %de;
#
# my @diffs = grep { !exists $es{$_} } sort keys %de;
# print map "diff: $_\n", @diffs;
#
# print Dumper(\%langs);






sub analyze_po
{
    my $po = shift;
    open PO, $po or die "cannot read file $po: $!\n";
    my @lines = <PO>;
    close PO;
    chomp @lines;

    @lines = merge_multi_lines(@lines);

    my %translations = get_translations($po, @lines);
}




sub get_translations
{
    my $filename = shift;
    my @lines    = @_;
    my $dirname  = $filename;
    $dirname     =~ s:/[^/]+$::;
    my %res      = ();

    my $record   = {};

#    if ($filename =~ m:convert-presets/po/es.po$: ) {
#        print map ("debug: $filename: $_\n", @lines);
#    }

    while (@lines) {
        my $line = shift @lines;

        # file name and line number
        if ($line =~ /^\#:\s+/) {
            $$record{location} = "$'";
            $$record{key}      = "$dirname -- $$record{location}";
        }
        # file name and line number
        elsif ($line =~ /^\#\.\si18n:\s+file:\s+/) {
            $$record{uilocation} = $';
        }
        elsif ($line =~ /^\#,\s+.*\bfuzzy\b/) {
            $$record{fuzzy} = 1;
        }
        elsif ($line =~ /^msgid\s+\"(.+)\"\s*$/) {
            $$record{msgid} = $1;
        }
        elsif ($line =~ /^msgstr\s+\"(.*)\"\s*$/) {
            $$record{msgstr} = $1;
        }
        elsif ($line =~ /^\s*$/ && exists $$record{msgid}) {
            $res{$$record{key}} = $record;
            $record = {};
        }
        else {
            #ignore
        }
    }
    if (exists $$record{msgid}) {
        $res{$$record{key}} = $record;
        $record = {};
    }
#    if ($filename =~ m:convert-presets/po/es.po$: ) {
#        print Dumper(\%res);
#    }
    return %res;
}


sub merge_multi_lines
{
    my @input  = @_;
    my @output = ();
    foreach my $line(@input) {
        # if line starts with quoted string, continue previous line
        if ($line =~ /^\"/ && @output && $output[$#output] =~ /\"\s*$/) {
            $output[$#output] =~ s/\"\s*$//;
            $line             =~ s/^\"//;
            $output[$#output] .= $line;
        }
        # else just add the line to the list of output lines
        else {
            push @output, $line;
        }
    }
    return @output;
}


# for LANG in $(find -iname "*.po" | while read line; do basename $line .po ; done | sort -u) ; do
#     TOTAL=$(         find -iname "$LANG.po" | xargs -r cat | egrep "^msgid \".+\"$"                               | wc -l)
#     TRANSLATED=$(    find -iname "$LANG.po" | xargs -r cat | egrep "^msgid \".+\"$" -A1 | egrep "^msgstr \".+\"$" | wc -l)
#     NOT_TRANSLATED=$(find -iname "$LANG.po" | xargs -r cat | egrep "^msgid \".+\"$" -A1 | egrep "^msgstr \"\"$"   | wc -l)
#     FUZZY=$(         find -iname "$LANG.po" | xargs -r cat | egrep "^#, *fuzzy"                                   | wc -l)
#     echo "$LANG:   total:$TOTAL     fuzzy: $FUZZY      untranslated: $NOT_TRANSLATED"
# done

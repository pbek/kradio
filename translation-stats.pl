#!/usr/bin/perl -w

use strict;
use Data::Dumper;
use Locale::PO;

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
    $langs{$lang}{$po} = \%res;
}


my %stats = ();

while (my ($lang, $pofiles) = each(%langs)) {
    my $total          = 0;
    my $fuzzy          = 0;
    my $not_translated = 0;
    while (my ($pofile, $postats) = each(%$pofiles)) {
        my %po = %$postats;
        $total += $po{total};
        $fuzzy += $po{fuzzy};
        $not_translated += $po{untranslated};
    }
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
    my $aref = Locale::PO->load_file_asarray($po) or die "cannot read file $po: $!\n";
    my %res = ();
    foreach my $entry(@{$aref}) {
        my $msgid = $entry->msgid();
        if (defined($entry->obsolete()) or $msgid eq '""') {
            # skip obsolete messages or the header
        } else {
            $res{total}++;
            if (defined($entry->msgstr_n())) {
                my $count = 0;
                foreach my $str(%{$entry->msgstr_n()}){
                    $count++ if $str eq '""';
                }
                $res{untranslated}++ if $count eq scalar($entry->msgstr_n());
            } else {
                $res{untranslated}++ if $entry->msgstr() eq '""';
            }
            if ($entry->fuzzy()) {
                $res{fuzzy}++;
            }
        }
    }
    if (not defined($res{total})) {
        $res{total} = 0;
    }
    if (not defined($res{fuzzy})) {
        $res{fuzzy} = 0;
    }
    if (not defined($res{untranslated})) {
        $res{untranslated} = 0;
    }
    return %res;
}



# for LANG in $(find -iname "*.po" | while read line; do basename $line .po ; done | sort -u) ; do
#     TOTAL=$(         find -iname "$LANG.po" | xargs -r cat | egrep "^msgid \".+\"$"                               | wc -l)
#     TRANSLATED=$(    find -iname "$LANG.po" | xargs -r cat | egrep "^msgid \".+\"$" -A1 | egrep "^msgstr \".+\"$" | wc -l)
#     NOT_TRANSLATED=$(find -iname "$LANG.po" | xargs -r cat | egrep "^msgid \".+\"$" -A1 | egrep "^msgstr \"\"$"   | wc -l)
#     FUZZY=$(         find -iname "$LANG.po" | xargs -r cat | egrep "^#, *fuzzy"                                   | wc -l)
#     echo "$LANG:   total:$TOTAL     fuzzy: $FUZZY      untranslated: $NOT_TRANSLATED"
# done

#!/usr/bin/perl -w

use strict;
use Data::Dumper;

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


foreach my $lang(sort keys %langs) {
    my %records        = %{$langs{$lang}};
    my $total          = 0 + keys %records;
    my $fuzzy          = 0 + grep { exists $$_{fuzzy}   && $$_{fuzzy}        } values %records;
    my $not_translated = 0 + grep { !exists $$_{msgstr} || $$_{msgstr} eq "" } values %records;
    printf "%-5s:   total:%5i     fuzzy: %5i     untranslated: %5i\n", $lang, $total, $fuzzy, $not_translated;
}


#print Dumper(\%langs);






sub analyze_po
{
    my $po = shift;
    open PO, $po or die "cannot read file $po: $!\n";
    my @lines = <PO>;
    close PO;
    chomp @lines;

    @lines = merge_multi_lines(@lines);

    my %translations = get_translations(@lines);
}




sub get_translations
{
    my @lines  = @_;
    my %res    = ();

    my $record = {};

    while (@lines) {
        my $line = shift @lines;

        # file name and line number
        if ($line =~ /^\#:\s+/) {
            $$record{location} = $';
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
            $res{$$record{location}} = $record;
            $record = {};
        }
        else {
            #ignore
        }
    }
    if (exists $$record{msgid}) {
        $res{$$record{location}} = $record;
        $record = {};
    }
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

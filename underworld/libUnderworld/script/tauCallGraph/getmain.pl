#!/usr/bin/perl
use Term::ANSIColor;
use Data::Dumper;
sub getYnotinX{#returns Y-(Y && X)
    my ( $Y, $X) = @_;
    my %original = ();
    my @subY  = ();
    map { $original{$_} = 'x' } @$X;
    @subY  = grep { !defined  $original{$_} } @$Y; ## i.e. $_ takes values from the @Y array and sees if they are in the original hash table
    return @subY ; # returns a subset of @Y
}
# this is now dependent on the callgraph depth from tau: requires depth of 5 to work properly
sub getInitialListfromFirstWord{
    my ( $testword, $linesarray) = @_;
    my $i;
    my @single = ();
    my @lines_ = @$linesarray;
    for $i (0..$#lines_) {
	if($lines_[$i] =~ /^($testword => \w+)\b/){
	    push(@single, $lines_[$i]);
	}
	if($lines_[$i] =~ /^\w+ => ($testword => \w+ => \w+ => \w+)\b/){
	    push(@single, $1);
	}
	if($lines_[$i] =~ /^\w+ => \w+ => ($testword => \w+ => \w+)\b/){
	    push(@single, $1);
	}
	if($lines_[$i] =~ /^\w+ => \w+ => \w+ => ($testword => \w+)\b/){
	    push(@single, $1);
	}
    }
    return @single;
}
sub getlistfromFirstWord{
    my ( $testword, $linesarray) = @_;
    my $i;
    my @single = ();
    my @lines_ = @$linesarray;
    for $i (0..$#lines_) {
	if($lines_[$i] =~ /^($testword => \w+)\b/){
	    push(@single, $lines_[$i]);
	}
    }
    return @single;
}
sub getlistfromSecondWord{
    my ( $testword, $linesarray) = @_;
    my $i;
    my @single = ();
    @lines = @$linesarray;
    for $i (0..$#lines) {
	if($lines[$i] =~ /^(\w+ => $testword)\b/){
	    push(@single, $lines[$i]);
	}
    }
    return @single;
}
sub getFirstPairs{
     my ( $arrayin) = @_;
     my @array = @$arrayin;
     my @arrayout = ();
     my $kk;
     my $line;
    
    for $kk (0..$#array){
 	$array[$kk] =~ /^(\w+ => \w+)\b/;
 	$line = $1;
 	$arrayout[$kk] = "$line\n";
    }
     return @arrayout;
}
sub getRemainder{
     my ( $arrayin) = @_;
     my @array = @$arrayin;
     my @arrayout = ();
     my $kk;
     my $line;
    
    for $kk (0..$#array){
 	$array[$kk] =~ /^(\w+ => )(\w+\b.*)/;
 	$line = $2;
 	$arrayout[$kk] = "$line\n";
    }
     return @arrayout;
}

###########################################################################
# gets each line of the form e.g. A => B => C
# then compares against every other line ahead of the current line
# and removes the current line if it is contained within another line
# e.g.
# if current line is A => B => C
# and another line is X => A => B => C => Y
# then the line A => B => C is removed
###########################################################################
sub removesubsets{
    my ( $array ) = @_;
    my @single = @$array;
    my $i,$j,$k;
    my $line;
    my $tmp;
    my $searching;
    # rename duplicates to xxxxx
    for $i (0..$#single){
	$line = $single[$i];
	chomp $line;chomp $line;
	$k=$i+1;
	$searching=1;
	$j=$i+1;
	while($searching == 1){
	    $tmp=$single[$j];
	    if ($tmp =~ /$line/){
		$single[$i] = 'xxxxx';
		$searching=0;
	    }
	    $j++;
	    if($j > $#single){ $searching=0; }
	}
    }
    # remove the xxxx's
    for ($i=$#single;$i >= 0;$i--){
	#print "i = $i\n";
	if($single[$i] =~ /xxxx/){
	    #print "deleting $i\n";
	    splice(@single,$i,1);
	}
    }
    return @single;
}
sub removesubsetswithcol{
    my ( $array ) = @_;
    my @single = @$array;
    my $i,$j,$k;
    my $line;
    my $tmp;
    my $searching;
    # rename duplicates to xxxxx
    for $i (0..$#single){
	$line = $single[$i];
	$line =~ s/ \[.*\]//;
#	$col = $1;
	chomp $line;chomp $line;
	$k=$i+1;
	$searching=1;
	$j=$i+1;
	while($searching == 1){
	    $tmp=$single[$j];
	    if ($tmp =~ /$line\b/){
		$single[$i] = 'xxxxx';
		$searching=0;
	    }
	    $j++;
	    if($j > $#single){ $searching=0; }
	}
    }
    # remove the xxxx's
    for ($i=$#single;$i >= 0;$i--){
	#print "i = $i\n";
	if($single[$i] =~ /xxxx/){
	    #print "deleting $i\n";
	    splice(@single,$i,1);
	}
    }
    return @single;
}
# assumes array is sorted
sub getsubset{
    ( $i, $k, $array ) = @_;
    my @single = @$array;
    my $j,$kk;
    my $done;
    my $testword;
    my @subset=();
    my $word;
    my $word1;
    my $word2;
    
    #get first word/node on line $i
    $done = 0;	
    $single[$i] =~ /^(\w+ => \w+)\b.*/;
    $testword = $1;
    $subset[0] = "$testword\n";
    #$single[$i] =~ /^(\w+ => )(\w+\b.*)/;
    #print "testword $testword <<>> $single[0] <<>> $subset[0]\n";
    #print @single;
    $j=1;
    $k=$i;
    while( $done == 0){
	# if($single[$k] =~ /IntegrationPointMapper_GetMaterialPointsSwarms/){
	#     print ">> $subset[$j] j=$j i=$i<< >>\n\n ((($testword)) - (($word2)) k =$k single = $#single\n";
	# }
	$single[$k] =~ /^(\w+ => \w+)\b.*/;
	$word2 = $1;
	if($testword eq $word2 && $k <= $#single){
	    $subset[$j] = $single[$k];

	    $j++;
	    $k++;
	}
	else{
	    $done = 1;
	    #for $kk (0..$j-1){
		#print "$#subset+1 -- $subset[$kk]";
	    #}
	    #print "---------------boundary\n";
	    #$#subset=0;
	}
    }
    #$k++;
    $i=$k;
    # we have a subset now
    return @subset;
}
sub cullfirstword{
    my ( $array ) = @_;
    my @subset=@$array;
    my $word;
    my $line;
    my $j;
    my $needtosort=0;
    my @tmp=();
    
    #$subset[0] =~ /(^\w+ => )\w+\b.*/;

    for $j (1 .. $#subset){
	$line = $subset[$j];
	if($line =~ /^\w+ => \w+ => \w+/){
	    $line =~ /^(\w+ => )(\w+) => .*/;
	    $word = $1;
	    $subset[$j] =~ s/$word//;
	}
	else{
	    # actually if I end up here then it means one of the terms in the list
	    # is actually A->B while first term is A->X
	    # but what gets passed in is already in the form
	    # A->B->X.. for every term
	    # so if here then A->B == A->X
	    $needtosort=1;
	    #print "In cullfirstword\n";
	    #print "subset[0] = $subset[0]\n";
	    #print "subset[$j] = $subset[$j]\n";
	    if($#subset == 1){
		pop(@subset);
	    } 
	}
    }
    if($needtosort == 1 && $#subset > 2){
	# 2 because if num elements is 3 then there 
	# are only 2 extra terms. So sorting would only change their order.
	# this is done to avoid situations like
	# A - B
	# B - C -
	# A - B 
	@tmp = @subset;
	shift(@tmp); # remove first element
	@tmp = sort(@tmp);
	unshift(@tmp,$subset[0]); # put first element back
	@subset = @tmp;
    }
    return @subset;
}
sub getgraph{
    ( $word, $single_, $lines) = @_;
    my $testword = $word;
    my @single = @$single_;# does this make a local copy now?
    my @subset = ();
    my @subsetmatch = ();
    my @subsetmatchcalled = ();
    my @firstpairs = ();
    my @remainders = ();
    my @tmp = ();
    my @allsets = ();
    my $secword;
    local $i=0;
    local $j=0;
    local $k=0;

    @allsets=();
    $i=0;
    #while($i < $#single){
    while($i <= $#single){# && $#single > 0){

	$#subset=-1;
	#print color 'blue';
	#print "ENTERING getsubset\n";
	#print color 'reset';
	@subset = &getsubset($i, $k, \@single);
	# if($#subset < 2){
	#     print "\n\nPrinting subsets: rlevel=$rlevel\n\n";
	#     print @subset;
	#     print "END subset\n\n";
	#     print "Printing single\n";
	#     print @single;
	#     print "END single\n";
	# }
	# print color 'yellow';
	# print "SUBSET\n";
	# print @subset;
	# print "END SUBSET\n";
	# print "k = $k i = $i size = $#subset\n";
	# print color 'reset';	
	@subset = &cullfirstword( \@subset );

	#print "k = $k i = $i size = $#subset\n";
	$i=$k;
	# want to get all lines from lines that start with second word in current subset?
	if($#subset > 0){
	    $subset[1] =~ /^(\w+) => \w+.*/;
	    $secword = $1;
	    # get lines that match $secword => X where $secword is first word in lines
	    @subsetmatch = &getlistfromFirstWord($secword, \@lines);
	    # Maybe here only add terms to the list (subset (1..$#subset)) that 'extend' the existing terms.
	    # i.e. if we already have A -> B -> C then only add terms like A -> B -> C -> D and then remove the orginal
	    $idx=0;
	    $subset_add = ();
	    $#subset_add = -1;
	    $Found = 0;
	    for($ii=1;$ii <= $#subset; $ii++){
		$term = $subset[$ii];
		chomp $term;chomp $term;

		for($jj=0;$jj <= $#subsetmatch; $jj++){
		    if($subsetmatch[$jj] =~ /$term/){# if $term is in subsetmatch
			$subset_add[$idx] = $subsetmatch[$jj];
			$idx++;			
		    }
		}
	    }
	    #@lines  = &getYnotinX(\@lines, \@subsetmatch);
	    #@subset=( @subset, @subset_add );
	    #@subset = sort(@subset);
	    #@subset = &removesubsets( \@subset );
	    @lines  = &getYnotinX(\@lines, \@subset_add);
	    # get lines that match X => $secword where X is first word
	    # X will be functions other than current function $testword that call the same
	    # $secword function
	    $#subsetmatchcalled = -1;	    # lets not do this for moment?
	    ##@subsetmatchcalled = &getlistfromSecondWord($secword, \@lines);
	    # if($#subsetmatchcalled > -1){
	    # 	@lines  = &getYnotinX(\@lines, \@subsetmatchcalled);
	    # 	@firstpairs = &getFirstPairs( \@subsetmatchcalled );
	    # 	@firstpairs = &removesubsets( \@firstpairs );
	    # 	@remainders = &getRemainder( \@subsetmatchcalled );
	    # 	@remainders = &removesubsets( \@remainders );
	    # }
	    # now join everything together and removesubsets
	    #@subset = (@subset, @subsetmatch, @firstpairs, @remainders);
	    @tmp = @subset;
	    shift(@tmp); # removing first element i.e. $testword => $secword is first element
	    # @tmp = ( @tmp, @subsetmatch, @remainders);
	    @tmp = ( @tmp, @subset_add );
	    @tmp = sort( @tmp );
	    @tmp = &removesubsets( \@tmp );
	    if ($Found == 1){
		#print @tmp;
		#print "END tmp\n";
	    }
	    # print "firstpairs is ----\n";
	    # print "subset[0] = $subset[0]\n";
	    # print @firstpairs;
	    # print "end firstpairs\n";
	    # print "tmp is -----------<><><><<\n";
	    # print @tmp;
	    # print "end tmp -----------<><><><<\n";
	    if($#tmp > -1){
		print color 'red';
		print "RECURSING into getgraph level=$rlevel\n";
		print color 'reset';
		#sleep 1;
		$rlevel++;
		@tmp = &getgraph($secword, \@tmp, \@lines);
		$rlevel--;
		my $Hue=rand();
		#my $G=rand();
		#my $B=rand();
		my $nodecol="[color=\"$Hue 1.0 1.0\"]";
		for $ii (0..$#tmp){
		    #$tmp[$ii] =~ s/ \[.*\]//;
		    if($tmp[$ii] !~ / \[.*\]/){
			#$tmp[$ii] =~ s/ \[.*\]//;
			chomp $tmp[$ii];
			$tmp[$ii] = "$tmp[$ii]"." $nodecol\n";
		    }
		}
		print  color 'blue';
		print "coming out of recurse level=$rlevel \n";
		print color 'reset';
	    }
	    else{
		print color 'green';
		print "NOT RECURSING\n";
		print color 'reset';
	    }
	    # @tmp has all the lines that are $secword => X
	    #unshift(@firstpairs, $subset[0]); # add to front of array
	    #@firstpairs = &removesubsets( \@firstpairs );
	    unshift(@tmp, $subset[0]); # add to front of array
	    @subset = @tmp;
	    #@subset = ( @firstpairs, @tmp );
	}
	@allsets = (@allsets, @subset);
	# what if I check for doubled up function calls as I go?
	#@allsets=sort(@allsets);
	#@allsets = &removesubsetswithcol( \@allsets );
	#print @tmp; print "tmp\n";
	if($#subset > -1){
	    #print @subsetmatch; print "subsetmatch\n";
	    #print @subsetmatchcalled; print "subsetmatchcalled\n";
	    #print @firstpairs; print "first pairs\n";
	    #print @remainders; print "remainders\n";
	}
	#$#subset=0;
    }
    return @allsets;
}
############################################################################
############################################################################
############################################################################
$filename="$ARGV[0]";
$function="$ARGV[1]";
open FILE, "$filename"  or die "can't find file: $!\n";
$i=0;
while (<FILE>){
    s/\(//g;
    s/\)//g;
    $lines[$i]=$_;
    $i++;
}
close FILE;


@lines=sort(@lines);
print "$#lines\n";

$testword="main";
$testword="StGermain_Init";
$testword="ModulesManager_Load";
$testword="stgMainBuildAndInitialise";
$testword="stgMainLoop";
#$testword="OperatorFeVariable_UnaryInterpolationFunc";

$testword="WeightsCalculator_CalculateAll";
$testword=$function;
#print "$#single\n";
@single = &getInitialListfromFirstWord($testword, \@lines);
print Dumper(@single);
@lines  = &getYnotinX(\@lines, \@single);
sort(@single);
print "$#single\n";print "$#lines\n";
@single = &removesubsets( \@single );

$rlevel=0;
my $R=rand();
my $G=rand();
my $B=rand();
#$nodecol="[color=\"$R $G $B\"]";
@allsets = &getgraph( $testword, \@single, \@lines);

#print @allsets;
print "lines $#lines\n";
print "allsets $#allsets\n";

#@allsets=sort(@allsets);
print "removing subsets from allsets: size is $#allsets\n";
@allsets = &removesubsetswithcol( \@allsets );
$dotfile="$function.dot";
open (MYFILE, ">$dotfile");
#332 332
print MYFILE "digraph $function {\noverlap=\"true\";\nsize=\"200,200\"\;\n";
print MYFILE "packMode=\"graph\"\;\n";
print MYFILE "node [color=lightblue2, style=filled]\;\n";
#print MYFILE "edge [penwidth=\"3\"]\;\n";
print MYFILE "ranksep=5\;\n";
for $i (0..$#allsets){
    chomp $allsets[$i];
    $allsets[$i] =~ s/^ =>.*//;
    $allsets[$i] =~ s/=>/->/g;
    $allsets[$i] =~ s/\(//g;
    $allsets[$i] =~ s/\)//g;
    
    #if($allsets[$i] =~ /->/g && $allsets[$i] !~ /Journal_Register/){
    if($allsets[$i] =~ /->/g){
	print MYFILE "$allsets[$i];\n";
    }
}
print MYFILE "}\n";
close (MYFILE)

#print @lines;
#usage: pprof -l > out
#       ./getmain.pl out function_name

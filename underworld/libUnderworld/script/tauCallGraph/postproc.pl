#!/usr/bin/perl
use Term::ANSIColor;
use Data::Dumper;

sub getYinX{#returns (Y && X)
    my ( $Y, $X) = @_;
    my %original = ();
    my @subY  = ();
    map { $original{$_} = 'x' } @$X;
    @subY  = grep { defined  $original{$_} } @$Y; ## i.e. $_ takes values from the @Y array and sees if they are in the original hash table
    return @subY ; # returns a subset of @Y
}
sub getYnotinX{#returns Y-(Y && X)
    my ( $Y, $X) = @_;
    my %original = ();
    my @subY  = ();
    map { $original{$_} = 'x' } @$X;
    @subY  = grep { !defined  $original{$_} } @$Y; ## i.e. $_ takes values from the @Y array and sees if they are in the original hash table
    return @subY ; # returns a subset of @Y
}
$reg = qr/(.*)\b(\w+)\b \[\{(\w+\.[ch])\} \{ ?(\d+)\,\d+\}\]/;
$reg2 = qr/(.*)\b(\w+)\b \[\{(\w+\.def)\} \{ ?(\d+)\,\d+\}\]/;
$home=$ENV{'HOME'};
$maindir="$home/uworld/stgUnderworldWarMad";
$maindir = "$ARGV[2]";
#print "dir = $maindir\n";
#sleep 5;
$filename="$ARGV[1]"; # the dot file
open FILE, "$filename"  or die "can't find file: $!\n";
$i=0;
while (<FILE>){
    if(/(\w+) -> (\w+)/){
	$dotlines[$i]=$1;
	$dotlines[$i+1]=$2;
	$i += 2;
    }
}
@dotlines=sort(@dotlines);
$prev = $dotlines[0];
# remove duplicates from sorted array
@dotlines = grep($_ ne $prev && (($prev) = $_), @dotlines);
close FILE;

$filename="$ARGV[0]"; # the pprof -a -p > outall
open FILE, "$filename"  or die "can't find file: $!\n";
$i=0;
$totaltime=0.0;
$maxinctime=0.0;
while (<FILE>){
    s/\(//g;
    s/\)//g;
    if(!/=>/ && (/$reg/||/$reg2/)){
	$lines[$i]=$_;
	$dd=$1;
	$dd =~ s/^\s+//; 
	$dd =~ s/\s+$//; 
	$dd =~ s/\s+/ /g;
	@tmp = split(/ /,$dd);
	$tmp[1] =~ s/,//g;
	$tmp[2] =~ s/,//g;
	$totaltime += $tmp[1];
	if($tmp[2] > $maxinctime){
	    $maxinctime = $tmp[2];
	}
	#print "$dd -- $tmp[1] i=$i\n";
	# $min=$tmp[1];
	# if($tmp[1] < 0.00001){
	#     $min=0.00001;
	# }
	$testtime[$i]= log($tmp[1]+1.0);
	$i++;
	#print $_;
    }
}
close FILE;
# numeric sort
@testtime=sort {$a <=> $b } @testtime;
$maxtime = $testtime[$i-1];
$mintime = $testtime[0];
$deltatime=$maxtime-$mintime;
print STDERR "max $maxtime min $mintime delta $deltatime\n";
print STDERR "maxinc $maxinctime totaltime $totaltime\n";
#@lines=sort(@lines);
#print @lines;
#print "size of lines $#lines\n";
#@fileList=`find -name "*.c"`;
for $i (0..$#lines){
    $tmp = $lines[$i];
    if($tmp =~ /$reg/){
	#print "$1 $2 $3\n";
	$ff[$i]=$2; $cc[$i]=$3; $ll[$i]=$4;
    }
    if($tmp =~ /$reg2/){
	#print "$1 $2 $3\n";
	$ff[$i]=$2; $cc[$i]=$3; $ll[$i]=$4;
    }
}

# now we want to pair up terms from lines using only those that appear in both
# my %orig =();
# foreach (@lines){
#     #$tmp=$_;
#     /.*\b(\w+)\b \[\{(\w+\.c)\} \{(\d+)\,\d+\}\]/;
#     $orig{$_} = $1;
#     #print "$1 -- $_\n";
# }
# print "$orig{$dotlines[3]} -- $dotlines[3]\n";
#$first = $dotlines[0];
#$i=0;
#@lines2 = grep( ($orig{$_}), @lines);
# $k=0;
# for $i (0..$#lines){
#     $tmp=$lines[$i];
#     #print "CC $tmp\n";
#     for $j (0..$#dotlines){
# 	if($tmp =~ /\b$dotlines[$j]\b/){
# 	    $new[$k]=$tmp;
# 	    $k++;
# 	    #print "$tmp\n";
# 	}
#     }
# }
$k=0; $m=0;
for $i (0..$#ff){
    $tmp=$ff[$i];
    #print "CC $tmp\n";
    for $j (0..$#dotlines){
	if($tmp =~ /\b$dotlines[$j]\b/){
	    $new[$k]="$lines[$i]";
	    $k++;
	    #print "$tmp\n";
	}
    }
    if($i%10 ==0){ print STDERR "*"; }
}
print STDERR "\n";
#print @dotlines;
#print @gg;
#print "size $new[0] $#new $#dotlines\n";
for $i (0..$#new){
    if($new[$i] =~ /$reg/){
	$new2[$i] = $2;
    }
    if($new[$i] =~ /$reg2/){
	$new2[$i] = $2;
    }
    #print "$dotlines[$i]\n";
}
@new2 = sort(@new2);
#print @new2;
#print "$new2[0] $dotlines[0]\n";
@tt =&getYnotinX(\@dotlines,\@new2);
#print Dumper(@new);
#print Dumper(@new2);
#main [{main.c} {144,0}]
for $i (0..$#new){
    $tmp = $new[$i];
    if($tmp =~ /$reg/){
	#print "$1 $2 $3\n";
	$data[$i]=$1; $f[$i]=$2; $c[$i]=$3; $l[$i]=$4;
    }
    if($tmp =~ /$reg2/){
	#print "$1 $2 $3\n";
	$data[$i]=$1; $f[$i]=$2; $c[$i]=$3; $l[$i]=$4;
    }
}
for $i (0..$#c){
    if($i%10 ==0){ print STDERR "-"; }
}
print STDERR "\n";
for $i (0..$#c){
    $flist[$i] = `find -L $maindir -name $c[$i]`; # -L means follow symbolic links
    if($i%10 ==0){ print STDERR "*"; }
    #print "$flist[$i]";
}
print STDERR "\n";
#print "done finding files\n";
# set up node colours
#$totaltime=0.0;
#$maxinctime=0.0;
for $i (0..$#c){
#$data contains %Time -- exclusive msec -- inclusive msec #Call #Subrs
    $data[$i] =~ s/^\s+//;
    $data[$i] =~ s/\s+$//;
    $data[$i] =~ s/\s+/ /g;

    @tmp = split(/ /,$data[$i]);
    $tmp[1] =~ s/,//g;
    $tmp[2] =~ s/,//g;

    ###$totaltime += $tmp[1];
    $exctime[$i] = log($tmp[1]+1.0);
    #$testtime[$i]= log($tmp[1]);
    $inctime[$i]= $tmp[2];
    $loginctime[$i]=log($tmp[2]+1.0);
    $calls[$i] = $tmp[3];
    # if($tmp[2] > $maxinctime){
    # 	$maxinctime = $tmp[2];
    # }
    #print STDERR "maxinctime $maxinctime $tmp[2]\n";
    #print "$data[$i]\n";
    #print "$exctime[$i]\n";
}

for $i (0..$#c){
    $red = ($exctime[$i]-$mintime)/$deltatime; # colours exc time wrt to max ext time
    #$red = ($exctime[$i])/log($maxinctime+1.0); # this colours exc time to same scale as inc time
    $hue = 2.0*(1.0-$red)/3.0;
    #print ">> $exctime[$i] <<red $red green $green\n";
    $nodecol[$i] ="\"$hue, 0.7, 1.0\"";
    $hue=2.0*(1.0-$loginctime[$i]/log($maxinctime+1.0))/3.0;
    $nodecol2[$i] ="\"$hue, 0.7, 1.0\"";
}
$text="";
for $i (0..$#c){
    @tmp = split(/\n/, $flist[$i]);
    #print Dumper(@tmp);
    if($#tmp > 0){
	for $j (0..$#tmp){
	    if($tmp[$j] =~ /\w+/){
		$tmp2 = `head -$l[$i] $tmp[$j] | tail -1`;
		#print "$f[$i] xxxxx  $tmp2 xxxx line num = $l[$i]\n";
		if($tmp2 =~ /$f[$i]/){
		    if($tmp2 =~ s/\n+//){
			$label[$i] = "$c[$i]:$l[$i]";
			$url[$i] = "file://$tmp[$j]";
			#$url[$i] = "$tmp[$j]";
		    }
		    else{
			$label[$i]= "";
		    }
		    #print "$f[$i] xxxxx  $label[$i] xxxx line num = $l[$i]\n";
		    
		}
	    }
	}
    }
    else{
	$label[$i] = "$c[$i]:$l[$i]";
	$url[$i] = "file://$tmp[0]";
	#$url[$i] = "$tmp[0]";
    }
    $percenttime=100.0*(exp($exctime[$i])-1.0)/$maxinctime;
    $perinctime = 100.0*$inctime[$i]/$maxinctime;
    if($percenttime > 0.0001){
	$percenttime = sprintf("%.4f", $percenttime);
    }
    else{
	$percenttime = sprintf("%.3g", $percenttime);
    }
    if($perinctime > 0.0001){
	$perinctime = sprintf("%.4f", $perinctime);
    }
    else{
	$perinctime = sprintf("%.3g", $perinctime);
    }   
#    print "$f[$i] [label=\"$f[$i]\\n$c[$i]:$l[$i]\\nExc Time \% $percenttime\\nInc Time \% $perinctime\\n\#Calls $calls[$i] \", URL=\"$url[$i]\", fillcolor=$nodecol[$i], color=$nodecol2[$i], penwidth=4, style=filled, tooltip=\"Exc Time \% $percenttime Inc Time \% $perinctime \#Calls $calls[$i] [ $f[$i] ] [ $c[$i]:$l[$i] ]\"];\n";
    $text = "$text"."$f[$i] [label=\"$f[$i]\\n$c[$i]:$l[$i]\\nExc Time \% $percenttime\\nInc Time \% $perinctime\\n\#Calls $calls[$i] \", URL=\"$url[$i]\", fillcolor=$nodecol[$i], color=$nodecol2[$i], penwidth=4, style=filled, tooltip=\"Exc Time \% $percenttime Inc Time \% $perinctime \#Calls $calls[$i] [ $f[$i] ] [ $c[$i]:$l[$i] ]\"];\n";
#    print "$flist[$i]";
}
$filename="$ARGV[1]"; # the dot file
open FILE, "$filename"  or die "can't find file: $!\n";
$i=0;
$newfile=$filename;
$newfile =~ s/(\w+)\.dot/$1/;
$newfile="$newfile"."2.dot";
open(FILEOUT, ">$newfile");
while (<FILE>){
    print FILEOUT "$_";
    if($i==5){ print FILEOUT "$text"; }
    $i++;
}
close FILE;
#_IArray_Destruct [{IArray.c} {58,0}] 
#stgMainLoop [{main.c} {137,0}] 
#usage: pprof -a outall
#       ./postproc.pl outall mainloop.dot | tee test.txt
#       ./postproc.pl outall mainloop.dot $HOME/$UWDIR
#dot -Tps mainloop.dot -o mainloop.ps
#a2ping mainloop.ps mainloop.pdf

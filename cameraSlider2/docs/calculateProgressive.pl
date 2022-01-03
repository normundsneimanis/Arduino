#!/usr/bin/perl -w
use strict;

my $length = 850; # mm
my $step = 1; # mm
my $startTime = 1; # seconds
my $increase = 0.33; # seconds

my $fh;
if (!open($fh, ">", "calculateIncreasing.csv")) {
	die("Couldn't open calculateIncreasing.csv: $!\n");
}
print $fh "StartTime	Step	Increase(Config)	Increase(sec)	Resulting Video Length	Last frame wait	Total filming time	First second filming time	Last second filming time\n";

while ($startTime < 30) {
	for (my $s = $step; $s <= 30; $s++) {
		for (my $j = $startTime; $j <= $increase * 30; $j += $increase) {
			$j = sprintf("%.2f", $j);
			calculate($fh, "increase", $s, $j, $startTime);
		}
	}
	if ($startTime == 1) {
		$startTime = 0; # make sure startTime is 5,10,15 etc.
	}
	$startTime+=5;
}

close $fh;
if (!open($fh, ">", "calculateDecreasing.csv")) {
	die("Couldn't open calculateDecreasing.csv: $!\n");
}
print $fh "StartTime	Step	Increase(Config)	Increase(sec)	Resulting Video Length	Last frame wait	Total filming time	First second filming time	Last second filming time\n";

$startTime = 99;
while ($startTime > 5) {
	for (my $s = $step; $s <= 30; $s++) {
		for (my $j = $increase; $j <= $increase * 30; $j += $increase) {
			$j = sprintf("%.2f", $j);
			calculate($fh, "decrease", $s, $j, $startTime);
		}
	}
	if ($startTime == 99) {
		$startTime = 100;
	}
	$startTime-=5;
}


close $fh;
if (!open($fh, ">", "calculateSlider.csv")) {
	die("Couldn't open calculateSlider.csv: $!\n");
}
print $fh "StartTime	Step	Resulting Video Length	Frame filming time	Total filming time\n";

$startTime = 1;
while ($startTime < 95) {
	#for (my $s = $step; $s <= 30; $s++) {
	for (my $s = 3; $s <= 30; $s++) {
		calculate($fh, "slider", $s, $startTime, $startTime);
	}
	if ($startTime == 1) {
		$startTime = 0; # make sure startTime is 5,10,15 etc.
	}
	$startTime+=5;
}


sub calculate {
	my $fh = shift;
	my $type = shift; # increase / decrease
	my $s = shift;
	my $j = shift;
	my $startTime = shift;
	my $waitEnd = $startTime;
	my $totalWait = $startTime;
	my $totalFrames = 0;
	my $firstSecond = 0;
	my $lastSecond = 0;
	for (my $i = 0; $i < $length; $i += $s) {
		$totalFrames++;
		if ($type eq "increase") {
		    $waitEnd += $j;
			if ($totalFrames <= 25) {
				$firstSecond += $waitEnd;
				#print "First second: $firstSecond j: $j frame: $totalFrames\n";
			}
			if ($totalFrames > ($length / $s) - 25) {
				$lastSecond += $waitEnd;
				#print "Last second: $lastSecond j: $waitEnd frame: $totalFrames\n";
			}
		} elsif ($type eq "decrease") {
			$waitEnd -= $j;
			if ($totalFrames <= 25) {
				$firstSecond += $waitEnd;
				#print "First second: $firstSecond j: $j frame: $totalFrames\n";
			}
			if ($totalFrames > ($length / $s) - 25) {
				$lastSecond += $waitEnd;
				#print "Last second: $lastSecond j: $waitEnd frame: $totalFrames\n";
			}
		} elsif ($type eq "slider") {
			$totalFrames++;
			$waitEnd = $startTime;
		}
	    $totalWait += $waitEnd;
		if (($type eq "decrease") and ($waitEnd <= 1)) {
			last;
		}
	}
	# if distance driven is less then 70cm, exit
	if ($s * $totalFrames < 800) {
		return;
	}

	return if ($totalWait / 60 / 60 > 24); # skip if slider time is more then 24 hours
	return if ($totalFrames / 25 < 5);
	my $incrPrint = int($j * 3 + 0.5);
	my $waitEndPrint = sprintf("%.2f", $waitEnd / 60) . " min";
	my $totalWaitPrint = sprintf("%.2f", $totalWait / 60 / 60) . " hours";
	if ($totalWait / 60 / 60 < 3) {
		$totalWaitPrint .= " (" . sprintf("%.0f", $totalWait / 60) . " min)";
	}
	if ($type eq "increase") {
		if ($firstSecond > 120) {
			$firstSecond /= 60;
			$firstSecond = sprintf("%.0f min", $firstSecond);
		} else {
			$firstSecond = sprintf("%.0f sec", $firstSecond);
		}
		$lastSecond /= 60;
		$lastSecond = sprintf("%.0f", $lastSecond);
		$totalWaitPrint .= "	$firstSecond	$lastSecond min";
	} elsif ($type eq "decrease") {
		$waitEndPrint = sprintf("%.0f", $waitEnd) . " sec";
		$firstSecond /= 60;
		$firstSecond = sprintf("%.0f", $firstSecond);
		if ($lastSecond > 120) {
			$lastSecond /= 60;
			$lastSecond = sprintf("%.0f min", $lastSecond);
		} else {
			$lastSecond = sprintf("%.0f sec", $lastSecond);
		}
		$totalWaitPrint .= "	$firstSecond min	$lastSecond";
	} elsif ($type eq "slider") {
		$waitEndPrint = sprintf("%.2f", $startTime * 25 / 60) . " min";
		$totalWaitPrint = sprintf("%.2f", $startTime * $totalFrames / 60) . " min";
		if ($startTime * $totalFrames / 60 > 3) {
			$totalWaitPrint = sprintf("%.2f", $startTime * $totalFrames / 60 / 60) . " hours";
		}
		return if ($totalFrames / 25 < 10);
		return if ($totalWait / 60 < 30);
	}
	my $totalFramesPrint = sprintf("%.0f", $totalFrames / 25);
	my $sign = ($type eq "increase" ? "+" : "-");
	if ($type ne "slider") {
		print $fh "$startTime	$s	$sign$incrPrint	$j	$totalFramesPrint	$waitEndPrint	$totalWaitPrint\n";
	} else {
		print $fh "$startTime	$s	$totalFramesPrint	$waitEndPrint	$totalWaitPrint\n";
	}
}

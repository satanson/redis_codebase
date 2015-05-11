#!/usr/bin/perl -w
use strict;
use Redis::hiredis;
use FileHandle;
use Carp qw(carp croak confess cluck);
use Digest::CRC qw(crc);
sub connect_to_redis_cluster{
	my $cfg=shift;
	my $CFG=FileHandle->new("< $cfg") or croak "fail to open file '$cfg'";
	my @slots;
	my %redises;
	while(<$CFG>){
		push @slots,[split /\s+/,$_,3];
		my $node=$slots[-1][0];
		$redises{$node}=Redis::hiredis->new();
		$redises{$node}->connect(split(/:/,$node)); 
	}
	return (\@slots,\%redises);
}


sub customize_crc{
	my %p=@_;
	sub{
		crc(shift,$p{width},$p{init},$p{xorout},$p{refout},$p{poly},$p{refin},$p{cont})%2**14;
	}
}

my $crc14=customize_crc(width=>16,init=>0,xorout=>0,refout=>0,poly=>0x1021,refin=>0,cont=>1);

sub query{
	my ($cmd,$slots,$redises,$reply_table)=@_;
	my $key=(split /\s+/,$cmd,3)[1];
	if ($key){
		my $idx=$crc14->($key);
		my $node=(grep{$_->[1]<=$idx && $idx<$_->[2]} @$slots)[0][0];
		my $redis=$redises->{$node};
		print "$key,$idx,$node\n";
		my $reply;
		eval{
			$reply=$redis->command($cmd);
		};
		if ($@){
			carp "$@";	
			return;
		}
		if (ref($reply) eq ref([])){
			print join(",",@$reply),"\n";
		}else{
			print "$reply\n";
		}
	}else{
		print "$cmd\n";
	}
}
my($slots,$redises)=connect_to_redis_cluster($ARGV[0]);
@ARGV=();
while(<>){
	chomp;
	query($_,$slots,$redises);
}

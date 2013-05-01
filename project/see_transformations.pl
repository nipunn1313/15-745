#!/usr/bin/perl

use strict;
if ($#ARGV < 0) {
  print "Need arg\n";
  die;
}

my $cfile = $ARGV[0];

my $opt_prefix = "/home/user/llvm-current/build-x86/Debug+Asserts/bin";
if ($#ARGV > 0 && $ARGV[1] eq "old") {
  $opt_prefix = "/home/user/llvm/llvm-3.2.install/bin";
}

my $prefix = $cfile;
$prefix =~ s/\.c//;
my $bcfile = $prefix . ".bc";
my $orig_bcfile = $prefix . ".bc";
my $llfile = $prefix . ".ll";

# Add optimization options here in order
#my @opt_options = ("-mem2reg -loop-simplify -simplifycfg -loop-idiom -lcssa -sink -load ./LoopVectorize.so -my-loop-vectorize -force-vector-width=4");
#my @opt_options = ("-mem2reg -debug -loop-vectorize -force-vector-width=4");
my @opt_options = ("-mem2reg", "-loop-simplify", " -loop-idiom", " -sink", " -simplifycfg", " -O3", " -debug -loop-vectorize -force-vector-width=4");
#my @opt_options = ("-mem2reg", "-loop-simplify", " -loop-idiom", " -sink", " -simplifycfg", " -O3");

my $opt_basic_options = " -mem2reg -loop-simplify -loop-idiom -sink -simplifycfg -O3 ";


# First do naive compilation to llvm ir:
`clang -O0 -emit-llvm -o $bcfile -c $cfile`;
`llvm-dis $bcfile`;

`cp $bcfile $orig_bcfile`;

# Now run the options one by one and show tkdiff 
my $current_options = "";

my $previous_dis_file = $llfile;

my $llvm_bc;

foreach my $new_option (@opt_options) {
    $current_options .= " $new_option";
    
    my $file_string = $current_options;
    $file_string =~ s/-//g;
    $file_string =~ s/\///g;
    $file_string =~ s/ +/./g;
    $llvm_bc =  $prefix . "$file_string.bc" ;
    my $llvm_dis = $llvm_bc;
    $llvm_dis =~ s/.bc/.ll/;

    print "$opt_prefix/opt $current_options -o $llvm_bc $bcfile\n";
    `$opt_prefix/opt $current_options -o $llvm_bc $bcfile`;
    
    # Copy to a final optimized bitcode file
    `cp $llvm_bc $prefix.opt.bc`;

    print "llvm-dis $llvm_bc\n";
    `llvm-dis $llvm_bc`;
    
    # Remove the intermediate bit-code:
    `rm $llvm_bc`;

    print "tkdiff $previous_dis_file $llvm_dis\n";
    `tkdiff $previous_dis_file $llvm_dis`;

    `rm $previous_dis_file`;
    $previous_dis_file = $llvm_dis;
}     

## Create a base line optimized version :
print "Creating baseline version using basic options\n";
print "$opt_prefix/opt $opt_basic_options -o $llvm_bc $orig_bcfile\n";
`$opt_prefix/opt $opt_basic_options -o $llvm_bc $orig_bcfile ; mv $llvm_bc $orig_bcfile`;

`rm $previous_dis_file`;

# Run the valgrind script:
print "*********************************************************************\n";
print "Running valgrind\n";
print "*********************************************************************\n";
`./compile_and_run_valgrind.sh $prefix`;

#!/usr/bin/perl

my $cfile = "simd.cpp";

# Add optimization options here in order
my @opt_options = ("-mem2reg", "-loop-simplify", "-simplifycfg", "-loop-idiom", "-lcssa", "-sink", "-loop-vectorize -force-vector-width=4");


# First do naive compilation to llvm ir:
`clang -O0 -emit-llvm -o simd.bc -c simd.c`;
`llvm-dis simd.bc`;

# Now run the options one by one and show tkdiff 
my $current_options = "";

my $previous_file = "simd.ll";

foreach my $new_option (@opt_options) {
    $current_options .= " $new_option";
    
    my $file_string = $current_options;
    $file_string =~ s/-//g;
    $file_string =~ s/ +/./g;
    my $llvm_bc =  "simd" . "$file_string.bc" ;
    my $llvm_dis = $llvm_bc;
    $llvm_dis =~ s/.bc/.ll/;

    print "opt $current_options -o $llvm_bc simd.bc\n";
    `opt $current_options -o $llvm_bc simd.bc`;
    
    # Copy to a final optimized bitcode file
    `cp $llvm_bc simd.opt.bc`;

    print "llvm-dis $llvm_bc\n";
    `llvm-dis $llvm_bc`;
    
    # Remove the intermediate bit-code:
    `rm $llvm_bc`;

    print "tkdiff $previous_file $llvm_dis\n";
    `tkdiff $previous_file $llvm_dis`;

    `rm $previous_file`;
    $previous_file = $llvm_dis;
}

`rm $previous_file`;


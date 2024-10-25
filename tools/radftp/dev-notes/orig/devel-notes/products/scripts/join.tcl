#!/usr/bin/tclsh
#
# Create the wsr88d-product.csv from the "awips, desc, file" files
#

set keys [split [exec cat "wsr88d-product-files.txt"] "\n"];
set descs [split [exec cat "wsr88d-product-desc.txt"] "\n"];
set awips [split [exec cat "wsr88d-product-awips.txt"] "\n"];

foreach k $keys d $descs a $awips {
    set wmoid [lindex [split $a] 1];
    set aw [string range [lindex [split $a] 2] 0 2];
    puts "$k, $aw, $wmoid, $d";
}

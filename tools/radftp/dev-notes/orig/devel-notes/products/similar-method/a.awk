# Usage: awk -f a.awk products.txt | cut -b 4- > product-list.txt

BEGIN {
    FS = " ";
}

{
    print $9;
}

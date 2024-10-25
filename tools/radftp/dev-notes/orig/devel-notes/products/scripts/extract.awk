# print the wmoid and awips from a file

NR ==1 {
    printf("%s ", $1);
}

NR == 2 {
    print $1;
}

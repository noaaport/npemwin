BEGIN {
  
  n1 = "0000000000";
  n2 = "0000000001";
}

{
  print $1, n1, n2, "y";
}

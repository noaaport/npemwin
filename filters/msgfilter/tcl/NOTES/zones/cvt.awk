BEGIN {
  FS = "|";
}

{
  state = tolower($1);
  zone = $2;

  printf("set forecastzones(%s,%s) \"", state, zone);
  for(i = 3; i < NF; ++i){
    printf("%s,", $i);
  }

  printf("%s\";\n", $NF);
}

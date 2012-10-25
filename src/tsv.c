/*
 * $Id$
 *
 * Library for importing a table of space-separated values.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "tsv.h"

/* 
 * This is used by new_buffer and any function that calls
 * new_buffer().
 */
#define CHUNK		BUFSIZ

/*
 * static variables
 */
static int tsv_error = 0;
static int tsv_argc = 0;
static char *tsv_line = NULL;
static int tsv_linesize = 0;	/* size of allocated buffer tsv_line */
static char **tsv_argv = NULL;
static char *tsv_cpline = NULL;

static char *new_buffer(char *old);
static int splitcount(char *s);
static int bufsplit(char *s,char **argv);
static char *tsvreadline(FILE *f);

int tsverror(void){

  return(tsv_error);
}

void tsvrelease(void){

  tsv_error = 0;
  tsv_argc = 0;

  free(tsv_line);
  free(tsv_argv);
  free(tsv_cpline);

  tsv_line = NULL;
  tsv_argv = NULL;
  tsv_cpline = NULL;

  tsv_linesize = 0;

  /*
   * This reinitializes the (local) static variables 
   */
  tsvgetline(NULL);
}

int tsvargc(void){

  return(tsv_argc);
}

char *tsvargv(int n){

  char *p = NULL;

  if(n < tsv_argc)
    p = tsv_argv[n];

  return(p);
}

char **tsvargvp(void){

  return(tsv_argv);
}

char *tsvgetline(FILE *fp){
  /* 
   * This function reads a line from f, then calls the functions to 
   * split the line into fields, and returns a pointer to the original line.
   * The storage is reused on subsequent calls,
   * so that the caller must make a copy of the line, and the fields,
   * if they are required.
   * Returns NULL if there was a memory or a file error, or when EOF was
   * encountered before anything was read.
   * In case of error, tvserror() returns 1 for a memory error or
   * 2 for a file error. It returns zero otherwise.
   */

  static int size_argv = 0;
  static size_t size_cpline = 0;
  int argc;

  /*
   * initialization by tsvrelease()
   */
  if(fp == NULL){
    size_argv = 0;
    size_cpline = 0;

    return(NULL);
  }

  tsv_line = tsvreadline(fp);
  if(tsv_line == NULL)
    return(NULL);

  if(size_cpline < strlen(tsv_line) + 1){
    free(tsv_cpline);
    tsv_cpline = malloc(strlen(tsv_line) + 1);
  }

  if(tsv_cpline != NULL){
    size_cpline = strlen(tsv_line) + 1;
    strncpy(tsv_cpline, tsv_line, size_cpline);
  }else{
    tsv_error = 1;
  }

  if(tsv_error == 0){
    tsv_argc = splitcount(tsv_line);
    if(tsv_argc > size_argv){
      free(tsv_argv);
      if( (tsv_argv = (char**)calloc(tsv_argc,sizeof(void*))) == NULL)
	tsv_error = 1;
      else
	size_argv = tsv_argc;
    }
  }

  if(tsv_error == 0){
    argc = bufsplit(tsv_cpline,tsv_argv);
    /* 
     * This would be a BUG
     */
    assert(tsv_argc == argc);
  }

  if(tsv_error != 0)    
    tsvrelease();
    
  return(tsv_line);
}

static char *tsvreadline(FILE *f){
  /* 
   * This function reads a line from the file.
   * Returns NULL if there was a memory or file error, or if EOF was
   * encountered before anything was read.
   * In case of error, tvserror() returns 1 for a memory error or
   * 2 for a file error. 
   */

  char *buf = NULL;
  int last;
  int c;
  int b = 0;
  char *p;

  /* 
   * Try to reuse the previously allocated storage, or create  new
   * one the first time.
   */
  if(tsv_line != NULL)
    buf = tsv_line;
  else if( (buf = new_buffer(NULL)) == NULL)
    return(NULL);

  last = tsv_linesize - 1;
  while( ((c = fgetc(f)) != EOF) && (c != '\n') ){
    if(b == last - 1){
      p = new_buffer(buf);
      if(p == NULL){
	free(buf);
	buf = NULL;
	tsv_error = 1;
	break;
      }
      buf = p;
      last = tsv_linesize - 1;
    }
    buf[b++] = c;
  }

  if(c == EOF){
    if(ferror(f) != 0)
      tsv_error = 2;
    
    if(b == 0){
      free(buf);
      buf = NULL;
    }
  }

  if(buf != NULL)
    buf[b] = '\0';
  
  return(buf);
}

static int bufsplit(char *s,char **argv){
  /* 
   * This function splits the string pointed to by buf, 
   * on blanks (strings of spaces and/or tabs and/or newlines).
   * It is asumed that the caller has called splitcount
   * prior to this, and that argv[] is large enough to hold
   * all the split pieces. It returns the number of elements
   * of the argv array that were filled.
   */
  int n;
  int i;
  int c;

  i = n = 0;
  c = s[0];
  while(c != '\0'){
    if(isspace(c)){
      while(isspace(c))
	c = s[++i];
    }else{
      argv[n] = &s[i];
      ++n;
      while(!isspace(c) && (c != '\0') ){
	c = s[++i];
      }
      s[i] = '\0';
    }
  }
  return(n);
}

static int splitcount(char *s){
  /* 
   * This function counts the number of tokens in which the string
   * pointed to by s would be split on spaces and/or tabs.
   */

  int n = 0;
  int c;
  int i = 0;

  c = s[0];
  while(c != '\0'){
    if(isspace(c)){
      while(isspace(c))
	c = s[++i];
    }else{
      ++n;
      while(!isspace(c) && (c != '\0') ){
	c = s[++i];
      }
    }
  }

  return(n);
}
      
static char *new_buffer(char *old){

  char *new;

  if(old == NULL){
    tsv_linesize = CHUNK;
    new = (char*)calloc(tsv_linesize,sizeof(char));
  }else{
    tsv_linesize += CHUNK;
    new = (char*)realloc(old,tsv_linesize*sizeof(char));
  }
  return(new);
}

/**
 * test
 **
int main(void){

  char *file = "testfile";
  FILE *fp;
  char *line;
  char *field;
  int i;
  int n;

  fp = fopen(file,"r");
  if(fp == NULL)
    perror(file);

  while( (line = tsvgetline(fp)) != NULL){
    n = tsvargc();
    for(i = 0; i < n; ++i){
      field = tsvargv(i);
      fprintf(stdout,"%s--",field);
    }
    fprintf(stdout,"\n");
  }

  fprintf(stderr,"tsverror = %d\n",tsverror());
  tsvrelease();

  fprintf(stderr,"tsvrelease() seems to be ok too\n");

  return(0);
}

**/

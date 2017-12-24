
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

void option_b(int t, int index,int argc, char **argv) {
  int i,j,length;
  for (i=index; i<argc; ++i){
    // get string and its length
    char *s = argv[i];
    length = 0;
    for(j=0;s[j] != '\0'; j++) {
      ++length;
    }
    j = 0;
    // any odd number of repetitions of e
    int count_e = 0;
    while(s[j] != '\0' && s[j] == 'e') {
      ++count_e;
      ++j;
    }
    
    if( !(count_e & 1) ) {
      if(t){
	continue;
      }
      else {
	printf("%s\n","no");
	continue;
      }
    }

    // exactly one "="
    if(s[j] != '\0' && s[j] != '=') {
      if(t) {
	continue;
      }
      else {
	printf("%s\n","no");
	continue;
      }
    }
     ++j;
    // between 1 and 3 decimal digits sequence X  
    int d = 0;
    while( s[j] != '\0' && isdigit(s[j]) >0  ){
      ++d;
      ++j;
    }
    int middle_X = j - d + 1;
    if( !(d>=1 && d<=3) ) {
      if(t) {
	continue;
      }
      else{
	printf("%s\n","no");
	continue;
      }
      
    }
   
    // 1 or more repetitions of w
    int w = 0;
    while(s[j] != '\0' && s[j] == 'w') {
      ++w;
      ++j;
    }
    if(w<1) {
      if(t){
	continue;
      }
      else {
	printf("%s\n","no");
	continue;
      }
    }

    // exactly one ":"
    if(s[j] != '\0' && s[j] != ':') {    
      if(t){
	continue;
      }
      else {
	printf("%s\n","no");
	continue;
      }
    }
    ++j;

    // an odd number of uppercase letters;
    int upper = 0;
    int f = 0;
    while(s[j] != '\0' && s[j] >= 'A' && s[j] <= 'Z') {
      if(s[j] == 'F') {
	++f;
      }
      ++upper;
      ++j;
    }
   
    if( !(upper & 1) ) {
      if(t){
	continue;
      }
      else {
	printf("%s\n","no");
	continue;
      }
    }

    // the same characters as the odd-positioned characters in X.
    // and trailing letter is not \0
    if((d>1 && s[j] != '\0' && s[j] != s[middle_X]) || s[++j] != '\0')  {
       if(t){
	continue;
      }
      else {
	printf("%s\n","no");
	continue;
      }
    }
 
    // match. print yes or reverse
    if(t){
      add_c(s,length,f);
    }
    else {
      printf("%s\n","yes");
    }
  }
    
  return ;
}

void add_c(char *s, int length, int f) {
  int size = length + f + 1;
  char *r = malloc(size);
  int j = 0;
  int count = 0;
  while(s[j] != '\0') {
    r[count] = s[j];
    if(s[j] == 'F') {
      r[++count] = 'C';
    }
    ++count;
    ++j;
  }
  r[length+f] = '\0';
  printf("%s\n",r);
}

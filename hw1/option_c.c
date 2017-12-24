
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

void option_c(int t, int index,int argc, char **argv) {
  int i,j,length;
  for (i=index; i<argc; ++i){
    // get string and its length
    char *s = argv[i];
    length = 0;
    for(j=0;s[j] != '\0'; j++) {
      ++length;
    }
    j = 0;
    // any odd number of repetitions of m
    int m = 0;
    while(s[j] != '\0' && s[j] == 'm') {
      ++m;
      ++j;
    }
    
    if( m & 1 ) {
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
      if(t){
	continue;
      }
      else {
	printf("%s\n","no");
	continue;
      }
    }
    ++j;
    // an odd number of uppercase letters sequence X;
    int upper = 0;
    int fg = 0;
    while(s[j] != '\0' && s[j] >= 'A' && s[j] <= 'Z') {
      if(s[j] == 'F' || s[j] == 'G' ) {
	++fg;
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
    int first_X = j - upper;
    int last_X = j -1; 
     // 4 or more repetitions of z
    int w = 0;
    while(s[j] != '\0' && s[j] == 'z') {
      ++w;
      ++j;
    }
    if(w<4) {
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
    // the same characters as the odd-positioned characters in X.
    int count = 1;
    while(upper>1 && first_X < last_X && s[j] != '\0' && isdigit(s[j]) == 0 )  {
      if (s[first_X + count] != s[j]) {
	break;
      }     
      if(s[j] == 'F' || s[j] == 'G' ) {
	++fg;
      }
      count += 2;
      ++j;      
    }
    if(s[j] != '\0' && isdigit(s[j]) == 0) {
      if(t){
	continue;
      }
      else {
	printf("%s\n","no");
	continue;
      }
    }
    
    // between 1 and 3 decimal digits
    // trailing letter is not \0
    int d = 0;
    while( s[j] != '\0' && isdigit(s[j]) >0  ){
      ++d;
      ++j;
    }
    
    if( !(d>=1 && d<=3) || s[j] != '\0' ) {
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
      remove_gf(s,length,fg);
    }
    else {
      printf("%s\n","yes");
    }
  }
  
  return ;
}


void remove_gf(char *s, int length,int fg) {
  int size = length -fg + 1;
  char *r = malloc(size);
  int j = 0;
  int r_count = -1;
  while(s[j] != '\0') {
    if(!(s[j] == 'F' || s[j] == 'G')) {
      r[++r_count] = s[j];
    }
    ++j;
  }
  r[size-1] = '\0';
  printf("%s\n",r);
}

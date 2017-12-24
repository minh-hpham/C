#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
// AUTHOR: MINH PHAM
// ID : U0975798
void option_a(int t, int index, int argc, char **argv);
void option_b(int t, int index, int argc, char **argv);
void option_c(int t, int index, int argc, char **argv);
void reverse_string(char *s,int length);
void add_c(char *s,int length, int f);
void remove_gf(char *s,int length, int fg);

int main( int argc, char **argv) {
  if (argc < 2)
    return 1;
  int i=0;
  char option='a';
  int t=0;
  // find flags
  for (i = 1; i < argc && argv[i][0] == '-' ; i++ ) {
    if(argv[i][1] == 'a')
      option = 'a';    
    else if (argv[i][1] == 'b')
      option = 'b';
    else if (argv[i][1] == 'c')
      option = 'c';
    else if (argv[i][1] == 't')
      t = 1;;
  }
  // handle flag cases
  if(option == 'a')
    option_a(t,i,argc,argv);
  else if(option == 'b')
    option_b(t,i,argc,argv);
  else if(option == 'c')
    option_c(t,i,argc,argv);
 
  return 0;
}

void option_a(int t, int index,int argc, char **argv) {
  int i,j,length;
  for (i=index; i<argc; ++i){
    char *s = argv[i];
    length = 0;
    for(j=0;s[j] != '\0'; j++) {
      ++length;
    }
    j = 0;
    // i*
    while(s[j] != '\0' && s[j] == 'i') {
       ++j;
    }
    // not i* or not :
    if(s[j] != '\0' && s[j] != ':') {
      if(t) {
	continue;
      }
      else {
	printf("%s\n","no");
	continue;
      }
    }
    ++j;
    // between 2 and 6 repetitions of x
    int x = 0;

    while(s[j] != '\0' && s[j] == 'x') {
      ++x;
      ++j;
    }
    if( !(x>=2 && x<=6) ) {
      if(t){
	continue;
      }
      else {
	printf("%s\n","no");
	continue;
      }
    }

    // ,
    if(s[j] != '\0' && s[j] != ',') {    
      if(t){
	continue;
      }
      else {
	printf("%s\n","no");
	continue;
      }
    }
    ++j;

    // between 1 and 3 decimal digits
    
    int d = 0;
	
    while( s[j] != '\0' && isdigit(s[j]) >0  ){
      ++d;
      ++j;
    }
  
    if( !(d>=1 && d<=3) ||  s[j] != '\0' ) {
      if(t) {
	continue;
      }
      else{
	printf("%s\n","no");
	continue;
      }
      
    }
    // match. print yes or reverse
    if(t){
      reverse_string(s,length);
      printf("%s\n",s);
    }
    else {
      printf("%s\n","yes");
    }
  }
    
  return ;
}
void reverse_string(char *s, int length) {
  char *start = s;
  char *end = s + length-1;
  while (end > start) {
    char t = *end;
    *end-- = *start;
    *start++ = t;
  }
}
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

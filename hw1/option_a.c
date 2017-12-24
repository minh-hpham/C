
#include <stdio.h>
#include <ctype.h>
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

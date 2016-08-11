#include <stdio.h>
#include <string.h>
 
int main(void)
{
    char s[] = "What's sauce for the goose is sauce for the gander.";
    char t[25];
     
    strncpy(t, s, 20);
     
    printf("%s\n, %s\n", t,s);
     
     
    return 0;    
}

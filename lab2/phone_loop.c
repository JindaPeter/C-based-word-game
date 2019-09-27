#include <stdio.h>
#include <stdlib.h>

int main(){
    char phone[11];
    int i;
    int error = 0;    

    scanf("%s", phone);
    
    while (scanf("%d", &i) != EOF){
      if (i < -1 || i > 9){
        printf("ERROR\n");
	error = 1;	
      } else if (i == -1){
        printf("%s\n", phone);
      } else {
        printf("%c\n", phone[i]);
      }
    }

    return error;
}

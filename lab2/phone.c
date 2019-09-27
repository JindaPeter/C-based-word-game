#include <stdio.h>
#include <stdlib.h>

int main(){
    char phone[11];
    int i;
    
    scanf("%s%d", phone, &i);

    if (i < -1 || i > 9){
      printf("ERROR");
      return 1;	
    } else if (i == -1){
      printf("%s", phone);
      return 0;
    } else {
      printf("%c", phone[i]);
      return 0;
    }
}

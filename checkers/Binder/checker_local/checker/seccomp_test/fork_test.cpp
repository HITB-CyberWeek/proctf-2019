#include <stdio.h>
#include <unistd.h>


int main(){

  for (int i=0; i<10; i++){
    if (!fork()){
        printf("%d\n",i);
        return 0;
    }
  }

}

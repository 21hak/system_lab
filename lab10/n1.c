#include <unistd.h>

int main(void){
    if(write(1, "study hard\n", 15) < 0){
        write(2, "error", 6);
    }
    return 1;
}
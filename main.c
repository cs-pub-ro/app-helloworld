#include <stdio.h>
#include <fcntl.h> 
#include <unistd.h>

int main(void)
{
    int ret = access("./test.txt", F_OK);

    if (ret) {
        printf("data viitoare, nenicule\n");
    } else {
        printf("iz a win\n");
    }
}
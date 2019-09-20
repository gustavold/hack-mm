#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(){

        char *p = (char*)0x140000000;
        char *buf = malloc(64*1024*3);
	buf += 64*1024;
        buf = (char*) ((unsigned long)buf & ~0xffff);

        strcpy(buf, "Surprise!");

        printf("my pid: %d\n", getpid());
        printf("page address: %p\n", buf);
        getchar();

        printf("found: %s\n", p);
        printf("should have been: %s\n", buf);
        getchar();

        return 0;
}


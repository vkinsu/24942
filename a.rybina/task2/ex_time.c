#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

extern char *tzname[];

int main(){
    time_t now;

    // Set timezone to Pacific Standard Time (California)
    // This works on UNIX systems including Solaris
    setenv("TZ", "PST8PDT", 1);
    tzset(); // Update timezone information

    (void) time( &now );

    printf("Current time in California:\n");
    printf("%s", ctime( &now ) );

    return 0;
}
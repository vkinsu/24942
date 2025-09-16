#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

extern char *tzname[];

int main(){
    time_t now;
    struct tm *sp;

    // Set timezone to Pacific Standard Time (California)
    // This works on UNIX systems including Solaris
    setenv("TZ", "PST8PDT", 1);
    tzset(); // Update timezone information

    (void) time( &now );

    printf("Current time in California:\n");
    printf("%s", ctime( &now ) );

    sp = localtime(&now);
    printf("Formatted: %d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);
    return 0;
}
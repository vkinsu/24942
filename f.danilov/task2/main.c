#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(){
    setenv("TZ", "America/Los_Angeles", 1); // los angeles - california
    tzset(); 
    
    time_t now;
    struct tm *sp;

    (void)time(&now);

    // в локальное (теперь — PST/PDT)
    sp = localtime(&now);
    
    //  время в формате, аналогичном ctime
    printf("%s", asctime(sp));
    
    return 0;
}
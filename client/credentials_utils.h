#include "utils.h"


//cat /etc/passwd | egrep "(\/bin\/bash)|(\/bin\/sh)" | cut -f1 -d: |tr "\n" " "
//Momentan folosim system pentru convenienta :)
void etc_passwd_shadow_checker(int fd_out);
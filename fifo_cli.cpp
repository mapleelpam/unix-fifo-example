#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "fifo_info.h"

int main()
{
   int fd_c2s;
   char *myfifo = STR_PATH_C2S;

   int fd_s2c;
   char *myfifo2 = STR_PATH_S2C;

   char str[BUFSIZ];
   printf("Input message to serwer: ");
   scanf("%s", str);


   /* write str to the FIFO */
   fd_c2s = open(myfifo, O_WRONLY);
   fd_s2c = open(myfifo2, O_RDONLY);
   write(fd_c2s, str, sizeof(str));

   perror("Write:"); //Very crude error check

   read(fd_s2c,str,sizeof(str));

   perror("Read:"); // Very crude error check

   printf("...received from the server: %s\n",str);
   close(fd_c2s);
   close(fd_s2c);

   /* remove the FIFO */

   return 0;
}

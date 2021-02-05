#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *target;

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void 
findHelper(char *path) 
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, " %s can not open\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "can not stat\n");
        close(fd);
        return;
    }
    
    // fprintf(2, "========");
    // fprintf(2, "%d %s\n", st.type, fmtname(path));
        int k = strcmp(target, fmtname(path));

    switch(st.type){
        case T_FILE:
        fprintf(2, "k: %d %s\n", k, fmtname(path));
            if (k == 0) {
                printf("%s\n", path);
            }
            close(fd);
            break;

        case T_DIR:
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            // fprintf(2, "buf: %s\n", buf);
 
            while (1) {
                int  n = read(fd, &de, sizeof(de));
                // fprintf(2, "%d\n", n);
                if (n != sizeof(de))
                    break;
                
                if (de.inum == 0)
                    continue;

                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                    continue;
                strcpy(p, de.name);
                // memmove(p, de.name, DIRSIZ);
                // p[DIRSIZ] = 0;
                findHelper(buf);
            }
            close(fd);
            break;
    }
}

int 
main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(2, "should be three argements\n");
    }
    target = argv[2];
    fprintf(2, "target: %s\n", target);
    findHelper(argv[1]);
    exit(0);
}
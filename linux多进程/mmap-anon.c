/*
    匿名映射：不需要文件实体进程一个内存映射
*/

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {

    // 1.创建匿名内存映射区
    int len = 4096;
    void * ptr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(ptr == MAP_FAILED) {
        perror("mmap");
        exit(0);
    }

    // 父子进程间通信
    pid_t pid = fork();

    if(pid > 0) {
        // 父进程
        strcpy((char *) ptr, "hello, world");
        wait(NULL);
    }else if(pid == 0) {
        // 子进程
        sleep(1);
        printf("%s\n", (char *)ptr);
    }

    // 释放内存映射区
    int ret = munmap(ptr, len);

    if(ret == -1) {
        perror("munmap");
        exit(0);
    }
    return 0;
}

/*
1.如果对mmap的返回值(ptr)做++操作(ptr++), munmap是否能够成功?
void * ptr = mmap(...);
ptr++;  可以对其进行++操作
munmap(ptr, len);   // 错误,要保存地址

2.如果open时O_RDONLY, mmap时prot参数指定PROT_READ | PROT_WRITE会怎样?
错误，返回MAP_FAILED
open()函数中的权限建议和prot参数的权限保持一致。

3.如果文件偏移量为1000会怎样?
偏移量必须是4K的整数倍，返回MAP_FAILED

4.mmap什么情况下会调用失败?
    - 第二个参数：length = 0
    - 第三个参数：prot
        - 只指定了写权限
        - prot PROT_READ | PROT_WRITE
          第5个参数fd 通过open函数时指定的 O_RDONLY / O_WRONLY

5.可以open的时候O_CREAT一个新文件来创建映射区吗?
    - 可以的，但是创建的文件的大小如果为0的话，肯定不行
    - 可以对新的文件进行扩展
        - lseek()
        - truncate()

6.mmap后关闭文件描述符，对mmap映射有没有影响？
    int fd = open("XXX");
    mmap(,,,,fd,0);
    close(fd); 
    映射区还存在，创建映射区的fd被关闭，没有任何影响。

7.对ptr越界操作会怎样？
void * ptr = mmap(NULL, 100,,,,,);
4K
越界操作操作的是非法的内存 -> 段错误

*/
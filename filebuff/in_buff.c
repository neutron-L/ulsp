/*
 * 探索《TLPI》中的设置流缓冲的接口setvbuf
 * 本程序用setvbuf为stdin设置缓冲区，尤其观察fflush(stdin)的效果
 * 结论：fflush(stdin)是未定义行为，在我的环境中，其作用几乎没有，无法
 * 像书本上描述的，丢弃已经缓冲的输入数据，而是继续读取以前曾经有的数据。
 * 值得一提的是，标准库的带缓冲io函数，内部应该会追踪缓冲区的状态，如当前读取
 * 的字节数等等。
 * */
#include "tlsp_hdr.h"

int main()
{
    char buf[BUFSIZ];
    char in[64];

    if (setvbuf(stdin, buf, _IOLBF, BUFSIZ))
        errExit("setvbuf");
    scanf("%s", in);
    printf("in %s;\nbuf: %s\n", in, buf);
    scanf("%s", in);
    printf("in %s;\nbuf: %s\n", in, buf);
    fflush(NULL);
    printf("after flush, buf: %s\n", buf);
    scanf("%s", in);
    printf("in %s;\nbuf: %s\n", in, buf);
    scanf("%s", in);
    printf("in %s;\nbuf: %s\n", in, buf);
    scanf("%s", in);
    printf("in %s;\nbuf: %s\n", in, buf);

    scanf("%s", in);
    printf("in %s;\nbuf: %s\n", in, buf);

    exit(EXIT_SUCCESS);
}
#include "./dns.c"


int main(int argc, char **argv)
{
    if(argc != 2){
        fprintf(stderr, "Usage dns <域名> \n");
        return 0;
    }
    char **list = getipbyname(argv[1]);
    for (int i = 0; list[i] != NULL; ++i) {
        puts(list[i]);
        free(list[i]);
    }
    free(list);
    return EXIT_SUCCESS;
}
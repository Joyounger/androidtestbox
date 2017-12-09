

#include <stdio.h>
#include <string.h>

// Parameters for transfer list command functions
typedef struct {
    char* cmdname;
    char* cpos;
    /*char* freestash;
    char* stashbase;
    int canwrite;
    int createdstash;
    int fd;
    int foundwrites;
    int isunresumable;
    int version;
    int written;
    NewThreadInfo nti;
    pthread_t thread;
    size_t bufsize;
    uint8_t* buffer;
    uint8_t* patch_start;*/
} CommandParameters;

int main()
{
    CommandParameters params;
    memset(&params, 0, sizeof(params));
    
    
    char* line = "move b569d4f018e1cdda840f427eddc08a57b93d8c2e 2,545836,545840 4 2,545500,545504\n";
    
    params.cmdname = strtok_r(line, " ", &params.cpos);

    //printf("line is %s\n", line);
    //printf("params_cmdname is %s\n", params.cmdname);
    printf("params_cpos is %s\n", params.cpos);
    
    return 0;
}

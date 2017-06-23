


#include <stdio.h>

// Parameters for transfer list command functions
typedef struct {
    char* cmdname;
    char* cpos;
    char* freestash;
    char* stashbase;
    int canwrite;
    int createdstash;
    int fd;
    int foundwrites;
    int isunresumable;
    int version;
    int written;
    //NewThreadInfo nti;
    //pthread_t thread;
    size_t bufsize;
    uint8_t* buffer;
    uint8_t* patch_start;
} CommandParameters;

// Definitions for transfer list command functions
typedef int (*CommandFunction)(CommandParameters*);

typedef struct {
    const char* name;
    CommandFunction f;
} Command;

static int PerformCommandMove(CommandParameters* params) {
}

static int PerformCommandStash(CommandParameters* params) {
}

static int PerformCommandFree(CommandParameters* params) {
}

static int PerformCommandZero(CommandParameters* params) {
}

static int PerformCommandNew(CommandParameters* params) {
}

static int PerformCommandDiff(CommandParameters* params) {
}

static int PerformCommandErase(CommandParameters* params) {
}


int main()
{
    const Command verify_commands[] = {     // BlockImageVerifyFn
        { "bsdiff",     PerformCommandDiff  },
        { "erase",      NULL                },
        { "free",       PerformCommandFree  },
        { "imgdiff",    PerformCommandDiff  },
        { "move",       PerformCommandMove  },
        { "new",        NULL                },
        { "stash",      PerformCommandStash },
        { "zero",       NULL                }
    };

    const Command update_commands[] = {        // BlockImageUpdateFn
        { "bsdiff",     PerformCommandDiff  },
        { "erase",      PerformCommandErase },
        { "free",       PerformCommandFree  },
        { "imgdiff",    PerformCommandDiff  },
        { "move",       PerformCommandMove  },
        { "new",        PerformCommandNew   },
        { "stash",      PerformCommandStash },
        { "zero",       PerformCommandZero  }
    };
    
    printf("BlockImageVerifyFn cmdcount--%d\n", sizeof(verify_commands) / sizeof(verify_commands[0]));
    printf("BlockImageUpdateFn cmdcount--%d\n", sizeof(update_commands) / sizeof(update_commands[0]));
    
}


/* 
$ ./calc_cmdcount
BlockImageVerifyFn cmdcount--8
BlockImageUpdateFn cmdcount--8
 */
/*

usage: /system/bin/klogcat [-f log_path],
-f参数可选,后面指定自定义的日志路径,如果未指定-f,默认将log输出到标准输出
eg:  /system/bin/klogcat -f /data/property/kmsg.log

注意:如果要在自定义路径多次执行klogcat保存多份log, log文件名直接用kmsg.log即可,不需要手动修改文件名,否则会修改之前保存的log.
程序会自动将之前的log重命名,最近一次重命名为kmsg.log.1,以此类推直到kmsg.log.10,最多保存之前10次log,之后最晚一次的kmsg.log.10会被覆盖
*/

#define LOG_TAG "klogcat"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <cutils/log.h>
#include <cutils/properties.h>

static const int LOG_NUM = 10;                          // the max number of log files we want to store.
static const unsigned long long LOG_SIZE = 83886080;    // the max size of log file we want to store, now is 80MB
const char* log_path;                                   // log_path is meaningful when -f optiob is set

void rotate_logs(int max, const char *log_path);
int read_dev_kmsg(int fd_out);

int main(int argc __unused, char **argv __unused)
{
    umask(S_IWGRP | S_IWOTH); // pre_umask is 077, set umask is 022, for group and other user to read.

    int retval;
    int opt;
    bool f_flag = false;
    const char* optstr = ":f:";

    while ((opt = getopt(argc, argv, optstr)) != -1) {
        switch (opt) {
            case 'f':
                log_path = optarg;
                ALOGD("log_path--%s", log_path);
                f_flag = true;
                break;
            case ':':
                ALOGE("klogcat: option need a value");
                ALOGE("usage: /system/bin/klogcat [-f log_path]");
                break;
            case '?':
                ALOGE("klogcat: unknown option:%c", optopt);
                break;
        }
    }

    if (f_flag) {
        rotate_logs(LOG_NUM, log_path);
        int flog = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (flog == -1) {
            ALOGE("open %s failed: %s", log_path, strerror(errno));
        }
        retval = read_dev_kmsg(flog);
        close(flog);
    } else {
        retval = read_dev_kmsg(STDIN_FILENO);
    }

    exit(retval);
}

// Rename last_kmsg -> last_kmsg.1 -> ... -> last_kmsg.$max.
// Overwrite any existing last_kmsg.$max.
void rotate_logs(int max, const char *log_path)
{
    for (int i = max-1; i >= 0; --i) {
        std::string old_kmsg = android::base::StringPrintf("%s", log_path);
        if (i > 0) {
          old_kmsg += "." + std::to_string(i);
        }
        std::string new_kmsg = android::base::StringPrintf("%s.%d", log_path, i+1);
        if (rename(old_kmsg.c_str(), new_kmsg.c_str()) != 0) {
            ALOGE("rename %s failed: %s", old_kmsg.c_str(), strerror(errno));
            if (errno == EACCES) {
                // Permission denied, let klogcat exit with status 4.
                ALOGE("klogcat has no permission, just exit with status 4");
                exit(4);
            }
        }
    }
}

int read_dev_kmsg(int fd_out)
{
    unsigned long long bytesWritten = 0;

    // Each read returns one message. We block when there are no
    // more messages (--follow);
    int fd = open("/dev/kmsg", O_RDONLY);
    if (fd == -1) {
        ALOGE("open /dev/kmsg failed: %s", strerror(errno));
    }
    int retval = 0;

    while (1) {
        char msg[8192]; // CONSOLE_EXT_LOG_MAX.
        unsigned long long time_us;
        int facpri, subsystem, pos;
        char *p, *text;
        ssize_t len;

        // kmsg fails with EPIPE if we try to read while the buffer moves under
        // us; the next read will succeed and return the next available entry.
        do {
            len = read(fd, msg, sizeof(msg));
        } while (len == -1 && errno == EPIPE);

        // All reads from kmsg fail if you're on a pre-3.5 kernel.
        if (len == -1 && errno == EINVAL) {
            ALOGE("klogcat now read /dev/kmsg, which is not supported on pre-3.5 kernel, exit with status 2:%s", strerror(errno));
            retval = 2;
            break;
        }
        if (len <= 0) {
            ALOGE("read /dev/kmsg return value less than 0, exit with it:%s", strerror(errno));
            retval = (int)len;
            break;
        }

        msg[len] = 0;

        if (sscanf(msg, "%u,%*u,%llu,%*[^;];%n", &facpri, &time_us, &pos) != 2) {
            continue;
        }

        // Drop extras after end of message text.
        text = msg + pos;
        if ((p = strchr(text, '\n'))) {
            *p = 0;
        }

        // Is there a subsystem? (The ": " is just a convention.)
        p = strstr(text, ": ");
        subsystem = p ? (p - text) : 0;

        // print the syslog facility/priority at the start of each line, timestamps
        bytesWritten += dprintf(fd_out, "<%d>[%5lld.%06lld] ", facpri, time_us/1000000, time_us%1000000);

        if (subsystem) {
            bytesWritten += dprintf(fd_out, "%.*s", subsystem, text);
            text += subsystem;
        }
        bytesWritten += dprintf(fd_out, "%s\n", text);
        if (fd_out != STDIN_FILENO) {
            if (bytesWritten >= LOG_SIZE) {
                fdatasync(fd_out);
                bytesWritten = 0;
                close(fd_out);
                rotate_logs(LOG_NUM, log_path);
                fd_out = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0666);
            }
        }
    }

    close(fd);
    return retval;
}

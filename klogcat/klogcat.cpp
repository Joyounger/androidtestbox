/*
usage: /system/bin/klogcat [-f log_path],
-f参数可选,后面指定自定义的日志路径,如果未指定-f,log_path取默认值/data/misc/logd/kmsg.log
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


static const int LOG_NUM = 10;                                  // the max number of log files we want to store.

void rotate_logs(int max, const char *log_path);
int read_dev_kmsg(const char *log_path);

int main(int argc __unused, char **argv __unused)
{
    int retval;

    const char* optstr = ":f:";
    int opt;
    const char* log_path = "/data/misc/logd/kmsg.log";         // the default path fot kmsg

    while ((opt = getopt(argc, argv, optstr)) != -1) {
        switch (opt) {
            case 'f':
                log_path = optarg;
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

    ALOGD("log_path--%s", log_path);

    // pre_umask is 077, set umask is 022, for group and other user to read.
    umask(S_IWGRP | S_IWOTH);

    rotate_logs(LOG_NUM, log_path);
    retval = read_dev_kmsg(log_path);

    exit(retval);
}

// Rename last_kmsg -> last_kmsg.1 -> ... -> last_kmsg.$max.
// Overwrite any existing last_log.$max and last_kmsg.$max.
void rotate_logs(int max, const char *log_path)
{
    // Logs should only be rotated once.
    static bool rotated = false;
    if (rotated) {
        return;
    }
    rotated = true;

    for (int i = max-1; i >= 0; --i) {
        std::string old_kmsg = android::base::StringPrintf("%s", log_path);
        if (i > 0) {
          old_kmsg += "." + std::to_string(i);
        }
        std::string new_kmsg = android::base::StringPrintf("%s.%d", log_path, i+1);
        if (rename(old_kmsg.c_str(), new_kmsg.c_str()) != 0) {
            ALOGE("rename %s failed: %s", old_kmsg.c_str(), strerror(errno));
        }
    }
}

int read_dev_kmsg(const char *log_path)
{
    int retval = 0;
    int flog = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0666);

    // Each read returns one message. We block when there are no
    // more messages (--follow);
    int fd = open("/dev/kmsg", O_RDONLY);

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

        // print the syslog facility/priority at the start of each line.
        dprintf(flog, "<%d>", facpri);
        // print timestamps
        dprintf(flog, "[%5lld.%06lld] ", time_us/1000000, time_us%1000000);

        if (subsystem) {
            dprintf(flog, "%.*s", subsystem, text);
            text += subsystem;
        }
        dprintf(flog, "%s\n", text);
    }

    close(fd);
    close(flog);
    return retval;
}

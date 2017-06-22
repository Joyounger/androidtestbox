

// g++ -o test test_recovery_rsaverify.cpp -std=c++11 -lssl -lcrypto -Wall
// usage: print sha1 or sha246 digest form ota package
// 输入参数 string str = "ota-package-bad.zip";     // ota升级包包名
/* 
ok: SHA-1 digest: 06ef60613c6c7ceb49e9563e25b9e6b485571e09
文件名称：ota-package-ok.zip
文件大小：98642111 字节
修改时间：2017年6月21日 14:44:45
MD5     ：38E0454AFA747E4E074F78A8C4F6FF85
SHA1    ：61822710D7E970473FE6E4BC78F91467A0BDB802
CRC32   ：4F0A5FBD

fail: SHA-1 digest: 06ef60613c6c7ceb49e9563e25b9e6b485571e09
文件名称：ota-package-bad.zip
文件大小：98642111 字节
修改时间：2017年6月21日 19:01:55
MD5     ：1CD5E4CE66ACAB8C9034DA1493021F9C
SHA1    ：680494E982425CC60D813918817DF31156157B71
CRC32   ：440185F8

*/
 


#include <stdio.h>
#include <algorithm>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <string>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/sha.h>   
#include <openssl/crypto.h>  // OPENSSL_cleanse 

using namespace std;

#define FOOTER_SIZE 6
#define EOCD_HEADER_SIZE 22
static constexpr size_t MiB = 1024 * 1024;

typedef unsigned char   uint8_t;     //无符号8位数

typedef struct MappedRange {
    void* addr;
    size_t length;
} MappedRange;

/*
 * Use this to keep track of mapped segments.
 */
typedef struct MemMapping {
    unsigned char* addr;           /* start of data */
    size_t         length;         /* length of data */

    int            range_count;
    MappedRange*   ranges;
} MemMapping;

static bool sysMapFD(int fd, MemMapping* pMap) {
    assert(pMap != NULL);

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        printf("fstat(%d) failed: %s\n", fd, strerror(errno));
        return false;
    }

    void* memPtr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (memPtr == MAP_FAILED) {
        printf("mmap(%d, R, PRIVATE, %d, 0) failed: %s\n", (int) sb.st_size, fd, strerror(errno));
        return false;
    }

    pMap->addr = (unsigned char*)memPtr;
    pMap->length = sb.st_size;
    pMap->range_count = 1;
    pMap->ranges = (MappedRange*)malloc(sizeof(MappedRange));
    if (pMap->ranges == NULL) {
        printf("malloc failed: %s\n", strerror(errno));
        munmap(memPtr, sb.st_size);
        return false;
    }
    pMap->ranges[0].addr = memPtr;
    pMap->ranges[0].length = sb.st_size;

    return true;
}

int sysMapFile(const char* fn, MemMapping* pMap)
{
    memset(pMap, 0, sizeof(*pMap));

    if (fn && fn[0] == '@') {
        printf("@@@@@@@@\n");
    } else {
        // This is a regular file.
        int fd = open(fn, O_RDONLY);
        if (fd == -1) {
            printf("Unable to open '%s': %s\n", fn, strerror(errno));
            return -1;
        }

        if (!sysMapFD(fd, pMap)) {
            printf("Map of '%s' failed\n", fn);
            close(fd);
            return -1;
        }

        close(fd);
    }
    return 0;
}

static std::string print_sha1(const uint8_t* sha1, size_t len) {
    const char* hex = "0123456789abcdef";
    std::string result = "";
    for (size_t i = 0; i < len; ++i) {
        result.push_back(hex[(sha1[i]>>4) & 0xf]);
        result.push_back(hex[sha1[i] & 0xf]);
    }
    return result;
}

static std::string print_hex(const uint8_t* bytes, size_t len) {
  return print_sha1(bytes, len);
}

int verify_file(unsigned char* addr, size_t length)
{
    printf("stage 2\n");
    //size_t length = 1496;
    unsigned char* footer = addr + length - FOOTER_SIZE;
    if (footer[2] != 0xff || footer[3] != 0xff) {
        printf("footer is wrong\n");
        return -1;
    }
    size_t comment_size = footer[4] + (footer[5] << 8); //size_t comment_size = 1520;
    size_t signature_start = footer[0] + (footer[1] << 8); //size_t signature_start = 1502;
    printf("comment is %zu bytes; signature %zu bytes from end\n",
         comment_size, signature_start);
    if (signature_start > comment_size) {
        printf("signature start: %zu is larger than comment size: %zu\n", signature_start,
             comment_size);
        return -1;
    }
    if (signature_start <= FOOTER_SIZE) {
        printf("Signature start is in the footer");
        return -1;
    }
#define EOCD_HEADER_SIZE 22
    // The end-of-central-directory record is 22 bytes plus any
    // comment length.
    size_t eocd_size = comment_size + EOCD_HEADER_SIZE; //size_t eocd_size = comment_size + EOCD_HEADER_SIZE;
    if (length < eocd_size) {
        printf("not big enough to contain EOCD\n");
        return -1;
    }
    size_t signed_len = length - eocd_size + EOCD_HEADER_SIZE - 2;
    unsigned char* eocd = addr + length - eocd_size;
    // If this is really is the EOCD record, it will begin with the
    // magic number $50 $4b $05 $06.
    if (eocd[0] != 0x50 || eocd[1] != 0x4b ||
        eocd[2] != 0x05 || eocd[3] != 0x06) {
        printf("signature length doesn't match EOCD marker\n");
        return -1;
    }
    for (size_t i = 4; i < eocd_size-3; ++i) {
        if (eocd[i  ] == 0x50 && eocd[i+1] == 0x4b &&
            eocd[i+2] == 0x05 && eocd[i+3] == 0x06) {
            // if the sequence $50 $4b $05 $06 appears anywhere after
            // the real one, minzip will find the later (wrong) one,
            // which could be exploitable.  Fail verification if
            // this sequence occurs anywhere after the real one.
            printf("EOCD marker occurs after start of EOCD\n");
            return -1;
        }
    }
    
    SHA_CTX sha1_ctx;
    SHA1_Init(&sha1_ctx);
    printf("stage 3\n");
    printf("addr--%s\n", addr); //PK
    printf("signed_len--%zu\n", signed_len); // signed_len--18446744073709551590

    double frac = -1.0;
    size_t so_far = 0;
    while (so_far < signed_len) {
        size_t size = std::min(signed_len - so_far, 16 * MiB);

        SHA1_Update(&sha1_ctx, addr + so_far, size);
        so_far += size;

        double f = so_far / (double)signed_len;
        if (f > frac + 0.02 || size == so_far) {
            //ui->SetProgress(f);
            frac = f;
        }
    }
    //while (1) {
    //}
    
    printf("stage 4\n");
    uint8_t sha1[SHA_DIGEST_LENGTH];
    SHA1_Final(sha1, &sha1_ctx);
    printf("SHA-1 digest: %s\n", print_hex(sha1, SHA_DIGEST_LENGTH).c_str());
    int i;
    for (i = 0; i < SHA_DIGEST_LENGTH; i++) {
        //printf("%c", sha1[i]);
    }
    printf("\n");
}
                
bool verify_package(const unsigned char* package_data, size_t package_size)
{
    int err = verify_file(const_cast<unsigned char*>(package_data), package_size);
    return err;
}

void sysReleaseMap(MemMapping* pMap)
{
    int i;
    for (i = 0; i < pMap->range_count; ++i) {
        if (munmap(pMap->ranges[i].addr, pMap->ranges[i].length) < 0) {
            printf("munmap(%p, %d) failed: %s\n",
                 pMap->ranges[i].addr, (int)pMap->ranges[i].length, strerror(errno));
        }
    }
    free(pMap->ranges);
    pMap->ranges = NULL;
    pMap->range_count = 0;
}



int main()
{
    string str = "ota-package-bad.zip";     // ota升级包包名
    const char* path = str.c_str();
    MemMapping map;

    printf("stage 1\n");
    if (sysMapFile(path, &map) != 0) {
        printf("failed to map file\n");
        return -1;
    }

    verify_package(map.addr, map.length);
    sysReleaseMap(&map);
    
    return 0;
}

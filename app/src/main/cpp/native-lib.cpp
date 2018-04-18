#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/bitmap.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

extern "C" {
#include "include/jpeglib.h"
}

typedef uint8_t BYTE;
#define TAG "image "
#define LOGE(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)

#define true 1
#define false 0

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21


/* POINTER defines a generic pointer type */
typedef unsigned char * POINTER;

/* UINT2 defines a two byte word */
//typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;


/* MD5 context. */
typedef struct {
    UINT4 state[4];         /* state (ABCD) */
    UINT4 count[2];  /* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64];       /* input buffer */
} MD5_CTX;

void MD5Init (MD5_CTX *context);
void MD5Update (MD5_CTX *context, unsigned char *input, unsigned int inputLen);
void MD5UpdaterString(MD5_CTX *context,const char *string);
int MD5FileUpdateFile (MD5_CTX *context,char *filename);
void MD5Final (unsigned char digest[16], MD5_CTX *context);
void MDString (char *string,unsigned char digest[16]);
int MD5File (char *filename,unsigned char digest[16]);


static void MD5_memcpy (POINTER output, POINTER input, unsigned int len);
static void MD5Transform (UINT4 state[4], unsigned char block[64]);
static void Encode (unsigned char *output, UINT4 *input, unsigned int len);
static void MD5_memset (POINTER output, int value, unsigned int len);
static void Decode (UINT4 *output, unsigned char *input, unsigned int len);

static unsigned char PADDING[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
*/
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
*/
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
*/
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
 }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
 }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
 }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
 }

/* MD5 initialization. Begins an MD5 operation, writing a new context.
 */
void MD5Init (MD5_CTX *context)          /* context */
{
    context->count[0] = context->count[1] = 0;
    /* Load magic initialization constants.
    */
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
 operation, processing another message block, and updating the
 context.
 */
void MD5Update (MD5_CTX *context, unsigned char *input, unsigned int inputLen)

{
    unsigned int i, index, partLen;

    /* Compute number of bytes mod 64 */
    index = (unsigned int)((context->count[0] >> 3) & 0x3F);

    /* Update number of bits */
    if ((context->count[0] += ((UINT4)inputLen << 3))
        < ((UINT4)inputLen << 3))
        context->count[1]++;
    context->count[1] += ((UINT4)inputLen >> 29);

    partLen = 64 - index;

    /* Transform as many times as possible.
    */
    if (inputLen >= partLen) {
        MD5_memcpy((POINTER)&context->buffer[index], (POINTER)input, partLen);
        MD5Transform (context->state, context->buffer);

        for (i = partLen; i + 63 < inputLen; i += 64)
            MD5Transform (context->state, &input[i]);

        index = 0;
    }
    else
        i = 0;

    /* Buffer remaining input */
    MD5_memcpy((POINTER)&context->buffer[index], (POINTER)&input[i],inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
 the message digest and zeroizing the context.
 */
void MD5Final (unsigned char digest[16], MD5_CTX *context)
{
    unsigned char bits[8];
    unsigned int index, padLen;

    /* Save number of bits */
    Encode (bits, context->count, 8);

    /* Pad out to 56 mod 64.
    */
    index = (unsigned int)((context->count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    MD5Update (context, PADDING, padLen);

    /* Append length (before padding) */
    MD5Update (context, bits, 8);

    /* Store state in digest */
    Encode (digest, context->state, 16);

    /* Zeroize sensitive information.
    */
    MD5_memset ((POINTER)context, 0, sizeof (*context));
}

/* MD5 basic transformation. Transforms state based on block.
 */
static void MD5Transform (UINT4 state[4], unsigned char block[64])
{
    UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    Decode (x, block, 64);

    /* Round 1 */
    FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
    FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
    FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
    FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
    FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
    FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
    FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
    FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
    FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
    FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
    FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    /* Round 2 */
    GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
    GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
    GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
    GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
    GG (d, a, b, c, x[10], S22, 0x2441453); /* 22 */
    GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
    GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
    GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
    GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
    GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
    GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
    GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
    HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
    HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
    HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
    HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
    HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
    HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
    HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
    HH (b, c, d, a, x[ 6], S34, 0x4881d05); /* 44 */
    HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
    HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

    /* Round 4 */
    II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
    II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
    II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
    II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
    II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
    II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
    II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
    II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
    II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
    II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    /* Zeroize sensitive information.
    */
    MD5_memset ((POINTER)x, 0, sizeof (x));
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
 a multiple of 4.
 */
static void Encode (unsigned char *output, UINT4 *input, unsigned int len)
{
    unsigned int i, j;

    for (i = 0, j = 0; j < len; i++, j += 4) {
        output[j] = (unsigned char)(input[i] & 0xff);
        output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
        output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
        output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
    }
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
 a multiple of 4.
 */
static void Decode (UINT4 *output, unsigned char *input, unsigned int len)
{
    unsigned int i, j;

    for (i = 0, j = 0; j < len; i++, j += 4)
        output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
                    (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

/* Note: Replace "for loop" with standard memcpy if possible.
 */

static void MD5_memcpy (POINTER output, POINTER input, unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++)
        output[i] = input[i];
}

/* Note: Replace "for loop" with standard memset if possible.
 */
static void MD5_memset (POINTER output, int value, unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++)
        ((char *)output)[i] = (char)value;
}
/* Digests a string and prints the result.
 */
void MDString (char *string,unsigned char digest[16])
{
    MD5_CTX context;
    unsigned int len = strlen (string);

    MD5Init (&context);
    MD5Update (&context, (unsigned char *)string, len);
    MD5Final (digest, &context);
}
/* Digests a file and prints the result.
 */
int MD5File (const char *filename,unsigned char digest[16])
{
    FILE *file;
    MD5_CTX context;
    int len;
    unsigned char buffer[1024];

    if ((file = fopen (filename, "rb")) == NULL)
        return -1;
    else {
        MD5Init (&context);
        while (len = fread (buffer, 1, 1024, file))
            MD5Update (&context, buffer, len);
        MD5Final (digest, &context);

        fclose (file);
    }
    return 0;
}
void MD5UpdaterString(MD5_CTX *context,const char *string)
{
    unsigned int len = strlen (string);
    MD5Update (context, (unsigned char *)string, len);
}
int MD5FileUpdateFile (MD5_CTX *context,char *filename)
{
    FILE *file;
    int len;
    unsigned char buffer[1024];

    if ((file = fopen (filename, "rb")) == NULL)
        return -1;
    else {
        while (len = fread (buffer, 1, 1024, file))
            MD5Update (context, buffer, len);
        fclose (file);
    }
    return 0;
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_seraphzxz_libjpeg_utils_FileUtils_getMD5(JNIEnv *env, jclass type, jstring path_) {

    const char *path = env->GetStringUTFChars(path_, 0);

    unsigned char digest[16]; //存放结果

    //第一种用法:

    MD5_CTX md5c;
    MD5Init(&md5c); //初始化
//    MD5UpdaterString(&md5c,"你要测试的字符串");
//    MD5FileUpdateFile(&md5c,"你要测试的文件路径");
//    MD5Final(digest,&md5c);

    //第二种用法:
//    MDString("你要测试的字符串",digest); //直接输入字符串并得出结果

    //第三种用法:
    MD5File(path,digest); //直接输入文件路径并得出结果

    int i = 0;
    char szMd5[32] = {0};
    for (i = 0; i < 16; i++) {
        // 最终生成 32 位 ，不足前面补一位 0
        sprintf(szMd5, "%s%02x", szMd5, digest[i]);
    }

    env->ReleaseStringUTFChars(path_, path);

    return env->NewStringUTF(szMd5);
}

const char *PASSWORD = "pw";
long getFileSize(char* filePath);

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_seraphzxz_libjpeg_utils_FileUtils_fileEncrypt__Ljava_lang_String_2Ljava_lang_String_2(
        JNIEnv *env, jclass type, jstring normalFilePath_, jstring encryptFilePath_) {
    const char *normalFilePath = env->GetStringUTFChars(normalFilePath_, 0);
    const char *encryptFilePath = env->GetStringUTFChars(encryptFilePath_, 0);
    int passwordLen = strlen(PASSWORD);
    //读文件指针
    FILE *frp = fopen(normalFilePath, "rb");
    // 写文件指针
    FILE *fwp = fopen(encryptFilePath, "wb");
    if (frp == NULL) {
//        LOGE("没有写权限");
        return JNI_FALSE;
    }
    if (fwp == NULL) {
        return JNI_FALSE;
    }
    // 边读边写边加密
    int buffer;
    int index = 0;
    while ((buffer = fgetc(frp)) != EOF) {
        // write
        fputc(buffer ^ *(PASSWORD + (index % passwordLen)), fwp);  //异或的方式加密
        index++;
    }
    // 关闭文件流
    fclose(fwp);
    fclose(frp);
    env->ReleaseStringUTFChars(normalFilePath_, normalFilePath);
    env->ReleaseStringUTFChars(encryptFilePath_, encryptFilePath);
    return JNI_TRUE;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_seraphzxz_libjpeg_utils_FileUtils_fileDecode__Ljava_lang_String_2Ljava_lang_String_2(
        JNIEnv *env, jclass type, jstring encryptFilePath_, jstring decodeFilePath_) {
    const char *encryptFilePath = env->GetStringUTFChars(encryptFilePath_, 0);
    const char *decodeFilePath = env->GetStringUTFChars(decodeFilePath_, 0);

    int passwordLen = strlen(PASSWORD);
    // 打开文件
    FILE *frp = fopen(encryptFilePath, "rb");
    FILE *fwp = fopen(decodeFilePath, "wb");
    if (frp == NULL) {
        return JNI_FALSE;
    }
    if (fwp == NULL) {
        return JNI_FALSE;
    }
    // 读取文件
    int buffer;
    int index = 0;
    while ((buffer = fgetc(frp)) != EOF) {
        // 写文件
        fputc(buffer ^ *(PASSWORD + (index % passwordLen)), fwp);
        index++;
    }
    // 关闭流
    fclose(fwp);
    fclose(frp);

    env->ReleaseStringUTFChars(encryptFilePath_, encryptFilePath);
    env->ReleaseStringUTFChars(decodeFilePath_, decodeFilePath);

    return JNI_TRUE;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_seraphzxz_libjpeg_utils_FileUtils_fileSplit__Ljava_lang_String_2Ljava_lang_String_2I(JNIEnv *env, jclass type, jstring splitFilePath_,
                                                  jstring suffix_, jint fileNum) {
    const char *splitFilePath = env->GetStringUTFChars(splitFilePath_, 0);
    const char *suffix = env->GetStringUTFChars(suffix_, 0);

    //要分割文件，首先得到分割文件的路径列表，动态申请内存存储路径列表
    char** split_path_list = (char**)malloc(sizeof(char*) * fileNum);

    //得到文件大小
    long file_size = getFileSize((char *) splitFilePath);

    //得到路径字符长度
    int file_path_str_len = strlen(splitFilePath);

    //组合路径
    char file_path[file_path_str_len + 5];
    strcpy(file_path, splitFilePath);
    strtok(file_path, ".");
    strcat(file_path, "_%d");
    strcat(file_path, suffix);

    int i = 0;
    for(; i < fileNum; ++i) {
        //申请单个文件的路径动态内存存储
        split_path_list[i] = (char*)malloc(sizeof(char) * 128);
        //组合单个路径
        sprintf(split_path_list[i], file_path, (i+1));
    }


    //读文件
    FILE* fp = fopen(splitFilePath, "rb");
    if(fp == NULL) {
        return JNI_FALSE;
    }

    //能整除，平均分割
    if(file_size % fileNum) {
        //单个文件大小
        int part_file_size = file_size / fileNum;
        int i = 0;
        //分割多少个文件就分段读多少次
        for(; i < fileNum; i++) {
            //写文件
            FILE* fwp = fopen(split_path_list[i], "wb");
            if(fwp == NULL) {
                return JNI_FALSE;
            }
            int j = 0;
            //单个文件有多大，就读写多少次
            for(; j < part_file_size; j++) {
                //边读边写
                fputc(fgetc(fp), fwp);
            }
            //关闭文件流
            fclose(fwp);
        }
    } else {  //不能整除
        int part_file_size = file_size / (fileNum -1);
        int i = 0;
        for(; i < fileNum - 1; i++) {
            //写文件
            FILE* fwp = fopen(split_path_list[i], "wb");
            if(fwp == NULL) {
                return JNI_FALSE;
            }

            int j = 0;
            for(; j < part_file_size; j++) {
                //边读边写
                fputc(fgetc(fp), fwp);
            }
            //关闭流
            fclose(fwp);
        }
        //剩余部分
        FILE* last_fwp = fopen(split_path_list[fileNum -1], "wb");
        i = 0;
        for(; i < file_size % (fileNum - 1); i++) {
            fputc(fgetc(fp), last_fwp);
        }
        //关闭流
        fclose(last_fwp);
    }
    //关闭文件流
    fclose(fp);
    //释放动态内存
    i = 0;
    for(; i < fileNum; i++) {
        free(split_path_list[i]);
    }
    free(split_path_list);

    env->ReleaseStringUTFChars(splitFilePath_, splitFilePath);
    env->ReleaseStringUTFChars(suffix_, suffix);
    return JNI_TRUE;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_seraphzxz_libjpeg_utils_FileUtils_fileMerge__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2I(JNIEnv *env, jclass type, jstring splitFilePath_, jstring splitSuffix_,
                                                  jstring mergeSuffix_, jint fileNum) {
    const char *splitFilePath = env->GetStringUTFChars(splitFilePath_, 0);
    const char *splitSuffix = env->GetStringUTFChars(splitSuffix_, 0);
    const char *mergeSuffix = env->GetStringUTFChars(mergeSuffix_, 0);

    //1.申请 split 文件路径列表动态内存
    char** split_path_list = (char**)malloc(sizeof(char*) * fileNum);

    //2.组装 split 文件路径
    int split_file_path_len = strlen(splitFilePath);
    int split_file_path_suffix_len = strlen(splitSuffix);
    char split_file_path[split_file_path_len + split_file_path_suffix_len];
    strcpy(split_file_path, splitFilePath);
    strtok(split_file_path, ".");
    strcat(split_file_path, "_%d");
    strcat(split_file_path, splitSuffix);

    //3.组装 merge 文件路径
    int merge_file_path_len = strlen(mergeSuffix);
    char merge_file_path[split_file_path_len + merge_file_path_len];
    strcpy(merge_file_path, splitFilePath);
    strtok(merge_file_path, ".");
    strcat(merge_file_path, mergeSuffix);


    //4.循环得到 split 文件路径列表
    int file_path_str_len = strlen(split_file_path);
    int i = 0;
    for(; i < fileNum; i++) {
        split_path_list[i] = (char*)malloc(sizeof(char) * file_path_str_len);
        sprintf(split_path_list[i], split_file_path, i+1);
    }

    //5.创建并打开merge file
    FILE* merge_fwp = fopen(merge_file_path, "wb");

    //6.边读边写，读多个文件，写入一个文件
    i = 0;
    for(; i < fileNum; i++) {
        FILE* split_frp = fopen(split_path_list[i], "rb");
        if(split_frp == NULL) {
            return JNI_FALSE;
        }
        long part_split_file_size = getFileSize(split_path_list[i]);
        int j = 0;
        for(; j < part_split_file_size; j++) {
            fputc(fgetc(split_frp), merge_fwp);
        }

        //关闭流
        fclose(split_frp);
        //每合并一个文件，就删除它
        remove(split_path_list[i]);
    }
    //关闭文件流
    fclose(merge_fwp);

    //释放动态内存
    i = 0;
    for(; i < fileNum; i++) {
        free(split_path_list[i]);
    }

    free(split_path_list);

    env->ReleaseStringUTFChars(splitFilePath_, splitFilePath);
    env->ReleaseStringUTFChars(splitSuffix_, splitSuffix);
    env->ReleaseStringUTFChars(mergeSuffix_, mergeSuffix);
    return JNI_TRUE;
}

/*获取文件的大小*/
long getFileSize(char* filePath) {
    FILE* fp = fopen(filePath, "rb");
    if(fp == NULL) {
    }
    fseek(fp, 0, SEEK_END);
    return ftell(fp);
}

extern "C"
//图片压缩方法
int generateJPEG(BYTE *data, int w, int h, int quality,
                 const char *outfilename, jboolean optimize) {
    int nComponent = 3;

    struct jpeg_compress_struct jcs;

    struct jpeg_error_mgr jem;

    jcs.err = jpeg_std_error(&jem);

    //为JPEG对象分配空间并初始化
    jpeg_create_compress(&jcs);
    //获取文件信息
    FILE *f = fopen(outfilename, "wb");
    if (f == NULL) {
        return 0;
    }
    //指定压缩数据源
    jpeg_stdio_dest(&jcs, f);
    jcs.image_width = w;//image_width->JDIMENSION->typedef unsigned int
    jcs.image_height = h;

    jcs.arith_code = false;
    //input_components为1代表灰度图，在等于3时代表彩色位图图像
    jcs.input_components = nComponent;
    if (nComponent == 1)
        //in_color_space为JCS_GRAYSCALE表示灰度图，在等于JCS_RGB时代表彩色位图图像
        jcs.in_color_space = JCS_GRAYSCALE;
    else
        jcs.in_color_space = JCS_RGB;

    jpeg_set_defaults(&jcs);
    //optimize_coding为TRUE，将会使得压缩图像过程中基于图像数据计算哈弗曼表，由于这个计算会显著消耗空间和时间，默认值被设置为FALSE。
    jcs.optimize_coding = optimize;
    //为压缩设定参数，包括图像大小，颜色空间
    jpeg_set_quality(&jcs, quality, true);
    //开始压缩
    jpeg_start_compress(&jcs, TRUE);

    JSAMPROW row_pointer[1];//JSAMPROW就是一个字符型指针 定义一个变量就等价于=========unsigned char *temp
    int row_stride;
    row_stride = jcs.image_width * nComponent;
    while (jcs.next_scanline < jcs.image_height) {
        row_pointer[0] = &data[jcs.next_scanline * row_stride];
        //写入数据 http://www.cnblogs.com/darkknightzh/p/4973828.html
        jpeg_write_scanlines(&jcs, row_pointer, 1);
    }

    //压缩完毕
    jpeg_finish_compress(&jcs);
    //释放资源
    jpeg_destroy_compress(&jcs);
    fclose(f);

    return 1;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_seraphzxz_libjpeg_utils_FileUtils_compressBitmap(JNIEnv *env, jclass type, jobject bitmap,
                                                jint width, jint height, jstring fileName,
                                                jint quality) {

    AndroidBitmapInfo infoColor;
    BYTE *pixelColor;
    BYTE *data;
    BYTE *tempData;
    const char *filename = env->GetStringUTFChars(fileName, 0);

    if ((AndroidBitmap_getInfo(env, bitmap, &infoColor)) < 0) {
        LOGE("解析错误");
        return false;
    }

    if ((AndroidBitmap_lockPixels(env, bitmap, (void **) &pixelColor)) < 0) {
        LOGE("加载失败");
        return false;
    }

    BYTE r, g, b;
    int color;
    data = (BYTE *) malloc(width * height * 3);
    tempData = data;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            color = *((int *) pixelColor);
            r = ((color & 0x00FF0000) >>
                                      16);//与操作获得rgb，参考java Color定义alpha color >>> 24 red (color >> 16) & 0xFF
            g = ((color & 0x0000FF00) >> 8);
            b = color & 0X000000FF;

            *data = b;
            *(data + 1) = g;
            *(data + 2) = r;
            data += 3;
            pixelColor += 4;
        }
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    int resultCode = generateJPEG(tempData, width, height, quality, filename, true);

    free(tempData);
    if (resultCode == 0) {
        return false;
    }

    return true;
}




























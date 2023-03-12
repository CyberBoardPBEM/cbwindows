/*
 **********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved. **
 **                                                                  **
 ** License to copy and use this software is granted provided that   **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message     **
 ** Digest Algorithm" in all material mentioning or referencing this **
 ** software or this function.                                       **
 **                                                                  **
 ** License is also granted to make and use derivative works         **
 ** provided that such works are identified as "derived from the RSA **
 ** Data Security, Inc. MD5 Message Digest Algorithm" in all         **
 ** material mentioning or referencing the derived work.             **
 **                                                                  **
 ** RSA Data Security, Inc. makes no representations concerning      **
 ** either the merchantability of this software or the suitability   **
 ** of this software for any particular purpose.  It is provided "as **
 ** is" without express or implied warranty of any kind.             **
 **                                                                  **
 ** These notices must be retained in any copies of any part of this **
 ** documentation and/or software.                                   **
 **********************************************************************
 */

/* Data structure for MD5 (Message Digest) computation */
typedef struct
{
    uint32_t i[2];                   /* number of _bits_ handled mod 2^64 */
    uint32_t buf[4];                                    /* scratch buffer */
    uint8_t in[64];                              /* input buffer */
    uint8_t digest[16];     /* actual digest after MD5Final call */
} MD5_CTX;

void MD5Init(MD5_CTX *mdContext);
void MD5Update(MD5_CTX *mdContext, const void *inBuf, uint32_t inLen);
#if defined(_WIN64)
inline void MD5Update(MD5_CTX* mdContext, const void* inBuf, size_t inLen)
{
    MD5Update(mdContext, inBuf, value_preserving_cast<uint32_t>(inLen));
}
#endif

void MD5Final(MD5_CTX *mdContext);

/*
 **********************************************************************
 ** End of md5.h                                                     **
 ******************************* (cut) ********************************
 */


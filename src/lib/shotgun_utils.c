#include <stdlib.h>
#include "cencode.h"
#include "cdecode.h"

char *
shotgun_base64_encode(const unsigned char *string, double len, size_t *size)
{
   base64_encodestate s;
   char *ret = NULL;
   int retlen[2];

   if ((len < 1) || (!string)) return NULL;

   if (!(ret = malloc(sizeof(char) * ((((len + 2) - ((int)(len + 2) % 3)) / 3) * 4) + 4)))
     return NULL;
   base64_init_encodestate(&s);
   retlen[0] = base64_encode_block((char*)string, len, ret, &s);
   retlen[1] = base64_encode_blockend(ret + retlen[0], &s);
   ret[retlen[0] + retlen[1]] = '\0';
   if (ret[retlen[0] + retlen[1] - 1] == '\n')
     {
        ret[retlen[0] + retlen[1] - 1] = '\0';
        *size = retlen[0] + retlen[1] - 2;
     }
   else
     *size = retlen[0] + retlen[1] - 1;

   return ret;
}

unsigned char *
shotgun_base64_decode(const char *string, int len, size_t *size)
{
   base64_decodestate s;
   unsigned char *ret = NULL;
   int retlen;

   if ((len < 1) || (!string)) return NULL;

   if (!(ret = malloc(sizeof(char) * (int)((double)len / (double)(4 / 3)))))
     return NULL;
   base64_init_decodestate(&s);
   retlen = base64_decode_block((char*)string, len, ret, &s);
   *size = retlen;

   return ret;
}

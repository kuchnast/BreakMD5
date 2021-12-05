 #include <stdio.h>
 #include <string.h>
 #include <openssl/evp.h>

 int main()
 {
     EVP_MD_CTX *mdctx;
     char mess[] = "Test";
     unsigned char md_value[EVP_MAX_MD_SIZE];
     unsigned int md_len, i;

     mdctx = EVP_MD_CTX_new();
     EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
     EVP_DigestUpdate(mdctx, mess, strlen(mess));
     EVP_DigestFinal_ex(mdctx, md_value, &md_len);
     //EVP_MD_CTX_free(mdctx);

     printf("Digest is: ");
     for (i = 0; i < md_len; i++)
         printf("%02x", md_value[i]);
     printf("\n");

     EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
     EVP_DigestUpdate(mdctx, mess, strlen(mess));
     EVP_DigestFinal_ex(mdctx, md_value, &md_len);
     EVP_MD_CTX_free(mdctx);

     printf("Digest is: ");
     for (i = 0; i < md_len; i++)
         printf("%02x", md_value[i]);
     printf("\n");

     exit(0);
 }
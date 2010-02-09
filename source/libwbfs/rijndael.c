/* Rijndael Block Cipher - rijndael.c

   Written by Mike Scott 21st April 1999
   mike@compapp.dcu.ie

   Permission for free direct or derivative use is granted subject 
   to compliance with any conditions that the originators of the 
   algorithm place on its exploitation.  

*/

#include <stdio.h>
#include <string.h>

#define u8 unsigned char       /* 8 bits  */
#define u32 unsigned long       /* 32 bits */
#define u64 unsigned long long

/* rotates x one bit to the left */

#define ROTL(x) (((x)>>7)|((x)<<1))

/* Rotates 32-bit word left by 1, 2 or 3 byte  */

#define ROTL8(x) (((x)<<8)|((x)>>24))
#define ROTL16(x) (((x)<<16)|((x)>>16))
#define ROTL24(x) (((x)<<24)|((x)>>8))

/* Fixed Data */

static u8 InCo[4]={0xB,0xD,0x9,0xE};  /* Inverse Coefficients */

static u8 fbsub[256];
static u8 rbsub[256];
static u8 ptab[256],ltab[256];
static u32 ftable[256];
static u32 rtable[256];
static u32 rco[30];

/* Parameter-dependent data */

int Nk,Nb,Nr;
u8 fi[24],ri[24];
u32 fkey[120];
u32 rkey[120];

static u32 pack(u8 *b)
{ /* pack bytes into a 32-bit Word */
    return ((u32)b[3]<<24)|((u32)b[2]<<16)|((u32)b[1]<<8)|(u32)b[0];
}

static void unpack(u32 a,u8 *b)
{ /* unpack bytes from a word */
    b[0]=(u8)a;
    b[1]=(u8)(a>>8);
    b[2]=(u8)(a>>16);
    b[3]=(u8)(a>>24);
}

static u8 xtime(u8 a)
{
    u8 b;
    if (a&0x80) b=0x1B;
    else        b=0;
    a<<=1;
    a^=b;
    return a;
}

static u8 bmul(u8 x,u8 y)
{ /* x.y= AntiLog(Log(x) + Log(y)) */
    if (x && y) return ptab[(ltab[x]+ltab[y])%255];
    else return 0;
}

static u32 SubByte(u32 a)
{
    u8 b[4];
    unpack(a,b);
    b[0]=fbsub[b[0]];
    b[1]=fbsub[b[1]];
    b[2]=fbsub[b[2]];
    b[3]=fbsub[b[3]];
    return pack(b);    
}

static u8 product(u32 x,u32 y)
{ /* dot product of two 4-byte arrays */
    u8 xb[4],yb[4];
    unpack(x,xb);
    unpack(y,yb); 
    return bmul(xb[0],yb[0])^bmul(xb[1],yb[1])^bmul(xb[2],yb[2])^bmul(xb[3],yb[3]);
}

static u32 InvMixCol(u32 x)
{ /* matrix Multiplication */
    u32 y,m;
    u8 b[4];

    m=pack(InCo);
    b[3]=product(m,x);
    m=ROTL24(m);
    b[2]=product(m,x);
    m=ROTL24(m);
    b[1]=product(m,x);
    m=ROTL24(m);
    b[0]=product(m,x);
    y=pack(b);
    return y;
}

u8 ByteSub(u8 x)
{
    u8 y=ptab[255-ltab[x]];  /* multiplicative inverse */
    x=y;  x=ROTL(x);
    y^=x; x=ROTL(x);
    y^=x; x=ROTL(x);
    y^=x; x=ROTL(x);
    y^=x; y^=0x63;
    return y;
}

void gentables(void)
{ /* generate tables */
    int i;
    u8 y,b[4];

  /* use 3 as primitive root to generate power and log tables */

    ltab[0]=0;
    ptab[0]=1;  ltab[1]=0;
    ptab[1]=3;  ltab[3]=1; 
    for (i=2;i<256;i++)
    {
        ptab[i]=ptab[i-1]^xtime(ptab[i-1]);
        ltab[ptab[i]]=i;
    }
    
  /* affine transformation:- each bit is xored with itself shifted one bit */

    fbsub[0]=0x63;
    rbsub[0x63]=0;
    for (i=1;i<256;i++)
    {
        y=ByteSub((u8)i);
        fbsub[i]=y; rbsub[y]=i;
    }

    for (i=0,y=1;i<30;i++)
    {
        rco[i]=y;
        y=xtime(y);
    }

  /* calculate forward and reverse tables */
    for (i=0;i<256;i++)
    {
        y=fbsub[i];
        b[3]=y^xtime(y); b[2]=y;
        b[1]=y;          b[0]=xtime(y);
        ftable[i]=pack(b);

        y=rbsub[i];
        b[3]=bmul(InCo[0],y); b[2]=bmul(InCo[1],y);
        b[1]=bmul(InCo[2],y); b[0]=bmul(InCo[3],y);
        rtable[i]=pack(b);
    }
}

void gkey(int nb,int nk,char *key)
{ /* blocksize=32*nb bits. Key=32*nk bits */
  /* currently nb,bk = 4, 6 or 8          */
  /* key comes as 4*Nk bytes              */
  /* Key Scheduler. Create expanded encryption key */
    int i,j,k,m,N;
    int C1,C2,C3;
    u32 CipherKey[8];
    
    Nb=nb; Nk=nk;

  /* Nr is number of rounds */
    if (Nb>=Nk) Nr=6+Nb;
    else        Nr=6+Nk;

    C1=1;
    if (Nb<8) { C2=2; C3=3; }
    else      { C2=3; C3=4; }

  /* pre-calculate forward and reverse increments */
    for (m=j=0;j<nb;j++,m+=3)
    {
        fi[m]=(j+C1)%nb;
        fi[m+1]=(j+C2)%nb;
        fi[m+2]=(j+C3)%nb;
        ri[m]=(nb+j-C1)%nb;
        ri[m+1]=(nb+j-C2)%nb;
        ri[m+2]=(nb+j-C3)%nb;
    }

    N=Nb*(Nr+1);
    
    for (i=j=0;i<Nk;i++,j+=4)
    {
        CipherKey[i]=pack((u8 *)&key[j]);
    }
    for (i=0;i<Nk;i++) fkey[i]=CipherKey[i];
    for (j=Nk,k=0;j<N;j+=Nk,k++)
    {
        fkey[j]=fkey[j-Nk]^SubByte(ROTL24(fkey[j-1]))^rco[k];
        if (Nk<=6)
        {
            for (i=1;i<Nk && (i+j)<N;i++)
                fkey[i+j]=fkey[i+j-Nk]^fkey[i+j-1];
        }
        else
        {
            for (i=1;i<4 &&(i+j)<N;i++)
                fkey[i+j]=fkey[i+j-Nk]^fkey[i+j-1];
            if ((j+4)<N) fkey[j+4]=fkey[j+4-Nk]^SubByte(fkey[j+3]);
            for (i=5;i<Nk && (i+j)<N;i++)
                fkey[i+j]=fkey[i+j-Nk]^fkey[i+j-1];
        }

    }

 /* now for the expanded decrypt key in reverse order */

    for (j=0;j<Nb;j++) rkey[j+N-Nb]=fkey[j]; 
    for (i=Nb;i<N-Nb;i+=Nb)
    {
        k=N-Nb-i;
        for (j=0;j<Nb;j++) rkey[k+j]=InvMixCol(fkey[i+j]);
    }
    for (j=N-Nb;j<N;j++) rkey[j-N+Nb]=fkey[j];
}


/* There is an obvious time/space trade-off possible here.     *
 * Instead of just one ftable[], I could have 4, the other     *
 * 3 pre-rotated to save the ROTL8, ROTL16 and ROTL24 overhead */ 

void encrypt(char *buff)
{
    int i,j,k,m;
    u32 a[8],b[8],*x,*y,*t;

    for (i=j=0;i<Nb;i++,j+=4)
    {
        a[i]=pack((u8 *)&buff[j]);
        a[i]^=fkey[i];
    }
    k=Nb;
    x=a; y=b;

/* State alternates between a and b */
    for (i=1;i<Nr;i++)
    { /* Nr is number of rounds. May be odd. */

/* if Nb is fixed - unroll this next 
   loop and hard-code in the values of fi[]  */

        for (m=j=0;j<Nb;j++,m+=3)
        { /* deal with each 32-bit element of the State */
          /* This is the time-critical bit */
            y[j]=fkey[k++]^ftable[(u8)x[j]]^
                 ROTL8(ftable[(u8)(x[fi[m]]>>8)])^
                 ROTL16(ftable[(u8)(x[fi[m+1]]>>16)])^
                 ROTL24(ftable[x[fi[m+2]]>>24]);
        }
        t=x; x=y; y=t;      /* swap pointers */
    }

/* Last Round - unroll if possible */ 
    for (m=j=0;j<Nb;j++,m+=3)
    {
        y[j]=fkey[k++]^(u32)fbsub[(u8)x[j]]^
             ROTL8((u32)fbsub[(u8)(x[fi[m]]>>8)])^
             ROTL16((u32)fbsub[(u8)(x[fi[m+1]]>>16)])^
             ROTL24((u32)fbsub[x[fi[m+2]]>>24]);
    }   
    for (i=j=0;i<Nb;i++,j+=4)
    {
        unpack(y[i],(u8 *)&buff[j]);
        x[i]=y[i]=0;   /* clean up stack */
    }
    return;
}

void decrypt(char *buff)
{
    int i,j,k,m;
    u32 a[8],b[8],*x,*y,*t;

    for (i=j=0;i<Nb;i++,j+=4)
    {
        a[i]=pack((u8 *)&buff[j]);
        a[i]^=rkey[i];
    }
    k=Nb;
    x=a; y=b;

/* State alternates between a and b */
    for (i=1;i<Nr;i++)
    { /* Nr is number of rounds. May be odd. */

/* if Nb is fixed - unroll this next 
   loop and hard-code in the values of ri[]  */

        for (m=j=0;j<Nb;j++,m+=3)
        { /* This is the time-critical bit */
            y[j]=rkey[k++]^rtable[(u8)x[j]]^
                 ROTL8(rtable[(u8)(x[ri[m]]>>8)])^
                 ROTL16(rtable[(u8)(x[ri[m+1]]>>16)])^
                 ROTL24(rtable[x[ri[m+2]]>>24]);
        }
        t=x; x=y; y=t;      /* swap pointers */
    }

/* Last Round - unroll if possible */ 
    for (m=j=0;j<Nb;j++,m+=3)
    {
        y[j]=rkey[k++]^(u32)rbsub[(u8)x[j]]^
             ROTL8((u32)rbsub[(u8)(x[ri[m]]>>8)])^
             ROTL16((u32)rbsub[(u8)(x[ri[m+1]]>>16)])^
             ROTL24((u32)rbsub[x[ri[m+2]]>>24]);
    }        
    for (i=j=0;i<Nb;i++,j+=4)
    {
        unpack(y[i],(u8 *)&buff[j]);
        x[i]=y[i]=0;   /* clean up stack */
    }
    return;
}

void aes_set_key(u8 *key) {
  gentables();
  gkey(4, 4,(char*) key);
}

// CBC mode decryption
void aes_decrypt(u8 *iv, u8 *inbuf, u8 *outbuf, unsigned long long len) {
  u8 block[16];
  unsigned int blockno = 0, i;

  //printf("aes_decrypt(%p, %p, %p, %lld)\n", iv, inbuf, outbuf, len);

  for (blockno = 0; blockno <= (len / sizeof(block)); blockno++) {
    unsigned int fraction;
    if (blockno == (len / sizeof(block))) { // last block
      fraction = len % sizeof(block);
      if (fraction == 0) break;
      memset(block, 0, sizeof(block));
    } else fraction = 16;

    //    debug_printf("block %d: fraction = %d\n", blockno, fraction);
    memcpy(block, inbuf + blockno * sizeof(block), fraction);
    decrypt((char*)block);
    u8 *ctext_ptr;
    if (blockno == 0) ctext_ptr = iv;
    else ctext_ptr = inbuf + (blockno-1) * sizeof(block);
    
    for(i=0; i < fraction; i++) 
      outbuf[blockno * sizeof(block) + i] =
	ctext_ptr[i] ^ block[i];
    //    debug_printf("Block %d output: ", blockno);
    //    hexdump(outbuf + blockno*sizeof(block), 16);
  }
}

// CBC mode encryption      
void aes_encrypt(u8 *iv, u8 *inbuf, u8 *outbuf, unsigned long long len) {
  u8 block[16];
  unsigned int blockno = 0, i;

  //  debug_printf("aes_decrypt(%p, %p, %p, %lld)\n", iv, inbuf, outbuf, len);

  for (blockno = 0; blockno <= (len / sizeof(block)); blockno++) {
    unsigned int fraction;
    if (blockno == (len / sizeof(block))) { // last block
      fraction = len % sizeof(block);
      if (fraction == 0) break;
      memset(block, 0, sizeof(block));
    } else fraction = 16;

    //    debug_printf("block %d: fraction = %d\n", blockno, fraction);
    memcpy(block, inbuf + blockno * sizeof(block), fraction);
        
    for(i=0; i < fraction; i++) 
      block[i] = inbuf[blockno * sizeof(block) + i] ^ iv[i];
    
    encrypt((char*)block);
    memcpy(iv, block, sizeof(block));
    memcpy(outbuf + blockno * sizeof(block), block, sizeof(block));
    //    debug_printf("Block %d output: ", blockno);
    //    hexdump(outbuf + blockno*sizeof(block), 16);
  }
}
      

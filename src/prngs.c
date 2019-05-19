extern unsigned int timestamp;
extern unsigned int last_input_timestampt;
extern unsigned short popup_time;

#define IB1 1
#define IB2 2
#define IB5 16
#define IB18 131072

#define MASK (IB1+IB2+IB5)

unsigned char get_rand_char(unsigned char min, unsigned char max){
    return ((ReadCoreTimer() & 0xff) % (max-min)) + min;
}

unsigned int irbit2(unsigned int iseed)
{
  if (iseed & IB18){
    iseed = ((iseed ^ MASK) << 1) | IB1;
  }
  else{
    iseed <<= 1;
  }
  return iseed;
}

unsigned int quick_rand(unsigned int seed){
    return irbit2(timestamp + seed);
}

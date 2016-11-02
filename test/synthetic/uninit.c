
// catch uninit variables..

#define AB1(A,B)         A##B
#define AB2(A,B)         AB1(A,B)
#define USE_INT_X(STMT)  void AB2(use,__LINE__) (void) { int x; STMT; }

void dostuff(int);

// Bugs
USE_INT_X(dostuff(x))
USE_INT_X(dostuff(2+x))
USE_INT_X(x++)
USE_INT_X(x--)
USE_INT_X(++x)
USE_INT_X(--x)
USE_INT_X(-x)
USE_INT_X(+x)
USE_INT_X(!x)
USE_INT_X(~x)
USE_INT_X(if (x>0))
USE_INT_X(if (x>=0))
USE_INT_X(if (x<=0))
USE_INT_X(if (x<0))
USE_INT_X(if (x==0))
USE_INT_X(if (x==0))
USE_INT_X(for (int a=x; a<10; a++))
USE_INT_X(for (int a=0; a<x; a++))
USE_INT_X(for (int a=0; a<10; a+=x))
USE_INT_X(x=x+1)
USE_INT_X(x=x-1)
USE_INT_X(x=x*1)
USE_INT_X(x=x/1)
USE_INT_X(x=x%1)
USE_INT_X(x=x&1)
USE_INT_X(x=x|1)
USE_INT_X(x=x^1)
USE_INT_X(x=x<<1)
USE_INT_X(x=x>>1)
USE_INT_X(x+=1)
USE_INT_X(x-=1)
USE_INT_X(x*=1)
USE_INT_X(x/=1)
USE_INT_X(x%=1)
USE_INT_X(x&=1)
USE_INT_X(x|=1)
USE_INT_X(x^=1)
USE_INT_X(x<<=1)
USE_INT_X(x>>=1)
USE_INT_X(1?x:1)
USE_INT_X(1&&x)
USE_INT_X(x=*(&x)+1)
USE_INT_X(int*p=&x; dostuff(*p))

// No bugs
USE_INT_X(x=0)
USE_INT_X(0?x:1)
USE_INT_X(0&&x)
USE_INT_X(int*p=&x; *p=0; dostuff(x))



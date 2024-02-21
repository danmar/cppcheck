static int f(int c){
    int b=0; // no issue
    b = (c==0) ? 0 : 1; //misra-c2012-10.6
    return b+c;
}
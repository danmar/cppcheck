void f1() {
   pthread_mutex_lock(m);
   functionCall() ;
   return ; 
}
    
void f2() {
   pthread_mutex_lock(m);
   if (m) {  
        functionCall1() ;
        pthread_mutex_unlock(m);
        return ; 
   } else {  
        functionCall2();
   } 
}

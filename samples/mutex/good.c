void f1() {
   pthread_mutex_lock(m);
   functionCall() ; 
   pthread_mutex_unlock(m) ;
}

int f2() {
   pthread_mutex_lock(m);
   if (n) {
       functionCall() ; 
       pthread_mutex_unlock(m) ;
       return 10; 
   } 
  
   pthread_mutex_unlock(m); 
   return 23; 
} 

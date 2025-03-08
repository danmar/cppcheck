#ifndef MISRA_TEST_H
#define MISRA_TEST_H
struct misra_h_s { int foo; };
bool test(char *a); // OK
int misra_8_2_no_fp(int a);
void misra_8_4_bar(void);
// #12978
typedef struct m8_4_stErrorDef
{
    uint8_t ubReturnVal;
} m8_4_stErrorDef;
extern const m8_4_stErrorDef * m8_4_pubTestPointer;



extern bool get_bool ( void );
extern char_t get_char ( void );
extern int8_t get_int8 ( void );
extern int16_t get_int16 ( void );
extern int32_t get_int32 ( void );
extern int64_t get_int64 ( void );
extern uint8_t get_uint8 ( void );
extern uint16_t get_uint16 ( void );
extern uint32_t get_uint32 ( void );
extern uint64_t get_uint64 ( void );
extern float32_t get_float32 ( void );
extern float64_t get_float64 ( void );
extern float128_t get_float128 ( void );

extern bool *get_bool_ptr ( void );
extern char_t *get_char_ptr ( void );
extern int8_t *get_int8_ptr ( void );
extern int16_t *get_int16_ptr ( void );
extern int32_t *get_int32_ptr ( void );
extern int64_t *get_int64_ptr ( void );
extern uint8_t *get_uint8_ptr ( void );
extern uint16_t *get_uint16_ptr ( void );
extern uint32_t *get_uint32_ptr ( void );
extern uint64_t *get_uint64_ptr ( void );
extern float32_t *get_float32_ptr ( void );
extern float64_t *get_float64_ptr ( void );
extern float128_t *get_float128_ptr ( void );
extern enum_t get_enum ( void );

/* Functions that use a variable */

extern void use_bool ( bool use_bool_param );
extern void use_char ( char_t use_char_param );
extern void use_int8 ( int8_t use_int8_param );
extern void use_int16 ( int16_t use_int16_param );
extern void use_int32 ( int32_t use_int32_param );
extern void use_int64 ( int64_t use_int64_param );
extern void use_int128( int128_t use_int128_param );
extern void use_uint8 ( uint8_t use_uint8_param );
extern void use_uint16 ( uint16_t use_uint16_param );
extern void use_uint32 ( uint32_t use_uint32_param );
extern void use_uint64 ( uint64_t use_uint64_param );
extern void use_uint128 ( uint128_t use_uint128_param );
extern void use_float32 ( float32_t use_float32_param );
extern void use_float64 ( float64_t use_float64_param );
extern void use_float128 ( float128_t use_float128_param );

extern void use_void_ptr ( void * void_ptr_param );
extern void use_bool_ptr ( bool *use_bool_ptr_param );
extern void use_char_ptr ( char_t *use_char_ptr_param );
extern void use_int8_ptr ( int8_t *use_int8_ptr_param );
extern void use_int16_ptr ( int16_t *use_int16_ptr_param );
extern void use_int32_ptr ( int32_t *use_int32_ptr_param );
extern void use_int64_ptr ( int64_t *use_int64_ptr_param );
extern void use_uint8_ptr ( uint8_t *use_uint8_ptr_param );
extern void use_uint16_ptr ( uint16_t *use_uint16_ptr_param );
extern void use_uint32_ptr ( uint32_t *use_uint32_ptr_param );
extern void use_uint64_ptr ( uint64_t *use_uint64_ptr_param );
extern void use_float32_ptr ( float32_t *use_float32_ptr_param );
extern void use_float64_ptr ( float64_t *use_float64_ptr_param );
extern void use_float128_ptr ( float128_t *use_float128_ptr_param );


extern void use_const_char_ptr ( const char_t *use_c_char_ptr_param );


extern void use_enum ( enum_t use_enum_param );
extern void use_sizet ( size_t st );
extern void use_ptrdiff ( ptrdiff_t pt );


#endif // MISRA_TEST_H

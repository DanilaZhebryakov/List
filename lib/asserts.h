#ifndef ASSERTP_H_INCLUDED
#define ASSERTP_H_INCLUDED



#define assert_e(cond) {                                                                                                        \
    if(!(cond)) {                                                                                                               \
        fprintf(stderr,"Assertion failed: %s \nFile:%s \nLine:%d \nFunc:%s\n", #cond, __FILE__, __LINE__, __PRETTY_FUNCTION__); \
        exit(EXIT_FAILURE);                                                                                                     \
    }                                                                                                                           \
}
#define assert_ret(cond, ret) {                                                                                                                   \
    if(!(cond)) {                                                                                                                                \
        fprintf(stderr,"Error: %s \nFile:%s \nLine:%d \nFunc:%s\n Function returns %s\n", #cond, __FILE__, __LINE__, __PRETTY_FUNCTION__, #ret); \
        return ret;                                                                                                                              \
    }                                                                                                                                            \
}

#define assert_retnull(cond) {                                                                                                                   \
    if(!(cond)) {                                                                                                                                \
        fprintf(stderr,"Error: %s \nFile:%s \nLine:%d \nFunc:%s\n Function returns nullptr\n", #cond, __FILE__, __LINE__, __PRETTY_FUNCTION__); \
        return nullptr;                                                                                                                              \
    }                                                                                                                                            \
}
#define assert_ret_err(cond, ret, err) {                                                                                                                   \
    if(!(cond)) {                                                                                                                                \
        fprintf(stderr,"Error: %s \nFile:%s \nLine:%d \nFunc:%s\n Function returns %s\n", #cond, __FILE__, __LINE__, __PRETTY_FUNCTION__, #ret); \
        errno = err;                                                                                                                             \
        return ret;                                                                                                                              \
    }                                                                                                                                            \
}
#endif // ASSERTP_H_INCLUDED

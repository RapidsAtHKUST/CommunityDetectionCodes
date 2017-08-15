

inline void assert_floats(double a, double b, \
                          string error_message=" no message", double precision=1e-7) {
    // checks that a and b are closer than precision
    if(fabs(a-b)>precision) {
        cerr<<"Assertation error: "<<a<<" "<<b<<" "<<error_message<<endl;
        exit(-1);
    }
}

inline void assert_ints(int a, int b, string error_message=" no message") {
    // checks that a and b have the same value
    if(a!=b) {
        cerr<<"Assertation error: "<<a<<" "<<b<<" "<<error_message<<endl;
        exit(-1);
    }
    
}

inline void general_assert(bool to_be_checked, string error_message=" no message") {
    
    if(to_be_checked==false) {
        cerr<<"Assertation error: "<<error_message<<endl;
        exit(-1);
    }
}


#ifndef EXCEPTIONS
#define EXCEPTIONS

namespace ikaros {

    class exception : public std::exception
    {
    public:
        std::string message;
        exception(std::string msg): message(msg) {};
        const char * what () const throw () { return message.c_str(); }
    };

    class fatal_error : public exception 
    {
    public:
        fatal_error(std::string msg) : exception(msg) {}
    };


    class not_a_matrix_element : public exception 
    {
    public:
        not_a_matrix_element(std::string msg) : exception(msg) {}
    };
};

#endif
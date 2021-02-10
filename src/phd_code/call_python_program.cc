#include <cstdlib>

int call_python_program() {
    system("python src/phd_code/average.py");
    return 0;
}

int 
main (int argc, char *argv[])
{
    return call_python_program();
}

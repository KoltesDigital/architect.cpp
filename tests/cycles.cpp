struct A;
struct B;
struct C;
C *f();

struct A
{
    B b;

    void m()
    {
        f();
    }
}

struct B
{
    C c;
}

struct C
{
    A a;
}

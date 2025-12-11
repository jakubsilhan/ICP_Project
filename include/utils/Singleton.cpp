// MUST be at least C++11

class Singleton
{
public:
    static Singleton& getInstance()
    {
        static Singleton instance; // Guaranteed to be destroyed.
        // Instantiated on first use = lazy init.
        return instance;
    }
    void doSomething(void) { std::cout << "Hi\n"; };
    int square(int i) { return i * i; }

private:
    Singleton() {}                               // Hidden constructor 

public:
    Singleton(Singleton const&) = delete;        // no copy constructor
    void operator=(Singleton const&) = delete;   // no assign operator
};


int main(int argc, char* argv[]) {

    // Accessing singleton:
    // a) calling getInstance
    Singleton::getInstance().doSomething();
    std::cout << Singleton::getInstance().square(5) << ‘\n’;

    // b) create variable with reference, can be global variable
    Singleton& smanager = Singleton::getInstance();

    smanager.doSomething();
    std::cout << smanager.square(5) << ‘\n’;

}


#include <iostream>
#include "Log.h"
#import <memory>
#import <string>
#include <functional>
#include "LiteHttpServer.h"
#include <thread>
double my_divide (double x, double y) {return x/y;}

struct MyPair {
    double a,b;
    double multiply() {return a*b;}
};

int testfunc () {
    using namespace std::placeholders;    // adds visibility of _1, _2, _3,...

    // binding functions:
    auto fn_five = std::bind (my_divide,10,2);               // returns 10/2
    std::cout << fn_five() << '\n';                          // 5

    std::function<double()> ff = std::bind(my_divide,10,2);
    std::cout << ff() << std::endl;

    auto fn_half = std::bind (my_divide,_1,2);               // returns x/2
    std::cout << fn_half(10) << '\n';                        // 5

    auto fn_invert = std::bind (my_divide,_2,_1);            // returns y/x
    std::cout << fn_invert(10,2) << '\n';                    // 0.2

    auto fn_rounding = std::bind<int> (my_divide,_1,_2);     // returns int(x/y)
    std::cout << fn_rounding(10,3) << '\n';                  // 3

    MyPair ten_two {10,2};

    // binding members:
    auto bound_member_fn = std::bind (&MyPair::multiply,_1); // returns x.multiply()
    std::cout << bound_member_fn(ten_two) << '\n';           // 20

    auto bound_member_data = std::bind (&MyPair::a,ten_two); // returns ten_two.a
    std::cout << bound_member_data() << '\n';                // 10

    return 0;
}

int main() {
    std::cout << "Hello, World!" << std::endl;
//    test();
//    testfunc();
//    std::shared_ptr<void> p1;
//    std::shared_ptr<std::string> p2 = std::make_shared<std::string>("fff");
//    p1 = p2;
//    std::shared_ptr<std::string> p3 = std::static_pointer_cast<std::string>(p1);
//
//    std::cout << *p3 << p3.use_count() << std::endl;

    std::thread thread1([](){
        LiteHttpServer server("0.0.0.0",8080);
        server.setUpCompleteCallback([](char * data,int data_len,char * name,int name_len){
            int c = 1;
        });
        server.start();
    });

    thread1.join();
    return 0;
}
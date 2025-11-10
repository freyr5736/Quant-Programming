#include <iostream>
#include <math.h>
#include <algorithm>
#include <random>
#include <atomic>
#include <chrono>
#include <thread>


bool check_circle(int x, int y, int r){
    return ((x*x) + (y*y))<= (r*r);
}

// #1 Base Implementation (avg = 40,356,797 ns ≈ 40.36 milliseconds)
// slow implementation
// void simulate(){
//     long double total_points  = 0;
//     long double circle_points = 0;
    
//     std::random_device rd_seed;
//     std::mt19937 gen(rd_seed());
//     std::uniform_real_distribution<> dis(1,960);
//     int r = 480;
//     for(long long i = 1; i<= 100000; ++i){
//         total_points ++;
//         // x - r  to center the circle 
//         long double x = dis(gen) - r;
//         long double y = dis(gen) - r;
//         circle_points += (check_circle(x,y,r)) ? 1 : 0;
//         //long double pi = 4.0 * (circle_points/total_points);
//         //long double error =abs(100 - ((pi/3.14159265359) * 100.00)) ;
//         //  if (i > 1) {
//         //     std::cout << "\033[2F"; // \033[2F moves cursor up two
//         //  }
//         // std::cout<<"pi = "<<pi<<std::endl<<"\% error = "<<error<<std::endl<<std::flush;
//     }
// }

// #2 Thread Implementation (avg = 13,942,939 ns = 13.94 milliseconds)
// using atomic like this creates precision error because of thread updation
void simulate(std::atomic<long double>&  total_points, std::atomic<long double>&  circle_points, const int& r){
    std::random_device rd_seed;
    std::mt19937 gen(rd_seed());
    std::uniform_real_distribution<> dis(1,960);
    for(int i = 1; i<20000;++i){
        total_points = total_points + 1;
        // x - r  to center the circle 
        long double x = dis(gen) - r;
        long double y = dis(gen) - r;
        if(check_circle(x,y,r)){
            circle_points = circle_points + 1;
        }
        //long double pi = 4.0 * (circle_points/total_points);
        //long double error =abs(100 - ((pi/3.14159265359) * 100.00)) ;
        //  if (i > 1) {
        //     std::cout << "\033[2F"; // \033[2F moves cursor up two
        //  }
        // std::cout<<"pi = "<<pi<<std::endl<<"\% error = "<<error<<std::endl<<std::flush;
    }
}


int main (){
    auto start = std::chrono::high_resolution_clock::now();

    //simulate();

    std::atomic<long double>  total_points  = 0;
    std::atomic<long double> circle_points = 0;
    const int r = 480;
    std::vector<std::thread> thread_collection;
    for(long long i = 1; i<= 5; ++i){
        thread_collection.emplace_back(simulate,std::ref (total_points),std::ref(circle_points),std::ref(r));
    }
    for(auto& t : thread_collection){
        t.join();
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = stop - start;
    // auto pi = 4.0 * (circle_points/total_points);
    // auto error =abs(100 - ((pi/3.14159265359) * 100.00)) ;
    // std::cout<<"pi = "<<pi<< "\n error = " <<error<<std::endl;
    std::cout<<"\nduration = "<<std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()<<" nanoseconds";
    return 0;
}
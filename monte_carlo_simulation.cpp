#include <iostream>
#include <math.h>
#include <algorithm>
#include <random>
#include <atomic>
#include <chrono>
#include <thread>
#include "thread_pool.hpp"


bool check_circle(int x, int y, int r){
    return ((x*x) + (y*y))<= (r*r);
}

// #1 Base Implementation
// slow implementation
// (avg π = 3.15329, avg % error = 0.36947%, avg time = 40,356,797 ns ≈ 40.36 milliseconds)

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
//         long double pi = 4.0 * (circle_points/total_points);
//         long double error =abs(100 - ((pi/3.14159265359) * 100.00)) ;
//           if (i > 1) {
//              std::cout << "\033[2F"; // \033[2F moves cursor up two
//           }
//         std::cout<<"pi = "<<pi<<std::endl<<"\% error = "<<error<<std::endl<<std::flush;
//     }
// }

// #2 Thread Implementation 
// using atomic like this creates precision error because of thread updation
// (avg π = 3.44011, avg % error = 9.50206%, avg time = 13,942,939 ns ≈ 13.94 milliseconds)

// void simulate(std::atomic<long double>&  total_points, std::atomic<long double>&  circle_points, const int& r){
//     std::random_device rd_seed;
//     std::mt19937 gen(rd_seed());
//     std::uniform_real_distribution<> dis(1,960);
//     for(int i = 1; i<20000;++i){
//         total_points = total_points + 1;
//         // x - r  to center the circle 
//         long double x = dis(gen) - r;
//         long double y = dis(gen) - r;
//         if(check_circle(x,y,r)){
//             circle_points = circle_points + 1;
//         }
//         //long double pi = 4.0 * (circle_points/total_points);
//         //long double error =abs(100 - ((pi/3.14159265359) * 100.00)) ;
//         //  if (i > 1) {
//         //     std::cout << "\033[2F"; // \033[2F moves cursor up two
//         //  }
//         // std::cout<<"pi = "<<pi<<std::endl<<"\% error = "<<error<<std::endl<<std::flush;
//     }
// }

// #3 Using global variables with threads
// increased precision but no where near close to #1
// (avg π = 3.35014, avg % error = 6.63827%, avg time = 12,539,322 ns ≈ 12.54 milliseconds)

// std::atomic<long double> global_total_points  = 0;
// std::atomic<long double> global_circle_points = 0;
// const int global_r = 480;
// void simulate(){
//     std::random_device rd_seed;
//     std::mt19937 gen(rd_seed());
//     std::uniform_real_distribution<> dis(1,960);
//     for(int i = 1; i<20000;++i){
//         global_total_points = global_total_points + 1;
//         // x - r  to center the circle 
//         long double x = dis(gen) - global_r;
//         long double y = dis(gen) - global_r;
//         if(check_circle(x,y,global_r)){
//             global_circle_points = global_circle_points + 1;
//         }
//         //long double pi = 4.0 * (circle_points/total_points);
//         //long double error =abs(100 - ((pi/3.14159265359) * 100.00)) ;
//         //  if (i > 1) {
//         //     std::cout << "\033[2F"; // \033[2F moves cursor up two
//         //  }
//         // std::cout<<"pi = "<<pi<<std::endl<<"\% error = "<<error<<std::endl<<std::flush;
//     }
// }

// #4 using locks (scoped_lock) and locak variables (avg = 8,309,403 ns ≈ 8.31 milliseconds)
// more precision and faster. But can be faster if called via thread pool
// without thread pool (avg π = 3.15220, avg % error = 0.33760%, avg = 8,317,047 ns ≈ 8.32 milliseconds)
// with thread_pool (avg π = 3.14864, avg % error = 0.23234%, avg time = 442,172 ns ≈ 0.44 milliseconds) 

void simulate(long double total_points, long double circle_points, const int r){
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
    auto pi = 4.0 * (circle_points/total_points);
    auto error =abs(100 - ((pi/3.14159265359) * 100.00)) ;
    std::cout<<"pi = "<<pi<< "\n% error = " <<error<<std::endl;
}


int main (){
    auto start = std::chrono::high_resolution_clock::now();

    // #1
    //simulate();

    // #2
    // std::atomic<long double>  total_points  = 0;
    // std::atomic<long double> circle_points = 0;
    // const int r = 480;
    // std::vector<std::thread> thread_collection;
    // for(long long i = 1; i<= 5; ++i){
    //     thread_collection.emplace_back(simulate,std::ref (total_points),std::ref(circle_points),std::ref(r));
    // }
    // for(auto& t : thread_collection){
    //     t.join();
    // }
    

    // #3
    // std::vector<std::thread> thread_collection;
    // for(long long i=1;i<=5;++i){
    //     thread_collection.emplace_back(simulate);
    // }
    // for(auto& t : thread_collection){
    //     t.join();
    // }

    // #4
    // std::vector<std::thread> thread_collection;
    // for(long long i=1;i<=5;++i){
    //     thread_collection.emplace_back(simulate,0,0,480);
    // }
    // for(auto& t : thread_collection){
    //     t.join();
    // }

    // #4 with threadpool
    thread_pool pool(5);
    for(int i=1;i<=5;++i){
        pool.enqueue([&](){simulate(0,0,480);});
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = stop - start;
    //auto pi = 4.0 * (circle_points/total_points);
    //auto error =abs(100 - ((pi/3.14159265359) * 100.00)) ;
    //std::cout<<"pi = "<<pi<< "\n% error = " <<error<<std::endl;
    std::cout<<"\nduration = "<<std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()<<" nanoseconds"<<std::endl;
    return 0;
}
#include "thread_pool.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <math.h>
#include <random>
#include <thread>

bool check_circle(int x, int y, int r) {
    return ((x * x) + (y * y)) <= (r * r);
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
// passing with reference creates precision error because of thread updation (Conceptually flawed (shared atomic floats)(marked by =>))
// (avg π = 3.44011, avg % error = 9.50206%, avg time = 13,942,939 ns ≈ 13.94 milliseconds)

// void simulate(std::atomic<long double>&  total_points, std::atomic<long
// double>&  circle_points, const int& r){
//     std::random_device rd_seed;
//     std::mt19937 gen(rd_seed());
//     std::uniform_real_distribution<> dis(1,960);
//     for(int i = 1; i<20000;++i){
//         total_points = total_points + 1; // => is not atomic. it does load/add/store, not a single atomic op.
//         // x - r  to center the circle
//         long double x = dis(gen) - r;
//         long double y = dis(gen) - r;
//         if(check_circle(x,y,r)){
//             circle_points = circle_points + 1;   // => is not atomic. it does load/add/store, not a single atomic op.
//         }
//         //long double pi = 4.0 * (circle_points/total_points);
//         //long double error =abs(100 - ((pi/3.14159265359) * 100.00)) ;
//         //  if (i > 1) {
//         //     std::cout << "\033[2F"; // \033[2F moves cursor up two
//         //  }
//         // std::cout<<"pi = "<<pi<<std::endl<<"\% error = "<<error<<std::endl<<std::flush;
//     }
// }

// #3 Using global atomic variables with threads
// increased precision but no where near close to #1
// (avg π = 3.35014, avg % error = 6.63827%, avg time = 12,539,322 ns ≈ 12.54
// milliseconds)

// std::atomic<long double> global_total_points  {0};
// std::atomic<long double> global_circle_points {0};
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

// #4 using locks (scoped_lock) and local variables
// more precision and faster. But can be faster if called via thread pool 
// without thread pool (avg π = 3.15412, avg % error = 0.3987%, avg time = 9,576,662 ns ≈ 9.58 milliseconds) 
// with thread_pool    (avg π = 3.1531,  avg % error = 0.3662%, avg time = 8,809,246 ns ≈ 8.81 milliseconds)

long double global_total_points = 0;
long double global_circle_points = 0;
std::mutex m;
const int global_r = 480;
void simulate(long double total_points, long double circle_points) {
    std::random_device rd_seed;
    std::mt19937 gen(rd_seed());
    std::uniform_real_distribution<> dis(1, 960);
    for (int i = 1; i < 20000; ++i) {
        total_points = total_points + 1;
        // x - r  to center the circle
        long double x = dis(gen) - global_r;
        long double y = dis(gen) - global_r;
        if (check_circle(x, y, global_r)) {
            circle_points = circle_points + 1;
        }
        // long double pi = 4.0 * (circle_points/total_points);
        // long double error =abs(100 - ((pi/3.14159265359) * 100.00)) ;
        //   if (i > 1) {
        //      std::cout << "\033[2F"; // \033[2F moves cursor up two
        //   }
        //  std::cout<<"pi = "<<pi<<std::endl<<"\% error = "<<error<<std::endl<<std::flush;
    }

    {
        std::scoped_lock lock(m);
        global_total_points += total_points;
        global_circle_points += circle_points;
    }
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    // #1
    // simulate();

    // #2
    // std::atomic<long double>  total_points  = 0;
    // std::atomic<long double> circle_points = 0;
    // const int r = 480;
    // std::vector<std::thread> thread_collection;
    // for(long long i = 1; i<= 5; ++i){
    //     thread_collection.emplace_back(simulate,std::ref
    //     (total_points),std::ref(circle_points),std::ref(r));
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
    //     thread_collection.emplace_back(simulate,0,0);
    // }
    // for(auto& t : thread_collection){
    //     t.join();
    // }

    // #4 with threadpool
    thread_pool pool(5);
    for (int i = 1; i <= 5; ++i) {
        pool.enqueue([&]() { simulate(0, 0); });
    }
    pool.wait_for_tasks();

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = stop - start;
    auto pi = 4.0 * (global_circle_points / global_total_points);
    auto error = abs(100 - ((pi / 3.14159265359) * 100.00));
    std::cout << "pi = " << pi << "\n% error = " << error << std::endl;
    std::cout << "\nduration = "<< std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()<< " nanoseconds" << std::endl;
    return 0;
}
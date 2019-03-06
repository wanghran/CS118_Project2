//
//  test.cpp
//  118_proj2_client
//
//  Created by Yunsheng Bai on 3/5/19.
//  Copyright © 2019 Yunsheng Bai. All rights reserved.
//

#include <iostream>
#include <chrono>
#include <thread>



using namespace std;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;

int main() {
    
    timestamp start_time = std::chrono::high_resolution_clock::now();
    
    
    cout << "sleep" << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    

    timestamp current_time = std::chrono::high_resolution_clock::now();
    
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
    
    std::cout << "Program has been running for " << duration << " milliseconds" << std::endl;
    
}

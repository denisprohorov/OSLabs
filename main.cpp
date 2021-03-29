#include <iostream>
#include "buffered_channel.h"
#include <thread>

bool isFinish = false;
BufferedChannel<int> bufferedChannel(10);
void send() {
    int i = 0;
    while(!isFinish){
        bufferedChannel.Send(i++);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
//        bufferedChannel.Send(i++);
}
void pop() {
    bool rez = true;
    while(rez){
        std::pair<int, bool> result = bufferedChannel.Recv();
        std::cout << result.first << "\n";
        rez = result.second;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main() {
    std::thread sendThread(send);
    std::thread popThread(pop);

    std::cin.get();
    isFinish = true;
    bufferedChannel.Close();
    sendThread.join();
    popThread.join();
//bufferedChannel.Send(19);
    return 0;
}

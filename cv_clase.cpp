#include <iostream>          
#include <thread>             
#include <mutex>              
#include <condition_variable> 

std::mutex mtx;
std::condition_variable cv;
bool ready = false;       
int current_turn = 0;    

void print_id (int id) {
  std::unique_lock<std::mutex> lck(mtx);
  cv.wait(lck, [id]() { return ready && (id == current_turn); });
  std::cout << "thread " << id << '\n';
  current_turn++;
  cv.notify_all();
}

void go() {
  std::unique_lock<std::mutex> lck(mtx);
  ready = true;
  cv.notify_all(); 
}

int main ()
{
  std::thread threads[10];
  for (int i=0; i<10; ++i)
    threads[i] = std::thread(print_id, i);

  std::cout << "10 threads ready to race...\n";
  go();                       // go!

  for (auto& th : threads) th.join();

  return 0;
}

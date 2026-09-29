#include <iostream>       // std::cout
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <mutex>
std::mutex mtx;
void pause_thread(int n) 
{
  std::this_thread::sleep_for (std::chrono::seconds(n));
  try{
  std::lock_guard<std::mutex> lck (mtx);
  int ii=7;
  throw (std::logic_error("error"));
  std::cout << "Yo soy: " << n << " seconds ended\n";
  }
  catch (const std::logic_error&){
    std::cout << "[EXCEPTION CAUGHT]\n";
} 
}
 
int main() 
{
  std::cout << "Spawning and detaching 3 threads...\n";
  for(int i=0; i<100; i++){
    std::thread (pause_thread, i).detach();
  }
    std::cout << "Done spawning threads.\n";

  std::cout << "(the main thread will now pause for 5 seconds)\n";
  // give the detached threads time to finish (but not guaranteed!):
  pause_thread(5);
  return 0;
}

#include <iostream>
#include <unistd.h>
int main()
{
  pid_t processId = getpid();
  pid_t parentProcessId = getppid();
  std::cout<<"Process ID : "<<processId<<std::endl;
  std::cout<<"Parent Process ID : "<<parentProcessId<<std::endl;
  
  return 0;
}
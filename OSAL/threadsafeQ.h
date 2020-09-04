
#ifndef THREADSAFEQ_H
#define THREADSAFEQ_H

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable> 
#include <chrono>

template<typename Data>
class threadsafeQ
{
private:
    std::queue<Data> the_queue;
    std::mutex  the_mutex;
    boost::condition_variable the_condition_variable;
public:
    void push(Data const& data)
    {
        std::unique_lock<std::mutex> lock(the_mutex);
        the_queue.push(data);
        lock.unlock();
        the_condition_variable.notify_one();
    }

    bool empty() const
    {
        std::unique_lock<std::mutex> lock(the_mutex);
        return the_queue.empty();
    }

    bool try_pop(Data& popped_value)
    {
        std::unique_lock<std::mutex>  lock(the_mutex);
        if(the_queue.empty())
        {
            return false;
        }
        
        popped_value=the_queue.front();
        the_queue.pop();
        return true;
    }

    void wait_and_pop(Data& popped_value)
    {
        std::unique_lock<std::mutex> lock(the_mutex);
		
		/* Equivalent implementation:
        while(the_queue.empty())
        {
            the_condition_variable.wait(lock);
        }
        */
		
		the_condition_variable.wait(lock, [=](){return !the_queue.empty();});
		
        popped_value=the_queue.front();
        the_queue.pop();
    }

	bool wait_and_pop(unsigned long timeout,Data& popped_value)
    {
        std::unique_lock<std::mutex> lock(the_mutex);
        auto now = std::chrono::steady_clock::now();
		if(the_condition_variable.wait_until(lock, now + std::chrono::microseconds(timeout), [=](){return !the_queue.empty();}))
        {
			popped_value=the_queue.front();
			the_queue.pop();
			return true;
		}
		else 
		{
			return false;
		}
    }

}

#endif


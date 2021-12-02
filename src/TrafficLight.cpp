#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // wait for new messages
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this]
               { return !_queue.empty(); }); // from class examples

    // pull them from the queue
    T msg = std::move(_queue.back());
    _queue.pop_back();
    _queue.clear();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> uLock(_mutex);
    _queue.clear();
    _queue.emplace_back(msg);//(std::move(msg));
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight() {

}

void TrafficLight::waitForGreen()
{
    // block until the light turns green
    while (_queue.receive() != TrafficLightPhase::green)
    {
    }

    return;
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // cycle through traffic light phases
    threads.emplace_back(&TrafficLight::cycleThroughPhases, this);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // ref
    // https://stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c
    // https://www.techiedelight.com/measure-elapsed-time-program-chrono-library/

    // set a random cycle duration between 4 and 6 seconds
    std::random_device device;
    std::mt19937 rng(device());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(4000, 6000); // ms
    auto cycleDuration = dist6(rng);

    std::chrono::time_point<std::chrono::high_resolution_clock> lastUpdate;

    // initialise cycle clock
    lastUpdate = std::chrono::high_resolution_clock::now();

    while (true)
    {
        // wait 1ms between cycles
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time since cycle update
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {

            // toggle current phase of traffic light
            _currentPhase = (_currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red);

            // send an update to the message queue using move semantics
            _queue.send(std::move(_currentPhase));

            // reset cycle clock
            lastUpdate = std::chrono::high_resolution_clock::now();
        }
    }
}
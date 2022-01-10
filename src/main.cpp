#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <cassert>
#include <vector>
#include <mutex>
#include <csignal>
#include <signal.h>

#include <utility>

#include "OtherStructures/FileOperations.hpp"
#include "BreakMD5.hpp"

BreakMD5 app;

/**
 * USAGE: ./BreakMD5 PathToDictionary PathToHashes
 */
void sendSIGHUP(int param)
{
    app.SignalSIGHUP();
} 

int main(int argc, char **argv)
{
    std::vector<std::thread> producer_threads;
    
    FileOperations dict(argv[1], std::ios_base::in);
    FileOperations hash(argv[2], std::ios_base::in);

    /**
    app.PrintDictionary();
    app.PrintHashes();
    app.PrintFound();
    std::cout << std::endl;
    **/

    if(dict.isOpen() && hash.isOpen())
    {
        app.ReadDictionary(dict.f());
        app.ReadHashes(hash.f());
    }
    else
    {
        std::cout << "Can't read dictionary or/and hashes file." << std::endl;
        return 1;
    }
    // ONE WORD PRODUCERS
    producer_threads.push_back(std::thread(&BreakMD5::OneWordProducer, &app, AllToLowercase, AddStringFront, DigitsModification));
    producer_threads.push_back(std::thread(&BreakMD5::OneWordProducer, &app, FirstToUppercase, AddStringFront, DigitsModification));
    producer_threads.push_back(std::thread(&BreakMD5::OneWordProducer, &app, AllToUppercase, AddStringFront, DigitsModification));
    producer_threads.push_back(std::thread(&BreakMD5::OneWordProducer, &app, AllToLowercase, AddStringBack, DigitsModification));
    producer_threads.push_back(std::thread(&BreakMD5::OneWordProducer, &app, FirstToUppercase, AddStringBack, DigitsModification));
    producer_threads.push_back(std::thread(&BreakMD5::OneWordProducer, &app, AllToUppercase, AddStringBack, DigitsModification));
    
    // TWO WORD PRODUCERS
    producer_threads.push_back(std::thread(&BreakMD5::TwoWordProducer, &app, AllToLowercase, AllToLowercase, AddStringsFront, DigitsModification));
    producer_threads.push_back(std::thread(&BreakMD5::TwoWordProducer, &app, AllToLowercase, AllToLowercase, AddStringsMiddle, DigitsModification));
    producer_threads.push_back(std::thread(&BreakMD5::TwoWordProducer, &app, AllToLowercase, AllToLowercase, AddStringsBack, DigitsModification));
    producer_threads.push_back(std::thread(&BreakMD5::TwoWordProducer, &app, FirstToUppercase, AllToLowercase, AddStringsFront, DigitsModification));
    producer_threads.push_back(std::thread(&BreakMD5::TwoWordProducer, &app, FirstToUppercase, AllToLowercase, AddStringsMiddle, DigitsModification));
    producer_threads.push_back(std::thread(&BreakMD5::TwoWordProducer, &app, FirstToUppercase, AllToLowercase, AddStringsBack, DigitsModification));

    std::thread consumer(&BreakMD5::Consumer, &app);
    signal(1, sendSIGHUP);

    //while(app.IsFinished())
    //{
        for (auto &t : producer_threads)
        {
            if(t.joinable())
                t.join();
        }
        consumer.join();
    //}

    return 0;
}

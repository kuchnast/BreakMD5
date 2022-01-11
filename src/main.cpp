#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <cassert>
#include <vector>
#include <mutex>
#include <csignal>
#include <chrono>
#include <signal.h>

#include <utility>

#include "OtherStructures/FileOperations.hpp"
#include "BreakMD5.hpp"
/**
 * USAGE: ./BreakMD5 PathToDictionary PathToHashes
 */
BreakMD5 app;

void sendSIGHUP(int param)
{
    app.SignalSIGHUP();
} 

void makeThreads()
{
    std::vector<std::thread> producer_threads;

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

    // CONSUMER
    std::thread consumer(&BreakMD5::Consumer, &app);

    for (auto &t : producer_threads)
    {
        t.detach();
    }
    consumer.detach();
}

int main(int argc, char **argv)
{
    signal(1, sendSIGHUP);

    FileOperations dict(argv[1], std::ios_base::in);
    FileOperations hash(argv[2], std::ios_base::in);

    std::string line;

    if(argc != 3)
    {
        std::cout << "USAGE: ./BreakMD5 PathToDictionary PathToHashes" << std::endl;
        return 1;
    }

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

    makeThreads();

    while(true)
    {
        using namespace std::chrono_literals;

        std::getline(std::cin, line);
        FileOperations new_hash(line, std::ios_base::in);

        app.Stop();
        while (!app.AllEnded())
            std::this_thread::sleep_for(100ms);
        app.Clear();

        if (new_hash.isOpen())
        {
            app.ReadHashes(new_hash.f());
            std::cout << "New hashes file loaded. Running..." << std::endl;
            makeThreads();
        }
        else
        {
            std::cout << "Can't open this file." << std::endl;
        }
    }

    return 0;
}

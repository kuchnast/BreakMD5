#pragma once

#include "CommandHandler.hpp"
#include "OtherStructures/FileOperations.hpp"
#include "BreakMD5.hpp"
#include "md5.hpp"
#include <iostream>
#include <tuple>
#include <string>
#include <array>
#include <vector>
#include <chrono>
#include <thread>

BreakMD5 app;
size_t num_of_threads = 0;

struct Quitter : CommandHandler
{
public:

    std::string getCommandName() const override
    {
        return "quit";
    }

    void handle(const std::vector<std::string>& parameters) const override
    {
        std::ignore = parameters;
        std::cout << "Console has quitted!" << std::endl;
        exit(0);
    }
};

struct NewHashFile : CommandHandler
{
public:
    std::string getCommandName() const override
    {
        return "newHashFile";
    }

    void handle(const std::vector<std::string> &parameters) const override
    {
        using namespace std::chrono_literals;
        // std::ignore = parameters;
        FileOperations file_in(parameters[0], std::fstream::in);

        app.Stop();
        while(!app.AllEnded())
            std::this_thread::sleep_for(100ms);
        app.Clear();

        if (file_in.isOpen()) {
            
            app.ReadHashes(file_in.f());
            std::cout << "New hashes file loaded.\n";

        } else {
            std::cout << "Can't open file." << std::endl;
        }
    }
};


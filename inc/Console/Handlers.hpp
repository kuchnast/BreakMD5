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

struct Printer : CommandHandler
{
public:

    std::string getCommandName() const override
    {
        
        return "print";
    }

    void handle(const std::vector<std::string>& parameters) const override
    {
        for(auto& p : parameters)
            std::cout << p << " ";
        std::cout<< std::endl;
    }
};

struct LoadNewDictionary : CommandHandler
{
public:
    std::string getCommandName() const override
    {
        return "LoadNewDictionary";
    }

    void handle(const std::vector<std::string> &parameters) const override
    {
    // std::ignore = parameters;
        FileOperations file_in(parameters[0], std::fstream::in);

        if (file_in.isOpen()) {
            app.ReadDictionary(file_in.f());
            std::cout << "Dictionary loaded.\n";

        } else {
            std::cout << "Can't open file." << std::endl;
        }
    }
};


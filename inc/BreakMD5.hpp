#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <list>
#include <mutex>
#include <locale>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <shared_mutex>
#include <condition_variable>

#include "FileOperations.hpp"
#include "md5.hpp"

class BreakMD5
{
public:
    using mutex_type = std::shared_mutex;
    using read_lock = std::shared_lock<mutex_type>;
    using write_lock = std::unique_lock<mutex_type>;

private:
    std::vector<std::string> m_dictionary;
    std::list<std::string> m_hashes;
    std::vector<std::pair<std::string, std::string>> m_found;
    std::atomic_bool m_bFinished;
    std::atomic<size_t> m_started;
    std::atomic_bool m_bSIGHUP;
    mutable mutex_type m_Mutex;
    std::condition_variable_any m_foundCv;

    read_lock lock_for_reading() { return read_lock(m_Mutex); }

    write_lock lock_for_writing() { return write_lock(m_Mutex); }



public:
    BreakMD5() : m_bFinished(false), m_started(0), m_bSIGHUP(false) {}

    void ReadDictionary(std::fstream &strm);

    void ReadHashes(std::fstream &strm);

    void PrintDictionary();

    void PrintHashes();

    void PrintFound();

    void OneWordProducer(void (*ModifyWord)(std::string &), void (*AdditionalCharacters)(std::string &, const std::string &), void (*AdditionalCharacterModification)(std::string &));

    void TwoWordProducer(void (*ModifyWordOne)(std::string &), void (*ModifyWordTwo)(std::string &), std::string (*AdditionalCharacters)(const std::string &, const std::string &, const std::string &), void (*AdditionalCharacterModification)(std::string &));

    void Consumer();

    void SignalSIGHUP();

    void Stop();

    bool AllEnded();

    void Clear();
};

void FirstToUppercase(std::string &str);

void AllToUppercase(std::string &str);

void AllToLowercase(std::string &str);

void AddStringFront(std::string &str, const std::string &dig);

void AddStringBack(std::string &str, const std::string &dig);

std::string AddStringsFront(const std::string &str1, const std::string &str2, const std::string &dig);

std::string AddStringsBack(const std::string &str1, const std::string &str2, const std::string &dig);

std::string AddStringsMiddle(const std::string &str1, const std::string &str2, const std::string &dig);

void DigitsModification(std::string &dig);
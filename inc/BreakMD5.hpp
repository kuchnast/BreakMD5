#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <cassert>
#include <vector>
#include <list>
#include <mutex>
#include <locale>
#include <algorithm>
#include <cctype>
#include <shared_mutex>
#include <condition_variable>
#include <OtherStructures/FileOperations.hpp>

#include <md5.hpp>

void DigitsModification(std::string &dig);

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
    std::atomic_bool m_bSIGHUP;
    mutable mutex_type m_Mutex;
    std::condition_variable_any m_foundCv;

    // returns a scoped lock that allows multiple
    // readers but excludes writers
    read_lock lock_for_reading() { return read_lock(m_Mutex); }

    // returns a scoped lock that allows only
    // one writer and no one else
    write_lock lock_for_writing() { return write_lock(m_Mutex); }



public:

    BreakMD5() : m_bFinished(false), m_bSIGHUP(false) {}

    void ReadDictionary(std::fstream & strm)
    {
        std::string line;

        while (!strm.eof() && strm.good())
        {
            strm >> line;
            m_dictionary.push_back(line);
        }
    }

    void ReadHashes(std::fstream &strm)
    {
        std::string line;

        while (!strm.eof() && strm.good())
        {
            strm >> line;
            m_hashes.push_back(line);
        }
    }

    void PrintDictionary()
    {
        if(m_dictionary.empty())
            std::cout << "Dictionary is empty." << std::endl;
        else
        {
            std::cout << "Dictionary:" << std::endl;
            for(const auto & s : m_dictionary)
            {
                std::cout << s << std::endl;
            }
        }
    }

    void PrintHashes()
    {
        if (m_hashes.empty())
            std::cout << "Hashes list is empty." << std::endl;
        else
        {
            std::cout << "Hashes:" << std::endl;
            for (const auto &s : m_hashes)
            {
                std::cout << s << std::endl;
            }
        }
    }

    void PrintFound()
    {
        if (m_found.empty())
            std::cout << "Found list is empty." << std::endl;
        else
        {
            std::cout << "Found:" << std::endl;
            for (const auto &h : m_found)
            {
                std::cout << h.first << " is hash of \"" << h.second << "\"" << std::endl;
        }
        }
    }

    void OneWordProducer(void (*ModifyWord)(std::string &), void (*AdditionalCharacters)(std::string &, const std::string &), void (*AdditionalCharacterModification)(std::string &))
    {
        std::string hash;
        std::string modification;

        while(!m_bFinished && !m_hashes.empty())
        {
            for (auto d : m_dictionary)
            {
                ModifyWord(d);
                AdditionalCharacters(d, modification);
                hash.assign(md5(d));

                {
                    auto rlock = lock_for_reading(); //make read protection when iterating on hash vector 
                    auto h = m_hashes.begin();
                    while (h != m_hashes.end())
                    {
                        {
                            if ((*h).compare(hash) == 0) // check if password is found
                            {
                                rlock.unlock();
                                auto wlock = lock_for_writing(); //wait until all read operations on hash vector ends to dont corupt other iterators
                                m_found.push_back(std::make_pair(*h, d));
                                h = m_hashes.erase(h); //now pointing to the next element
                                wlock.unlock(); // better performance (?)
                                m_foundCv.notify_one();
                            }
                            else
                            {
                                ++h;
                            }
                        }
                    }
                }
            }
            AdditionalCharacterModification(modification);
        }
    }

    void TwoWordProducer(void (*ModifyWordOne)(std::string &), void (*ModifyWordTwo)(std::string &), std::string (*AdditionalCharacters)(const std::string &, const std::string &, const std::string &), void (*AdditionalCharacterModification)(std::string &))
    {
        std::string hash;
        std::string modification;
        std::string d;

        while (!m_bFinished && !m_hashes.empty())
        {
            for (auto d1 : m_dictionary)
            {
                for (auto d2 : m_dictionary)
                    {
                    ModifyWordOne(d1);
                    ModifyWordTwo(d2);
                    d = AdditionalCharacters(d1, d2, modification);
                    hash.assign(md5(d));
                    {
                        auto rlock = lock_for_reading(); // make read protection when iterating on hash vector
                        auto h = m_hashes.begin();
                        while (h != m_hashes.end())
                        {
                            {
                                if ((*h).compare(hash) == 0) // check if password is found
                                {
                                    rlock.unlock();
                                    auto wlock = lock_for_writing(); // wait until all read operations on hash vector ends to dont corupt other iterators
                                    m_found.push_back(std::make_pair(*h, d));
                                    h = m_hashes.erase(h); // now pointing to the next element
                                    wlock.unlock();        // better performance (?)
                                    m_foundCv.notify_one();
                                }
                                else
                                {
                                    ++h;
                                }
                            }
                        }
                    }
                }
            }
            AdditionalCharacterModification(modification);
        }
    }

    void Consumer()
    {
        size_t i = 0;
        do
        {
            auto wlock = lock_for_writing();
            while(m_found.size() == i && m_bSIGHUP == false)
            {
                m_foundCv.wait(wlock, [&]()
                               { return (m_found.size() != i || m_bSIGHUP != false); });
            }

            if(m_bSIGHUP.load())
            {
                std::cout << "Progress: [" << m_found.size() << '/' << m_found.size() + m_hashes.size() << ']' << std::endl;
                m_bSIGHUP.store(false);
            }

            while (i < m_found.size())
            {
                std::cout << "Hash  " << m_found[i].first << " was made from password \"" << m_found[i].second << "\"" << std::endl;
                ++i;
            }

            if(m_hashes.empty())
                m_bFinished.store(true);

        } while (!m_bFinished);
    }

    void Console()
    {

    }

    void SignalSIGHUP()
    {
        m_bSIGHUP = true;
        m_foundCv.notify_one();
    }

    void Stop()
    {
        m_bFinished.store(true);
    }

    bool IsFinished()
    {
        return m_bFinished.load();
    }

};

void FirstToUppercase(std::string &str)
{
    for (auto &c : str)
    {
        c = std::tolower(c);
    }

    if(!str.empty())
    {
        str.front() = std::toupper(str.front());
    }
}

void AllToUppercase(std::string &str)
{
    for(auto & c: str)
    {
        c = std::toupper(c);
    }
}

void AllToLowercase(std::string &str)
{
    for (auto &c : str)
    {
        c = std::tolower(c);
    }
}

void AddStringFront(std::string &str, const std::string & dig)
{
    str.insert(0, dig);
}

void AddStringBack(std::string &str, const std::string &dig)
{
    str.append(dig);
}

std::string AddStringsFront(const std::string &str1, const std::string &str2, const std::string &dig)
{
    std::string str;
    str.append(dig);
    str.append(str1);
    str.append(str2);
    return str;
}

std::string AddStringsBack(const std::string &str1, const std::string &str2, const std::string &dig)
{
    std::string str;
    str.append(str1);
    str.append(str2);
    str.append(dig);
    return str;
}

std::string AddStringsMiddle(const std::string &str1, const std::string &str2, const std::string &dig)
{
    std::string str;
    str.append(str1);
    str.append(dig);
    str.append(str2);
    return str;
}

void DigitsModification(std::string &dig)
{
    if(dig.empty()) //first run
    {
        dig.push_back('0');
    }
    else    //other runs
    {
        size_t n = dig.size() - 1;
        do
        {
            if (dig[n] >= '0' && dig[n] < '9') // check and just increment
            {
                ++dig[n];
                break;
            }
            else
            {
                dig[n] = '0';
                if(n) //when not first character, increment other
                    --n;
                else  //when first character, add new
                {
                    dig.push_back('0');
                    break;
                }
            }
        } while (true);
    }
}
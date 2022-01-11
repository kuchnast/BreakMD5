#include "BreakMD5.hpp"

void BreakMD5::ReadDictionary(std::fstream &strm)
{
    std::string line;

    if (!m_dictionary.empty())
        m_dictionary.clear();

    while (!strm.eof() && strm.good())
    {
        strm >> line;
        m_dictionary.push_back(line);
    }
}

void BreakMD5::ReadHashes(std::fstream &strm)
{
    std::string line;

    if (!m_hashes.empty())
        m_hashes.clear();

    while (!strm.eof() && strm.good())
    {
        strm >> line;
        m_hashes.push_back(line);
    }
}

void BreakMD5::PrintDictionary()
{
    if (m_dictionary.empty())
        std::cout << "Dictionary is empty." << std::endl;
    else
    {
        std::cout << "Dictionary:" << std::endl;
        for (const auto &s : m_dictionary)
        {
            std::cout << s << std::endl;
        }
    }
}

void BreakMD5::PrintHashes()
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

void BreakMD5::PrintFound()
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

void BreakMD5::OneWordProducer(void (*ModifyWord)(std::string &), void (*AdditionalCharacters)(std::string &, const std::string &), void (*AdditionalCharacterModification)(std::string &))
{
    std::string hash;
    std::string modification;

    m_started++;

    while (!m_bFinished || !m_hashes.empty())
    {
        for (auto d : m_dictionary)
        {
            ModifyWord(d);
            AdditionalCharacters(d, modification);
            hash.assign(md5(d));

            {
                auto wlock = lock_for_writing();
                auto h = m_hashes.begin();
                while (h != m_hashes.end())
                {
                    {
                        if ((*h).compare(hash) == 0) // check if password is found
                        {
                            m_found.push_back(std::make_pair(*h, d));
                            m_hashes.erase(h); // now pointing to the next element
                            h = m_hashes.begin();
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

    m_started--;
}

void BreakMD5::TwoWordProducer(void (*ModifyWordOne)(std::string &), void (*ModifyWordTwo)(std::string &), std::string (*AdditionalCharacters)(const std::string &, const std::string &, const std::string &), void (*AdditionalCharacterModification)(std::string &))
{
    std::string hash;
    std::string modification;
    std::string d;

    m_started++;

    while (!m_bFinished || !m_hashes.empty())
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
                    auto wlock = lock_for_writing(); // make read protection when iterating on hash vector
                    auto h = m_hashes.begin();
                    while (h != m_hashes.end())
                    {
                        {
                            if ((*h).compare(hash) == 0) // check if password is found
                            {
                                m_found.push_back(std::make_pair(*h, d));
                                h = m_hashes.erase(h); // now pointing to the next element
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

    m_started--;
}

void BreakMD5::Consumer()
{
    m_started++;

    size_t i = 0;
    do
    {
        auto wlock = lock_for_writing();
        while (m_found.size() == i && m_bSIGHUP == false)
        {
            m_foundCv.wait(wlock, [&]()
                           { return (m_found.size() != i || m_bSIGHUP != false); });
        }

        if (m_bSIGHUP.load())
        {
            std::cout << "Progress: [" << m_found.size() << '/' << m_found.size() + m_hashes.size() << ']' << std::endl;
            m_bSIGHUP.store(false);
        }

        while (i < m_found.size())
        {
            std::cout << "Hash  " << m_found[i].first << " was made from password \"" << m_found[i].second << "\"" << std::endl;
            ++i;
        }

        if (m_hashes.empty())
            m_bFinished.store(true);

    } while (!m_bFinished);

    m_started--;
}

void BreakMD5::SignalSIGHUP()
{
    m_bSIGHUP = true;
    if (m_started)
        m_foundCv.notify_one();
    else
    {
        std::cout << "Consumer thread is not running." << std::endl;
        m_bSIGHUP = false;
    }
}

void BreakMD5::Stop()
{
    m_bFinished.store(true);
}

bool BreakMD5::AllEnded()
{
    if (m_started.load() > 0)
        return false;
    return true;
}

void BreakMD5::Clear()
{
    if (AllEnded())
    {
        m_found.clear();
        m_bFinished.store(false);
    }
}

void FirstToUppercase(std::string &str)
{
    for (auto &c : str)
    {
        c = std::tolower(c);
    }

    if (!str.empty())
    {
        str.front() = std::toupper(str.front());
    }
}

void AllToUppercase(std::string &str)
{
    for (auto &c : str)
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

void AddStringFront(std::string &str, const std::string &dig)
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
    if (dig.empty()) // first run
    {
        dig.push_back('0');
    }
    else // other runs
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
                if (n) // when not first character, increment other
                    --n;
                else // when first character, add new
                {
                    dig.push_back('0');
                    break;
                }
            }
        } while (true);
    }
}
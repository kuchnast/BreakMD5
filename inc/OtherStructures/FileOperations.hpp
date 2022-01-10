#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <vector>

class FileOperations
{
    private:
        const std::string m_filepath;
        std::ios_base::openmode m_mode;
        std::fstream m_fs;

    public:
        FileOperations() = delete;

        FileOperations(const std::string &filepath, std::ios_base::openmode mode) : m_filepath(filepath), m_mode(mode)
        {
            std::filesystem::create_directories(m_filepath.substr(0, m_filepath.find_last_of('/')));
            m_fs.open(m_filepath, m_mode);
        }

        ~FileOperations() 
        {
            if(isOpen())
                m_fs.close();
        }

        bool isOpen() const
        {
            return m_fs.is_open();
        }

/**
        std::vector<std::string> readFileAsStrings()
        {
            std::vector<std::string> output;
            std::string line;

            while(!m_fs.eof() && m_fs.good())
            {
                m_fs >> line;
                output.push_back(line);
            }

            return output;
        }
        **/

        std::fstream & f()
        {
            return m_fs;
        }
};
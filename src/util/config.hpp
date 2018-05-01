// Part of Asuka utility, internal header
// Copyleft 2018, vczn

#include <array>
#include <fstream>

#include "any.hpp"
#include "noncopyable.hpp"

#pragma once
#ifndef ASUKA_CONFIG_HPP
#define ASUKA_CONFIG_HPP

namespace Asuka
{

// singleton class
class Config : Noncopyable
{
public:
    // You can modify the `kConfigFile` in config.cpp
    static const char* kConfigFile;
public:
    static const std::size_t kPortIndex     = 0;
    static const std::size_t kThreadsIndex  = 1;
    static const std::size_t kUseIndex      = 2;
    static const std::size_t kLogIndex      = 3;
    static const std::size_t kNumberConfig  = 4;

    static const std::array<Any, kNumberConfig> kDefaultConfig;

public:
    static Config& instance();

    short get_port() const;

    int get_threads() const;

    bool get_use_epoll() const;

    std::string get_log_file() const;
private:
    Config();

    void load_config(std::ifstream& fin);
    void parse_line(const std::string& line, std::size_t curLine);
    void load_default_config();

private:
    // short port
    // int threads
    // bool useEpoll 
    // string logFile
    std::array<Any, kNumberConfig> mConfig;
};

} // namespace Asuka

#endif // ASUKA_CONFIG_HPP
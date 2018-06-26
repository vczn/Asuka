#include "config.hpp"

#include <fstream>

#include "util.hpp"

namespace Asuka
{

// You can modify the `kConfigFile` 
// to modify the path of configuration file 
const char* Config::kConfigFile = "./Asuka.conf";

// double-braces required in C++11
const std::array<Any, Config::kNumberConfig> Config::kDefaultConfig
{
{
    Any{ static_cast<short>(8888) },
    Any{ 0 },
    Any{ false },
    Any{ std::string{""} }
}
};

Config& Config::instance()
{
    static Config config;
    return config;
}

std::uint16_t Config::get_port() const
{
    return any_cast<std::uint16_t>(mConfig[kPortIndex]);
}

int Config::get_threads() const
{
    return any_cast<int>(mConfig[kThreadsIndex]);
}

bool Config::get_use_epoll() const
{
    return any_cast<bool>(mConfig[kUseIndex]);
}

std::string Config::get_log_file() const
{
    return any_cast<std::string>(mConfig[kLogIndex]);
}

Config::Config()
{
    std::ifstream fin{ kConfigFile };
    if (fin.fail())
    {
        load_default_config();
    }
    else
    {
        load_config(fin);
    }

}


// details
namespace
{

void parse_whitespace(const std::string& line, std::size_t& idx)
{
    // parse single line, so only judge ' ' and '\t'
    while (line[idx] == ' ' || line[idx] == '\t')
    {
        ++idx;
    }
}

void parse_assign(const std::string& line, std::size_t& idx,
    std::size_t curLine)
{
    parse_whitespace(line, idx);
    if (line[idx++] != '=')
    {
        err_quit("check = at line %zu", curLine);
    }
    parse_whitespace(line, idx);
}

std::string parse_value(const std::string& line, std::size_t& idx,
    const char* key, std::size_t len, std::size_t curLine)
{
    for (std::size_t i = 0; i < len; ++i, ++idx)
    {
        if (key[i] != line[idx])
        {
            err_quit("check %s config key at line %zu", key, curLine);
        }
    }

    parse_assign(line, idx, curLine);

    auto iter = std::find_if(line.begin() + idx, line.end(),
        [](char ch) { return ch == '#' || ch == ' '; });
    if (iter != line.end())
    {
        err_quit("check %s config value at line %zu", key, curLine);
    }
    std::string value = line.substr(idx);

    return value;
}

} // unamed namespace


void Config::load_config(std::ifstream& fin)
{
    // parse config file
    std::size_t curLine = 1;
    for (std::string line; std::getline(fin, line); ++curLine)
    {
        parse_line(line, curLine);
    }

    // fill default config for unconfigured items
    for (std::size_t i = 0; i < mConfig.size(); ++i)
    {
        if (!mConfig[i].has_value())
        {
            mConfig[i] = kDefaultConfig[i];
        }
    }
}

void Config::parse_line(const std::string& line, std::size_t curLine)
{
    std::size_t idx = 0;        // char idx in the line
    std::string value;

    parse_whitespace(line, idx);
    if (idx == line.size())     // whitespace line, skip
    {
        return;
    }

    switch (line[idx])
    {
    case '#':   // comment
        break;
    case 'p':   // port
        value = parse_value(line, idx, "port", 4, curLine);
        mConfig[kPortIndex] = static_cast<std::uint16_t>(std::stoi(value));
        break;
    case 't':   // threads
        value = parse_value(line, idx, "threads", 7, curLine);
        mConfig[kThreadsIndex] = std::stoi(value);
        break;
    case 'u':   // use
    {
        value = parse_value(line, idx, "use", 3, curLine);
        bool useEpoll = false;
        if (value == "epoll")
        {
            useEpoll = true;
        }
        else if (value != "poll")
        {
            err_quit("check use config at line %zu", curLine);
        }

        mConfig[kUseIndex] = useEpoll;
        break;
    }
    case 'l':   // logfile
        value = parse_value(line, idx, "logfile", 7, curLine);
        mConfig[kLogIndex] = value;
        break;
    default:    // error
        err_quit("check config file, include other wrong content at line %zu",
            curLine);
    }
}

void Config::load_default_config()
{
    mConfig[kPortIndex]     = kDefaultConfig[kPortIndex];
    mConfig[kThreadsIndex]  = kDefaultConfig[kThreadsIndex];
    mConfig[kUseIndex]      = kDefaultConfig[kUseIndex];
    mConfig[kLogIndex]      = kDefaultConfig[kLogIndex];
}

} // namespace Asuka
#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

struct Items {
    Items() { };
    ~Items()
    {
        _items.clear();
    };

    Items(const Items& src)
    {
        _items = src._items;
    }

    Items& operator=(const Items& src)
    {
        if (this == &src) {
            return *this;
        }
        _items = src._items;
        return *this;
    }

    std::unordered_map<std::string, std::string> _items;

    std::string operator[](const std::string& key)
    {
        auto it = _items.find(key);
        if (it == _items.end()) {
            return "";
        }
        return it->second;
    }
};

class ConfigManager {
public:
    ~ConfigManager()
    {
        _config_map.clear();
    }

    static ConfigManager& GetInstance()
    {
        static ConfigManager cfg;
        return cfg;
    }
    Items operator[](const std::string& key)
    {
        auto it = _config_map.find(key);
        if (it == _config_map.end()) {
            return Items();
        }
        return it->second;
    }

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

private:
    ConfigManager(const std::string& path)
    {
        fs::path current_path = fs::current_path();
        fs::path config_path = current_path / path;

        boost::property_tree::ptree pt;
        boost::property_tree::read_ini(config_path.string(), pt);

        for (const auto& item : pt) {
            const std::string& item_name = item.first;
            boost::property_tree::ptree ptc = item.second;
            std::unordered_map<std::string, std::string> items;
            for (const auto& [key, value] : ptc) {
                items[key] = value.get_value<std::string>();
            }
            Items item2;
            item2._items = std::move(items);
            _config_map[item_name] = item2;
        }
    }
    ConfigManager()
        : ConfigManager("./config.ini")
    {
    }

private:
    std::unordered_map<std::string, Items> _config_map;
};

#endif

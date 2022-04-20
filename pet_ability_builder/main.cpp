#include <chrono>
#include <cstdio>
#include <stdint.h>
#include <string>
#include <filesystem>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <fmt/core.h>
#include <fmt/color.h>

#include "binary_reader.hpp"
#include "binary_writer.hpp"
#include "utils.hpp"

struct item {
    uint32_t m_id;
    std::string m_name;
};
struct pet_ability {
    uint32_t m_item_id;
    uint8_t m_element;
    std::string m_ability;
    std::string m_cooldown_effect;
    std::chrono::seconds m_cooldown;
    std::string m_suffix;
};

std::vector<item> g_items;
std::vector<pet_ability> g_pet_ability;

enum {
    ELEMENT_TYPE_NONE,
    ELEMENT_TYPE_AIR = 1,
    ELEMENT_TYPE_EARTH = 3,
    ELEMENT_TYPE_FIRE = 5,
    ELEMENT_TYPE_WATER = 7
};
enum {
    READING_LINE_NAME,
    READING_LINE_ABILITY,
    READING_LINE_COOLDOWN_EFFECT,
    READING_LINE_COOLDOWN,
    READING_LINE_SUFFIX,
    READING_LINE_EXTRA_SUFFIX
};

const std::string element_to_string(const uint8_t& ele) {
    switch (ele) {
    case ELEMENT_TYPE_AIR:
        return "air";
    case ELEMENT_TYPE_EARTH:
        return "earth";
    case ELEMENT_TYPE_FIRE:
        return "fire";
    case ELEMENT_TYPE_WATER:
        return "water";
    default:
        return "none";
    }
}

void decode_items() {
    const uint64_t data_size = std::filesystem::file_size("items.dat");
    char* data = (char*)malloc(data_size);
    std::ifstream file("items.dat", std::ios::binary);
    if (file.bad()) {
        fmt::print("failed to open items.dat\n");
        return;
    }
    file.read(data, data_size);
    file.close();
    binary_reader_t br(reinterpret_cast<uint8_t*>(data));
    short version = br.read_short();
    int items_count = br.read_int();
    g_items.resize(items_count);
    for (int i = 0; i < items_count; i++) {
        item item;
        item.m_id = br.read_int();
        br.skip(4);
        item.m_name = br.read_item_name(item.m_id);
        br.read_string();
        br.skip(23);
        br.read_string();
        br.skip(8);
        br.read_string();
        br.read_string();
        br.read_string();
        br.read_string();
        br.skip(24);
        br.read_string();
        br.read_string();
        br.read_string();
        br.skip(80);
        br.read_string();
        br.skip(21);
        g_items.push_back(std::move(item));
    }
    fmt::print("items.dat initailized, version: {}, items count: {}\n", version, items_count);
}
void set_pet_ability_data(const uint8_t& element, const std::string& data) {
    size_t data_header = data.find("|-"),
        data_ending = data.find("|-");
    std::string item_data = data.substr(data_header + 9, data_ending - data_header - 9);
    std::vector<std::string> splited_data = utils::explode("|-", item_data);
    for (std::string& i_data : splited_data) {
        std::vector<std::string> lines = utils::explode("\n", i_data);
        if (lines.size() >= 7)
            continue; //TODO
        std::string name;
        pet_ability pet;
        pet.m_element = element;
        for (auto line = 0; line < lines.size(); line++) {
            const auto& val = lines[line];
            switch (line) {
            case READING_LINE_NAME: {
                if (val.find("{{ItemLink|") != std::string::npos) {
                    size_t name_start = val.find("{{ItemLink|") + 11,
                        name_end = val.find("}}", val.find("{{ItemLink|")) - val.find("{{ItemLink|") - 11;
                    name = val.substr(name_start, name_end);
                    if (name == "Rayman's Fist")
                        lines.resize(5);
                    break;
                }
                break;
            }
            case READING_LINE_ABILITY: {
                if (name.empty())
                    break;
                size_t ability_start = val.find("'''") + 3,
                    ability_end = val.find("'''", ability_start) - ability_start;
                if ((int)ability_start != 4 && (int)ability_end < 0)
                    pet.m_ability = "none";
                else
                    pet.m_ability = val.substr(ability_start, ability_end);
                break;
            }
            case READING_LINE_COOLDOWN_EFFECT: {
                if (lines.size() > 5) {
                    size_t ability_start = val.find("'''") + 3,
                        ability_end = val.find("'''", ability_start) - ability_start;
                    if ((int)ability_start != 4 && (int)ability_end < 0)
                        pet.m_ability = "none";
                    else
                        pet.m_ability = val.substr(ability_start, ability_end);
                    break;
                }
                pet.m_cooldown_effect = val;
                break;
            }
            case READING_LINE_COOLDOWN: {
                if (lines.size() > 5) {
                    pet.m_cooldown_effect = val;
                    break;
                }
                size_t cooldown_start = val.find("'''") + 3,
                    cooldown_end = val.find("s'''", cooldown_start) - cooldown_start;
                if ((int)cooldown_start != 4 && (int)cooldown_end < 0)
                    pet.m_cooldown = std::chrono::seconds(0);
                else
                    pet.m_cooldown = std::chrono::seconds(std::atoi(val.substr(cooldown_start, cooldown_end).c_str()));
                break;
            }
            case READING_LINE_SUFFIX: {
                if (lines.size() > 5) {
                    size_t cooldown_start = val.find("'''") + 3,
                        cooldown_end = val.find("s'''", cooldown_start) - cooldown_start;
                    if ((int)cooldown_start != 4 && (int)cooldown_end < 0)
                        pet.m_cooldown = std::chrono::seconds(0);
                    else
                        pet.m_cooldown = std::chrono::seconds(std::atoi(val.substr(cooldown_start, cooldown_end).c_str()));
                    break;
                }
                pet.m_suffix = val.substr(val.find("|") + 1);
                break;
            }
            case READING_LINE_EXTRA_SUFFIX: {
                if (lines.size() < 6)
                    break;
                pet.m_suffix = val.substr(val.find("|") + 1);
                break;
            }
            default:
                break;
            }
        }
        if (name.empty())
            continue;
        auto it = std::find_if(g_items.begin(), g_items.end(),
            [&name](item item) { return item.m_name == name; });
        if (it == g_items.end())
            continue;
        pet.m_item_id = it->m_id;
        g_pet_ability.push_back(std::move(pet));
    }
}
void get_pet_abilities() {
    httplib::Client client("https://growtopia.fandom.com");
    auto res = client.Get("/wiki/Guide:Pet_Battle_Abilities?action=raw");
    if (res->status != 200)
        return;
    std::string response = res->body;
    size_t element_header = response.find("=="), element_ending = response.find("==");
    std::vector<std::string> splited_element = utils::explode("==", response.substr(element_header + 2, element_ending - element_header - 2));
    for (auto element = 0; element < splited_element.size(); element++) {
        switch (element) {
        case ELEMENT_TYPE_AIR:
	case ELEMENT_TYPE_EARTH:
        case ELEMENT_TYPE_FIRE: 
	case ELEMENT_TYPE_WATER: {
            utils::trim(splited_element[element]);
            set_pet_ability_data(element, splited_element[element]);
            break;
        }
        default:
            break;
        }
    }
}

void output_data() {
    uintmax_t alloc = 14 + (19 * g_pet_ability.size());
    for (const auto& it : g_pet_ability)
        alloc += it.m_ability.size() + it.m_cooldown_effect.size() + it.m_suffix.size();
    auto buffer = new binary_writer_t(alloc);
    buffer->write(std::string("GrowXYZ"));
    buffer->write((int)g_pet_ability.size());
    for (int i = 0; i < g_pet_ability.size(); i++) {
        buffer->write(g_pet_ability[i].m_item_id);
        buffer->write(g_pet_ability[i].m_element);
        buffer->write(g_pet_ability[i].m_ability);
        buffer->write(g_pet_ability[i].m_cooldown_effect);
        buffer->write(g_pet_ability[i].m_cooldown.count());
        buffer->write(g_pet_ability[i].m_suffix);
    }
    FILE* fp = nullptr;
    fopen_s(&fp, std::string("pet_abilities.dat").data(), "wb");
    if (!fp)
        return;
    fwrite(buffer->get(), 1, buffer->get_pos(), fp);
    fflush(fp);
    fclose(fp);
    fmt::print("output file -> pet_abilities.dat\n");
}

int main(){
    decode_items();
    get_pet_abilities();
	output_data();
    while (true);
    return 0;
}

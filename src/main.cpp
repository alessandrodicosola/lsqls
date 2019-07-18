#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
using namespace std::literals::chrono_literals;
#include <filesystem>
#include <algorithm>

#include "byte_string_utility.hpp"
#include "mysql64.hpp"

//TODO move global variables inside function as parameters
//TODO use smart pointer

/* global */
mysql64 *file_to_read = nullptr;
std::filesystem::path current_path;
std::string max_size_formatted;
bool TOKEN_MODE = true;
bool DEBUG_MODE = false;
const unsigned int BUFFER_SIZE = 100 * 1024;

/* declarations */
void usage();
void option_t(const std::vector<std::string> &excluded_tables);
void option_s(uint64_t max_bytes);
void show_progress();

int main(int argc, char **args)
{

    if (argc <= 2)
    {
        usage();
        return 1;
    }

    const std::string filename = std::string(args[1]);
    file_to_read = new mysql64(filename, FILE_MODE_READ, BUFFER_SIZE);

    if (!file_to_read->is_open())
        return 1;

    max_size_formatted = to_bytes_string(file_to_read->size());
    std::filesystem::path temp_path{filename};
    current_path = std::move(temp_path);

    const std::string option = std::string(args[2]);

    if (std::string(args[argc - 1]) == "--dry")
    {
        TOKEN_MODE = true;
        DEBUG_MODE = false;
    }
    else if (std::string(args[argc - 1]) == "--debug")
    {
        DEBUG_MODE = true;
        TOKEN_MODE = false;
    }

    if (option == "-t" || option == "--tables")
    {
        std::vector<std::string> excluded_tables;

        if ((!DEBUG_MODE || !TOKEN_MODE) && argc > 2)
        {
            for (int i = 3; i <= argc - 1; i++)
            {
                excluded_tables.push_back(std::string(args[i]));
            }
        }

        option_t(excluded_tables);
    }
    else if (option == "-s" || option == "--split")
    {
        uint64_t max_bytes = from_bytes_string(std::string(args[3]));

        option_s(max_bytes);
    }
    else
    {
        std::cout << "Expected [-t | --tables ] or [ -s | --split ]"
                  << "\n\n";
        usage();
        return 1;
    }

    delete file_to_read;

    std::cout << " === WORK DONE === " << std::endl;

    return 0;
}

void usage()
{
    const std::string usage[]{
        "lsqls - split large sql file into small files",
        "lsqls FILE [OPTION]",
        "split sql FILE based on OPTION",
        "-t, --tables [EXCLUDED TABLES]\n\tcreate files named with table name used by FILE. Skip EXCLUDED TABLES if setted",
        "-s, --split [SIZE]\n\tsplit sql file into different file with fixed SIZE { digit: # suffix: B,KB,MB,GB,TB format: {#}{suffix} }",
        "--dry\n\tprint statement read from FILE without writing",
        "--debug\n\tprocess FILE without writing to file"};

    std::cout << "NAME"
              << "\n"
              << "\t" << usage[0] << "\n"
              << "SYNOPOSIS"
              << "\n"
              << "\t" << usage[1] << "\n"
              << "DESCRIPTION"
              << "\n"
              << "\t" << usage[2] << "\n"
              << usage[3] << "\n"
              << usage[4] << "\n"
              << usage[5] << "\n"
              << usage[6] << std::endl;
}

void option_t(const std::vector<std::string> &excluded_tables)
{
    statement curr_statement;

    std::string last_table = "";

    std::vector<statement> temp;

    mysql64 *file_to_write = nullptr;

    while (file_to_read->read(curr_statement))
    {
        //avoid high cpu temp caused by 100% usage
        std::this_thread::sleep_for(10ms);

        show_progress();
        if (TOKEN_MODE || DEBUG_MODE)
            std::cout << curr_statement << '\n';

        if (curr_statement.type == statement_type::COMMENT || curr_statement.empty())
            continue;

        if (!TOKEN_MODE)
        {
            if (curr_statement.has_table())
            {

                if (std::find(excluded_tables.cbegin(), excluded_tables.cend(), curr_statement.table) != excluded_tables.cend())
                {
                    if (last_table != curr_statement.table && file_to_write != nullptr)
                        file_to_write->flush();

                    if (file_to_write != nullptr && temp.size() > 0) //for avoid deleting executable comment at the top of the file
                        temp.clear();
                    continue;
                }

                if (last_table != curr_statement.table)
                {
                    std::filesystem::path path_to_write{current_path.remove_filename().string() + curr_statement.table + ".sql"};
                    if (file_to_write != nullptr)
                        file_to_write->flush();
                    file_to_write = new mysql64(path_to_write.string(), FILE_MODE_WRITE, BUFFER_SIZE);
                }

                if (temp.size() > 0)
                {
                    if (DEBUG_MODE)
                        std::cout << "writing temp data to " << file_to_write->filename() << '\n';
                    else
                        std::for_each(temp.cbegin(), temp.cend(), [&](const statement &statement) { file_to_write->write(statement); });

                    temp.clear();
                }

                if (DEBUG_MODE)
                    std::cout << "writing to " << file_to_write->filename() << '\n';
                else
                    file_to_write->write(curr_statement);

                last_table = curr_statement.table;
            }
            else
            {
                if (std::find(excluded_tables.cbegin(), excluded_tables.cend(), last_table) != excluded_tables.cend())
                {
                    continue;
                }
                else
                {
                    if (curr_statement.type == statement_type::UNLOCK)
                    {
                        if (DEBUG_MODE)
                            std::cout << "writing unlock." << '\n';
                        else
                            file_to_write->write(curr_statement);
                        continue;
                    }
                    else
                    {
                        temp.push_back(curr_statement);
                    }
                }
            }
        }
    }

    if (file_to_write != nullptr)
        delete file_to_write;
}

void option_s(uint64_t max_bytes)
{
    mysql64 *file_to_write = nullptr;

    uint64_t current_bytes = 0;
    statement curr_statement;
    std::string last_table;

    int index = 1;

    const auto filename = current_path.stem();
    const auto dir = std::filesystem::path(current_path).remove_filename();
    auto now_dir(dir / filename);

    if (!std::filesystem::exists(now_dir))
        std::filesystem::create_directory(now_dir);

    auto now_file = filename.string() + "." + std::to_string(index) + ".sql";
    auto now_path(now_dir / now_file);

    while (file_to_read->read(curr_statement))
    {
        std::this_thread::sleep_for(10ms);

        if (!file_to_write && !DEBUG_MODE)
            file_to_write = new mysql64(now_path, FILE_MODE_WRITE, 1024 * 8);

        if (curr_statement.type == statement_type::COMMENT || curr_statement.empty())
            continue;

        if (DEBUG_MODE)
            std::cout << "writing statement size: " << curr_statement.line.size() << " into file: " << now_path << '\n';
        else
            file_to_write->write(curr_statement);

        current_bytes += curr_statement.line.size();

        if (current_bytes > max_bytes)
        {
            current_bytes = 0;
            index++;

            now_file = filename.string() + "." + std::to_string(index) + ".sql";
            now_path = now_dir / now_file;
            if (!DEBUG_MODE && file_to_write)
            {
                file_to_write->flush();
                delete file_to_write;
                file_to_write = nullptr;
            }
        }
    }

    /* ensure all data are written on disk */
    if (file_to_write)
    {
        file_to_write->flush();
        delete file_to_write;
        file_to_write = nullptr;
    }
}

void show_progress()
{
    std::cout << to_bytes_string(file_to_read->position()) << "-" << max_size_formatted << "\n";
}

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
using namespace std::literals::chrono_literals;
#include <filesystem>
#include <cmath>

#include "mysql64.hpp"

/* global */
mysql64 *SQL64_FILE = nullptr;
std::filesystem::path current_path;
std::string max_size_formatted;
bool TOKEN_MODE = true;
bool DEBUG_MODE = false;
/* 100KB of buffer should be enough for Friends table which contains a lot of data for each line */
const unsigned int BUFFER_SIZE = 200 * 1024;

/* declarations */
void usage();
void option_t(const std::vector<std::string> &excluded_tables);
void option_s(int number_of_lines);
void show_progress();

const std::string format_bytes(const uint64_t value);

int main(int argc, char **args)
{

    if (argc <= 2)
    {
        usage();
        return 1;
    }

    const std::string filename = std::string(args[1]);
    SQL64_FILE = new mysql64(filename, FILE_MODE_READ, BUFFER_SIZE);

    if (!SQL64_FILE->is_open())
        return 1;

    max_size_formatted = format_bytes(SQL64_FILE->size());
    std::filesystem::path temp_path{filename};
    current_path = std::move(temp_path);

    const std::string option = std::string(args[2]);

    if (option == "-t" || option == "--tables")
    {
        std::vector<std::string> excluded_tables;

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
        else
        {
            TOKEN_MODE = false;
            DEBUG_MODE = false;

            if (argc > 2)
            {
                for (int i = 3; i <= argc - 1; i++)
                {
                    excluded_tables.push_back(std::string(args[i]));
                }
            }
        }
        option_t(excluded_tables);
    }
    else if (option == "-s" || option == "--split")
    {
        option_s(0);
    }
    else
    {
        std::cout << "Expected [-t | --tables ] or [ -s | --split ]"
                  << "\n\n";
        usage();
        return 1;
    }

    return 0;
}

void usage()
{
    const std::string usage[]{
        "lsqls - split large sql file into small files",
        "lsqls FILE [OPTION]",
        "split sql FILE based on OPTION",
        "-t, --tables [EXCLUDED TABLES]\n\tcreate files named with table name used by FILE. Skip EXCLUDED TABLES if setted",
        "-s, --split [NUMBER OF LINE]\n\tsplit sql file into different file with fixed number of lines",
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
              << usage[5] << std::endl;
}

void option_t(const std::vector<std::string> &excluded_tables)
{
    statement curr_statement;

    std::string last_table = "";

    std::vector<statement> temp;

    mysql64 *file_to_write = nullptr;

    while (SQL64_FILE->read(curr_statement))
    {
        //avoid high cpu temp caused by 100% usage
        std::this_thread::sleep_for(10ms);

        show_progress();
        if (TOKEN_MODE || DEBUG_MODE)
            std::cout << curr_statement << '\n';

        if (curr_statement.type == statement_type::COMMENT)
            continue;

        if (curr_statement.line == "\n" || curr_statement.line.empty())
            continue;

        if (!TOKEN_MODE)
        {
            if (curr_statement.has_table())
            {

                if (std::find(excluded_tables.cbegin(), excluded_tables.cend(), curr_statement.table) != excluded_tables.cend())
                {
                    temp.clear();
                    continue;
                }

                std::filesystem::path path_to_write{current_path.remove_filename().string() + curr_statement.table + ".sql"};

                if (last_table != curr_statement.table)
                {
                    if (file_to_write != nullptr)
                        file_to_write->flush();
                    file_to_write = new mysql64(path_to_write.string(), FILE_MODE_WRITE, BUFFER_SIZE);
                }

                if (temp.size() > 0)
                {
                    if (DEBUG_MODE)
                        std::cout << "writing temp data to " << path_to_write.string() << '\n';
                    else
                        std::for_each(temp.cbegin(), temp.cend(), [&](const statement &statement) { file_to_write->write(statement); });

                    temp.clear();
                }

                if (DEBUG_MODE)
                    std::cout << "writing to " << path_to_write.string() << '\n';
                else
                    file_to_write->write(curr_statement);

                last_table = curr_statement.table;
            }
            else
            {
                if (std::find(excluded_tables.cbegin(), excluded_tables.cend(), last_table) != excluded_tables.cend())
                {
                    temp.clear();
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

void option_s(int number_of_lines)
{
    throw std::runtime_error("not implemented");
}

void show_progress()
{
    std::cout << format_bytes(SQL64_FILE->current_position()) << "-" << max_size_formatted << "\n";
}

const std::string format_bytes(const uint64_t value)
{

    static const char *prefixes[5] = {"B", "KB", "MB", "GB", "TB"};
    if (value == 0)
        return "0B";
    if (value < 1024)
        return std::to_string(value) + "B";

    int exponent = log10(value) / 3;
    double return_value = value / pow(1024, exponent);

    char *out = new char[20];
    sprintf(out, "%3.2f%s", return_value, prefixes[exponent]);
    return out;
}

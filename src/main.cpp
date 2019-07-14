#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "mysql64.hpp"

#include <thread>
#include <chrono>

#include <filesystem>
#include <cinttypes>

using namespace std::literals::chrono_literals;

/* debug */
const bool IS_DEBUG = true;

/* global */
mysql64 *SQL64_FILE = nullptr;
std::filesystem::path current_path;

/* 100KB of buffer should be enough for Friends table which contains a lot of data for each line */
const unsigned int BUFFER_SIZE = 100 * 1024;

/* declarations */
void usage();
void option_t(const std::vector<std::string> &excluded_tables);
void option_s(int number_of_lines);
void show_progress();

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

    /* save file path */
    std::filesystem::path temp_path{filename};
    std::swap(current_path, temp_path);

    const std::string option = std::string(args[2]);

    if (option == "-t" || option == "--tables")
    {
        std::vector<std::string> excluded_tables;
        if (argc > 2)
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
        "lsqls - split large sql file into small files", "lsqls FILE [OPTION]",
        "split sql FILE based on OPTION",
        "-t, --tables [EXCLUDED TABLES]\n\tcreate files named with table name "
        "used by FILE. Skip EXCLUDED TABLES if setted",
        "-s, --split [NUMBER OF LINE]\n\tsplit sql file into different file with "
        "fixed number of lines"};

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
              << usage[4] << std::endl;
}

void option_t(const std::vector<std::string> &excluded_tables)
{
    statement curr_statement;

    statement_type last_statement_type = statement_type::NONE;
    std::string last_table = "";

    std::vector<statement> temp;

    mysql64 *file_to_write = nullptr;

    while (SQL64_FILE->read(curr_statement))
    {
        //avoid high cpu temp caused by 100% usage
        std::this_thread::sleep_for(10ms);

        if (IS_DEBUG)
            std::cout << curr_statement << '\n';
        else
        {

            show_progress();

            if (curr_statement.type == statement_type::COMMENT)
                continue;

            if (curr_statement.line == "\n" || curr_statement.line.empty())
                continue;

            if (curr_statement.has_table())
            {

                if (std::find(excluded_tables.cbegin(), excluded_tables.cend(), curr_statement.table) != excluded_tables.cend())
                {
                    temp.clear();
                    continue;
                }

                std::filesystem::path path_to_write{current_path.remove_filename().string() + curr_statement.table + ".sql"};

                if (last_table != curr_statement.table)
                    new mysql64(path_to_write.string(), FILE_MODE_APPEND, BUFFER_SIZE);

                if (temp.size() > 0)
                {
                    std::for_each(temp.cbegin(), temp.cend(), [&](const statement &statement) { file_to_write->write(statement); });
                    temp.clear();
                }

                file_to_write->write(curr_statement);

                last_table = curr_statement.table;
            }
            else
            {
                if (curr_statement.type == statement_type::UNLOCK)
                {
                    file_to_write->write(curr_statement);
                    continue;
                }

                if (std::find(excluded_tables.cbegin(), excluded_tables.cend(), last_table) != excluded_tables.cend())
                {
                    temp.clear();
                    continue;
                }
                else
                    temp.push_back(curr_statement);
            }
        }

        delete file_to_write;
    }
}

void option_s(int number_of_lines)
{
    throw std::runtime_error("not implemented");
}

void show_progress()
{
    int percentage = (int)(SQL64_FILE->current_position() / SQL64_FILE->size()) * 100;
    std::cout << SQL64_FILE->current_position() << " - " << SQL64_FILE->size() << " [" << percentage << "]\n";
}

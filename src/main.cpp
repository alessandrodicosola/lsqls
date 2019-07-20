#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
using namespace std::literals::chrono_literals;
#include <algorithm>
#include <filesystem>
#include <memory>

#include "mysql64.hpp"

bool TOKEN_MODE = false;
bool DEBUG_MODE = false;
const unsigned int BUFFER_SIZE = 100 * 1024;

/* declarations */
void usage();
void option_t(const std::unique_ptr<mysql64> &ptr,
              const std::vector<std::string> &excluded_tables);
void option_s(const std::unique_ptr<mysql64> &, uint64_t max_bytes);
void show_progress(const std::unique_ptr<mysql64> &ptr,
                   const std::string &max_size);

int main(int argc, char **args) {
  if (argc <= 2) {
    usage();
    return 1;
  }

  const std::string filename = std::string(args[1]);
  const auto file_to_read =
      std::make_unique<mysql64>(filename, FILE_MODE_READ, BUFFER_SIZE);

  if (!file_to_read->is_open())
    return 1;

  const std::string option = std::string(args[2]);

  if (std::string(args[argc - 1]) == "--dry") {
    TOKEN_MODE = true;
    DEBUG_MODE = false;
  } else if (std::string(args[argc - 1]) == "--debug") {
    DEBUG_MODE = true;
    TOKEN_MODE = false;
  }

  if (option == "-t" || option == "--tables") {
    std::vector<std::string> excluded_tables;

    if ((!DEBUG_MODE || !TOKEN_MODE) && argc > 2) {
      for (int i = 3; i <= argc - 1; i++) {
        excluded_tables.push_back(std::string(args[i]));
      }
    }

    option_t(file_to_read, excluded_tables);
  } else if (option == "-s" || option == "--split") {
    const uint64_t max_bytes =
        utility::byte::bytes_from_string(std::string(args[3]));

    option_s(file_to_read, max_bytes);
  } else {
    std::cout << "Expected [-t | --tables ] or [ -s | --split ]"
              << "\n\n";
    usage();
    return 1;
  }

  std::cout << " === WORK DONE === " << std::endl;

  return 0;
}

void usage() {
  const std::string usage[]{
      "NAME\n\tlsqls - split large sql file into small files",
      "SYNOPSIS\n\tlsqls FILE [OPTION]",
      "DESCRIPTION\n\tsplit sql FILE based on OPTION",
      "\t-t, --tables [EXCLUDED TABLES]\n\tcreate files named with table name ",
      "\tused by FILE. Skip EXCLUDED TABLES if setted",
      "\t-s, --split [SIZE]\n\tsplit sql file into different file with fixed "
      "\tSIZE { digit: # suffix: B,KB,MB,GB,TB format: {#}{suffix} }",
      "\t--dry\n\tprint statement read from FILE without writing",
      "\t--debug\n\tprocess FILE without writing to file"};

  std::for_each(std::begin(usage), std::end(usage),
                [](const std::string &line) { std::cout << line << '\t'; });
}

void option_t(const std::unique_ptr<mysql64> &file_to_read,
              const std::vector<std::string> &excluded_tables) {
  const std::string max_size =
      utility::byte::bytes_to_string(file_to_read->size());
  const auto current_path = std::filesystem::path(file_to_read->filename());

  statement curr_statement;

  std::string last_table = "";

  std::vector<statement> temp;

  std::unique_ptr<mysql64> file_to_write = nullptr;

  while (file_to_read->read(curr_statement)) {
    // avoid high cpu temp caused by 100% usage
    std::this_thread::sleep_for(10ms);

    show_progress(file_to_read, max_size);

    if (TOKEN_MODE || DEBUG_MODE)
      std::cout << curr_statement << '\n';

    if (curr_statement.type == statement_type::COMMENT ||
        curr_statement.empty())
      continue;

    if (!TOKEN_MODE) {
      if (curr_statement.has_table()) {
        if (std::find(excluded_tables.cbegin(), excluded_tables.cend(),
                      curr_statement.table) != excluded_tables.cend()) {
          if (last_table != curr_statement.table && file_to_write)
            file_to_write->flush();

          if (file_to_write && !temp.empty()) // for avoid deleting executable
            // comment at the top of the file
            temp.clear();
          continue;
        }

        if (last_table != curr_statement.table) {
          std::filesystem::path path_to_write{current_path.parent_path() /
                                              (curr_statement.table + ".sql")};
          if (!DEBUG_MODE) {
            if (file_to_write) {
              file_to_write->flush();

              file_to_write.reset(new mysql64(path_to_write.string(),
                                              FILE_MODE_WRITE, BUFFER_SIZE));
            }
          }
        }

        if (!temp.empty()) {
          if (DEBUG_MODE && file_to_write)
            std::cout << "writing temp data to " << file_to_write->filename()
                      << '\n';
          else
            std::for_each(temp.cbegin(), temp.cend(),
                          [&](const statement &statement) {
                            if (file_to_write)
                              file_to_write->write(statement);
                          });

          temp.clear();
        }

        if (DEBUG_MODE)
          std::cout << "writing statement" << '\n';
        else
          file_to_write->write(curr_statement);

        last_table = curr_statement.table;
      } else {
        if (std::find(excluded_tables.cbegin(), excluded_tables.cend(),
                      last_table) != excluded_tables.cend()) {
          continue;
        }
        if (curr_statement.type == statement_type::UNLOCK) {
          if (DEBUG_MODE)
            std::cout << "writing unlock." << '\n';
          else
            file_to_write->write(curr_statement);
        } else {
          temp.push_back(curr_statement);
        }
      }
    }
  }
}

void option_s(const std::unique_ptr<mysql64> &file_to_read,
              uint64_t max_bytes) {
  std::unique_ptr<mysql64> file_to_write = nullptr;

  uint64_t current_bytes = 0;
  statement curr_statement;
  std::string last_table;

  int index = 1;
  const auto current_path = std::filesystem::path(file_to_read->filename());
  const auto filename = current_path.stem();
  const auto dir = std::filesystem::path(current_path).remove_filename();
  const auto now_dir(dir / filename);

  if (!exists(now_dir))
    create_directory(now_dir);

  auto now_file = filename.string() + "." + std::to_string(index) + ".sql";
  auto now_path(now_dir / now_file);

  while (file_to_read->read(curr_statement)) {
    std::this_thread::sleep_for(10ms);

    if (!file_to_write && !DEBUG_MODE)
      file_to_write = std::make_unique<mysql64>(now_path.string(),
                                                FILE_MODE_WRITE, 1024 * 8);

    if (curr_statement.type == statement_type::COMMENT ||
        curr_statement.empty())
      continue;

    if (DEBUG_MODE)
      std::cout << "writing statement size: " << curr_statement.line.size()
                << " into file: " << now_path << '\n';
    else
      file_to_write->write(curr_statement);

    current_bytes += curr_statement.line.size();

    if (current_bytes > max_bytes) {
      current_bytes = 0;
      index++;

      now_file = filename.string() + "." + std::to_string(index) + ".sql";
      now_path = now_dir / now_file;
      if (!DEBUG_MODE && file_to_write) {
        file_to_write->flush();

        file_to_write.reset();
      }
    }
  }

  /* ensure all data are written on disk */
  if (file_to_write) {
    file_to_write->flush();
    file_to_write.reset();
  }
}

void show_progress(const std::unique_ptr<mysql64> &file_to_read,
                   const std::string &max_size) {
  std::cout << utility::byte::bytes_to_string(file_to_read->position()) << "-"
            << max_size << "\n";
}

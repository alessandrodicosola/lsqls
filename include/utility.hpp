#ifndef __STRING_UTILITY__
#define __STRING_UTILITY__

#include <string>
#include <cmath>
#include <stdexcept>
#include <array>
#include <sstream>
#include <cstdarg>
namespace utility
{
	inline void __throw(const char* fmt,...)
	{
		std::string container;
		container.reserve(1024);
		va_list args;
		va_start(args, fmt);
		vsprintf(&container[0], fmt, args);
		va_end(args);

		throw std::runtime_error(container);
	}

	inline void __throw_with_perror(const char* fmt, ...)
	{
		std::string container;
		container.reserve(1024);
		va_list args;
		va_start(args, fmt);
		vsprintf(&container[0], fmt, args);
		va_end(args);

		perror(container.c_str());
		throw std::runtime_error(container);
	}

	namespace byte
	{
		static constexpr size_t _kb = 1024;
		static constexpr size_t _mb = _kb * 1024;
		static constexpr size_t _gb = _mb * 1024;

		template <size_t N_power_of_1024>
		constexpr auto make_suffixes()
		{
			std::array<uint64_t, N_power_of_1024 + 1> out = { 0 };  //TODO: add = {0};
			for (uint64_t i = 0; i < N_power_of_1024 + 1; i++)
			{
				out[i] = static_cast<size_t>(pow(1024, i));
			}
			return out;
		}

		static const auto suffixes = make_suffixes<3>();



		inline const std::string bytes_to_string(const uint64_t value)
		{
			std::string suffix;
			uint64_t return_value;
			if (value > _gb)
			{
				suffix = "GB";
				return_value = value / suffixes.at(3);
			}
			else if (value > _mb)
			{
				suffix = "MB";
				return_value = value / suffixes.at(2);
			}
			else if (value > _kb)
			{
				suffix = "KB";
				return_value = value / suffixes.at(1);
			}
			else
			{
				suffix = "B";
				return_value = value / suffixes.at(0);
			}

			return std::to_string(return_value) + suffix;
		}

		inline const uint64_t bytes_from_string(const std::string& str)
		{
			std::istringstream input(str);
			size_t digit;
			std::string suffix;

			bool flag1 = static_cast<bool>(input >> digit);
			bool flag2 = static_cast<bool>(input >> suffix);

			if (!flag1 && !flag2)
			{
				char* msg = new char[50];
				sprintf(msg, "invalid format. expected:{digit}{suffix}, found: %s", str.c_str());
				throw std::runtime_error(msg);
			}

			if (suffix == "GB")
				return digit * suffixes.at(3);
			else if (suffix == "MB")
				return digit * suffixes.at(2);
			else if (suffix == "KB")
				return digit * suffixes.at(1);
			else
				return digit * suffixes.at(0);
		}



	}
}



#endif
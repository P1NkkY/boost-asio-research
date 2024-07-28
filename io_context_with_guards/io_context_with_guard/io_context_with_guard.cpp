#define _WIN32_WINNT 0x0A00
#include <iostream>
#include <mutex>
#include <vector>

#include <boost/asio.hpp>

inline static std::atomic_int tidgen = 0;
thread_local static int const thread_id = tidgen++;

/**
* @brief parseLine - word parser
* @param input - input(entered) string
* @return parsed words
*/
static std::vector<std::string> parseLine(std::string& input)
{
	std::vector<std::string> result;
	std::string word;

	for (auto& sym : input) {
		if ((sym >= 'a' && sym <= 'z') || (sym >= 'A' && sym <= 'Z')) {
			word.push_back(sym);
		}
		else {
			if (!word.empty()) {
				result.push_back(word);
				word.clear();
			}
		}
	}

	if (!word.empty())
		result.push_back(word);

	return result;
}

int main()
{
	try
	{
		boost::asio::io_context ioc;
		auto work = boost::asio::make_work_guard(ioc); //!< fake work
		
		// We can create a lot of threads to post output string
		std::thread thr1([&ioc] { ioc.run(); });
		std::thread thr2([&ioc] { ioc.run(); });
		
		// infinitive input 
		for (std::string input; getline(std::cin, input); ) {
			// avoid program termination
			if (input.empty())
				break;

			for (std::string& token : parseLine(input)) {
				// Lambda-function
				auto handler = [text = std::move(token)]() {
						std::string output = "THREAD_ID = " + std::to_string(thread_id) + "; text = " + text + "\n";
						std::cout << output;
					};
				// Submits a completion token or function object for execution.
				boost::asio::post(ioc.get_executor(), handler);
			};
		}

		work.reset();
		thr1.join();
		thr2.join();
	}
	catch (const std::exception& e) //!< catch exceptions
	{
		std::cerr << e.what() << std::endl;
	}
	catch (...) //!< catch unexpected error
	{
		std::cerr << "eAn unexpected error has happened" << std::endl;
	}
}

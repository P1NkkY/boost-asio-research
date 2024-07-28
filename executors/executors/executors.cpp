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
		/* Commit the line below if you want boost::asio::thread_pool::executor_type
		* instead of boost::asio::system_executor 
		*/
		#define system_exec

		#ifdef system_exec
			// The system executor represents an execution context where functions 
			// are permitted to run on arbitrary threads
			boost::asio::system_executor ex;
		#else
			// The thread pool class is an execution context where functions are permitted
			// to run on one of a fixed number of threads.
			boost::asio::thread_pool ioc(3);
			auto ex = ioc.get_executor();
		#endif

		// infinitive input 
		for (std::string input; getline(std::cin, input); ) {
			// avoid program termination
			if (input.empty())
				break;

			for (std::string& token : parseLine(input)) {
				// Lambda-function
				auto handler = [text = std::move(token)]() {
						std::mutex gMutex;
						const std::lock_guard lock(gMutex);
						std::string output = "THREAD_ID = " + std::to_string(thread_id) + "; text = " + text + "\n";
						std::cout << output;
						std::this_thread::sleep_for(std::chrono::milliseconds(2000));
					};
				// Submits a completion token or function object for execution.
				boost::asio::post(ex, handler); 
			}
		}

		ex.context().join();
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

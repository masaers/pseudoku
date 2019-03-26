#ifndef COM_MASAERS_TIMER_HPP
#define COM_MASAERS_TIMER_HPP
#include <chrono>
#include <iomanip>

namespace com_masaers {
	class timer {
		using Clock = std::chrono::high_resolution_clock;
		bool running_m;
		Clock::time_point mark_m;
		Clock::duration total_m;
	public:
		inline timer() : running_m(false), mark_m(), total_m(0) {}
		inline timer(const timer&) = default;
		inline timer(timer&&) = default;
		inline timer& operator=(const timer&) = default;
		inline timer& operator=(timer&&) = default;
		inline timer& operator+=(const timer& x) {
			total_m += x.total_m;
			if (x.running_m) {
				total_m += Clock::now() - x.mark_m;
			}
			return *this;
		}
		inline timer operator+(const timer& x) const {
			timer result(x);
			result.stop();
			result += *this;
			return result;
		}
		inline void start() {
			if (! running_m) {
				mark_m = Clock::now();
				running_m = true;
			}
		}
		inline void stop() {
			if (running_m) {
				total_m += Clock::now() - mark_m;
				running_m = false;
			}
		}
		inline uint64_t ns() const {
			uint64_t result = std::chrono::duration_cast<std::chrono::nanoseconds>(total_m).count();
			if (running_m) {
				result += std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - mark_m).count();
			}
			return result;
		}
		template<typename OStream>
		friend OStream& operator<<(OStream& ostream, const timer& t) {
			uint64_t ns = t.ns();
			uint64_t us = ns / 1000; ns = ns % 1000;
			uint64_t ms = us / 1000; us = us % 1000;
			uint64_t s = ms / 1000; ms = ms % 1000;
			uint64_t m = s / 60; s = s % 60;
			uint64_t h = m / 60; m = m % 60;
			ostream << std::setfill('0');
			if (h != 0) {
				ostream << h << ':' << std::setw(2) << m << ':';
			} else if (m != 0) {
				ostream << std::setw(2) << m << ':';
			}
			ostream << s;
			if (ms != 0 || us != 0 || ns != 0) {
				ostream << '.' << std::setw(3) << ms;
				if (us != 0 || ns != 0) {
					ostream << '\'' << std::setw(3) << us;
					if (ns != 0) {
						ostream << '\'' << std::setw(3) << ns;
					}
				}
			}
			return ostream;
		}
	}; // timer
} // namespace com_masaers

#endif

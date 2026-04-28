#include <iostream>
#include <chrono>
#include <string>

namespace kitty_ecs {
  class ScopedTimer {
    public:
      ScopedTimer(const std::string& name) : m_Name(name) {
        m_StartTimePoint = std::chrono::high_resolution_clock::now();
      }

      ~ScopedTimer() {
        Stop();
      }

      void Stop() {
        auto endTimePoint = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

        auto duration = end - start;
        double ms = duration * 0.001;

        std::cout << "\n\r\033[K" << m_Name << " took " << duration << "us (" << ms << "ms)\033[A\r";
      }

    private:
      std::string m_Name;
      std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimePoint;
  };
}

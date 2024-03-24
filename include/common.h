#pragma once
#include <random>

[[nodiscard]] static int RandomRange(int min, int max) noexcept {
  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution<> distr(min, max);
  return distr(eng);
}

[[nodiscard]] static bool CompareString(const std::string& str1,
                                 const std::string& str2) noexcept {
  if (str1.size() != str2.size()) {
    return false;
  }
  for (size_t i = 0; i < str1.size(); ++i) {
    if (str1[i] != str2[i]) {
      return false;
    }
  }
  return true;
}

[[nodiscard]] static std::string ToLower(const std::string& str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return result;
}
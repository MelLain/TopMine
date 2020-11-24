// Author: Murat Apishev (@mel-lain)

#include "boost/algorithm/string.hpp"

#include "include/batch.h"

void Batch::add_document(const std::string& src_document) {
  std::vector<std::string> parts;
  boost::split(parts, src_document, boost::is_any_of(delimiters.c_str()));

  if (parts.size() < 2) {
    throw std::runtime_error("Error: empty or incomplete document string: " + src_document);
  }

  long id = std::stol(parts[0]);
  std::vector<std::string> tokens;

  for (int i = 1; i < parts.size(); ++i) {
    auto& token = parts[i];
    if (token.empty()) {
      continue;
    }

    tokens.push_back(token);
  }

  if (tokens.empty()) {
    throw std::runtime_error("Error: empty or incomplete document string-2: " + src_document);
  }

  documents_.push_back({ id, tokens });
}

void Batch::add_document(long id, const std::vector<std::string>& tokens) {
  if (tokens.empty()) {
    throw std::runtime_error("Error: empty document with id " + std::to_string(id));
  }

  documents_.push_back({ id, tokens });
}

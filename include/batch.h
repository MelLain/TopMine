// Author: Murat Apishev (@mel-lain)

#pragma once

#include <string>
#include <vector>

struct Document {
  long id;
  std::vector<std::string> tokens;
};

class Batch {
 public:
  explicit Batch(const std::string& delimiters) : delimiters(delimiters) { }

  void add_document(const std::string& src_document);
  void add_document(long id, const std::vector<std::string>& tokens);

  const std::vector<Document>& get_documents() const { return documents_; }

  int size() const { return documents_.size(); }

  const std::string delimiters;

 private:
  std::vector<Document> documents_;
};

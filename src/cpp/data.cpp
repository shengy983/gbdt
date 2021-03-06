// Author: qiyiping@gmail.com (Yiping Qi)

#include "data.hpp"
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>


namespace gbdt {
static const std::string kItemDelimiter = " ";
static const std::string kKVDelimiter = ":";

std::string Tuple::ToString() const {
  if (feature == NULL)
    return std::string();

  std::string result = boost::lexical_cast<std::string>(label);
  result += kItemDelimiter;
  result += boost::lexical_cast<std::string>(weight);

  size_t n = g_conf.number_of_feature;
  for (size_t i = 0; i < n; ++i) {
    if (feature[i] == kUnknownValue)
      continue;
    result += kItemDelimiter;
    result += boost::lexical_cast<std::string>(i);
    result += kKVDelimiter;
    result += boost::lexical_cast<std::string>(feature[i]);
  }

  return result;
}

Tuple* Tuple::FromString(const std::string &l) {
  Tuple* result = new Tuple();
  size_t n = g_conf.number_of_feature;
  result->feature = new ValueType[n];
  for (size_t i = 0; i < n; ++i) {
    result->feature[i] = kUnknownValue;
  }

  std::vector<std::string> tokens;
  if (SplitString(l, kItemDelimiter, &tokens) < 2) {
    delete result;
    return NULL;
  }

  result->label = boost::lexical_cast<ValueType>(tokens[0]);
  result->weight = boost::lexical_cast<ValueType>(tokens[1]);

  // for two-class classifier, labels should be 1 or -1
  if (g_conf.loss == LOG_LIKELIHOOD) {
    result->label = result->label > 0? 1 : -1;
  }

  for (size_t i = 2; i < tokens.size(); ++i) {
    size_t found = tokens[i].find(kKVDelimiter);
    if (found == std::string::npos) {
      std::cerr << "feature value pair with wrong format: " << tokens[i];
      continue;
    }
    size_t index = boost::lexical_cast<size_t>(tokens[i].substr(0, found));
    if (index >= n) {
      std::cerr << "feature index out of boundary: " << index;
      continue;
    }
    ValueType value = boost::lexical_cast<ValueType>(tokens[i].substr(found+1));
    result->feature[index] = value;
  }

  return result;
}

void CleanDataVector(DataVector *data) {
  DataVector::iterator iter = data->begin();
  for (; iter != data->end(); ++iter) {
    delete *iter;
  }
}

bool LoadDataFromFile(const std::string &path, DataVector *data, bool ignore_weight) {
  data->clear();
  std::ifstream stream(path.c_str());
  if (!stream) {
    return false;
  }

  long buffer_size = 512 * 1024 * 1024;
  char* local_buffer = new char[buffer_size];
  stream.rdbuf()->pubsetbuf(local_buffer, buffer_size);

  std::string l;
  while(std::getline(stream, l)) {
    Tuple *t = Tuple::FromString(l);
    if (ignore_weight) {
      t->weight = 1;
    }
    data->push_back(t);
  }

  delete[] local_buffer;

  return true;
}

bool LoadDataFromFile(const std::string &path, DataVector *data) {
  return LoadDataFromFile(path, data, false);
}

}

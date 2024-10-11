#include "metadata.hpp"
#include <fstream>
#include <iostream>

using namespace std;

Metadata read_metadata(const filesystem::path& metadata_path)
{
  Metadata results;
  ifstream stream(metadata_path);

  if (stream.is_open())
  {
    string line_symbol;
    string line_path;

    while (getline(stream, line_symbol) && getline(stream, line_path))
      results.emplace(line_symbol, line_path);
  }
  else
    cerr << "failed to open " << metadata_path << endl;
  return results;
}

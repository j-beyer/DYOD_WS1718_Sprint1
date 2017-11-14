#include "storage_manager.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "utils/assert.hpp"

namespace opossum {

StorageManager& StorageManager::get() {
  static StorageManager instance;
  return instance;
}

void StorageManager::add_table(const std::string& name, std::shared_ptr<Table> table) {
  Assert(!has_table(name), "Table '" + name + "' already exists!");
  _tables[name] = table;
}

void StorageManager::drop_table(const std::string& name) {
  if (!_tables.erase(name)) {
    throw std::runtime_error("Table does not exist!");
  }
}

std::shared_ptr<Table> StorageManager::get_table(const std::string& name) const { return _tables.at(name); }

bool StorageManager::has_table(const std::string& name) const { return _tables.count(name); }

std::vector<std::string> StorageManager::table_names() const {
  std::vector<std::string> names;
  names.reserve(_tables.size());
  for (const auto& entry : _tables) {
    names.push_back(entry.first);
  }
  return names;
}

void StorageManager::print(std::ostream& out) const {
  for (const auto& name : table_names()) {
    const auto table = get_table(name);
    out << "name: " << name;
    out << "\t#columns: " << table->col_count();
    out << "\t#rows: " << table->row_count();
    out << "\t#chunks: " << table->chunk_count();
    out << "\tchunk size: " << table->chunk_size() << "\n";
  }
  out << std::endl;
}

void StorageManager::reset() { get() = StorageManager{}; }

}  // namespace opossum

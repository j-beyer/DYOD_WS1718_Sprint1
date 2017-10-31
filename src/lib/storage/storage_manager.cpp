#include "storage_manager.hpp"

#include <algorithm>
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
  if (has_table(name)) {
    throw std::runtime_error("the table '" + name + "' already exists");
  }
  m_tables[name] = table;
}

void StorageManager::drop_table(const std::string& name) {
  if (!has_table(name)) {
    throw std::runtime_error("the table '" + name + "' does not exist");
  }
  m_tables.erase(m_tables.find(name));
}

std::shared_ptr<Table> StorageManager::get_table(const std::string& name) const {
  if (!has_table(name)) {
    throw std::runtime_error("the table '" + name + "' does not exist");
  }
  return m_tables.at(name);
}

bool StorageManager::has_table(const std::string& name) const {
  auto it = m_tables.find(name);
  return (it != m_tables.end());
}

std::vector<std::string> StorageManager::table_names() const {
  std::vector<std::string> names;
  names.reserve(m_tables.size());

  auto get_name = [](auto entry) { return entry.first; };
  std::transform(m_tables.begin(), m_tables.end(), std::back_inserter(names), get_name);

  // sort the names, because it makes it easier for the user to read
  // assuming that table_names() is only called to display the table names to the user,
  // and assuming that there is a normal amount of tables, this should not be too expensive
  std::sort(names.begin(), names.end());
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

void StorageManager::reset() {
  auto& instance = get();
  instance = StorageManager{};
}

}  // namespace opossum

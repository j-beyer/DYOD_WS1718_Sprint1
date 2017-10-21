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
  auto it = m_tables.find(name);
  if (it != m_tables.end()) {
    throw std::runtime_error("the table " + name + " does already exist");
  }
  m_tables.insert({name, table});
}

void StorageManager::drop_table(const std::string& name) {
  auto it = m_tables.find(name);
  if (it != m_tables.end()) {
    m_tables.erase(it);
  } else {
    throw std::runtime_error("the table" + name + " does not exist");
  }
}

std::shared_ptr<Table> StorageManager::get_table(const std::string& name) const {
  auto it = m_tables.find(name);
  if (it != m_tables.end()) {
    return (*it).second;
  } else {
    throw std::runtime_error("the table" + name + " does not exist");
  }
}

bool StorageManager::has_table(const std::string& name) const {
  auto it = m_tables.find(name);
  return (it != m_tables.end());
}

std::vector<std::string> StorageManager::table_names() const {
  std::vector<std::string> names;

  auto get_name = [&](auto entry) -> std::string { return entry.first; };
  std::transform(m_tables.begin(), m_tables.end(), names.begin(), get_name);
  return names;
}

void StorageManager::print(std::ostream& out) const {
  out << "Currently there are the following tables: \n";
  for (auto& name : table_names()) {
    out << name << "\n";
  }
  out << "------------------" << std::endl;
}

void StorageManager::reset() {
  auto& instance = get();
  instance.~StorageManager();
  new (&instance) StorageManager{};
}

}  // namespace opossum

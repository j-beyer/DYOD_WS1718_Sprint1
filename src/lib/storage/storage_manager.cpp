﻿#include "storage_manager.hpp"

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
  std::vector<std::string> names(m_tables.size());

  auto get_name = [](auto entry) { return entry.first; };
  std::transform(m_tables.begin(), m_tables.end(), names.begin(), get_name);
  std::sort(names.begin(), names.end());
  return names;
}

void StorageManager::print(std::ostream& out) const {
  out << "Tables: \n";
  for (auto& name : table_names()) {
    out << name << "\n";
  }
  out << std::endl;
}

void StorageManager::reset() {
  auto& instance = get();
  instance.~StorageManager();
  new (&instance) StorageManager{};
}

}  // namespace opossum

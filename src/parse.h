#pragma once
#include <memory>
#include "cell.h"

std::shared_ptr<Cell> parse(std::istream& ss, Cell::CellEnv& env);
bool evalHelper(std::istream& ss, Cell::CellEnv& env, bool verbose = true);
void loadFile(const std::string& file, Cell::CellEnv& env, bool verbose = true);

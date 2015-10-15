#pragma once
#include <memory>
#include "cell.h"

std::vector<std::shared_ptr<Cell> > parse(std::istream& is, Cell::CellEnv& env);
bool evalHelper(std::istream& ss, Cell::CellEnv& env, bool verbose = true);
void loadFile(const std::string& file, Cell::CellEnv& env, bool verbose = true);
std::shared_ptr<Cell> parseCell(std::istream& is, Cell::CellEnv& env);

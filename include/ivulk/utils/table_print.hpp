/**
 * @file table_print.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Utilities to print to the console as a nice table.
 */

#pragma once

#include <ivulk/config.hpp>

#include <algorithm>
#include <initializer_list>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace ivulk::utils {
	class TableRow : public std::vector<std::string>
	{
	public:
		TableRow()
			: std::vector<std::string>()
		{ }

		TableRow(std::size_t size)
			: std::vector<std::string>(size)
		{ }

		TableRow(std::initializer_list<std::string> il)
			: std::vector<std::string>(il)
		{ }

		TableRow(TableRow&& rhs)
			: std::vector<std::string>(rhs)
		{ }

		TableRow(const TableRow&) = default;
		TableRow& operator=(const TableRow&) = default;
	};

	class Table : public std::vector<TableRow>
	{
	public:
		Table()
			: std::vector<TableRow>()
		{ }

		Table(std::size_t size)
			: std::vector<TableRow>(size)
		{ }

		Table(std::initializer_list<TableRow> il)
			: std::vector<TableRow>(il)
		{ }

		Table(Table&& rhs)
			: std::vector<TableRow>(rhs)
		{ }

		Table(const Table&) = delete;
		TableRow& operator=(const Table&) = delete;

		Table& enableHeadingRow()
		{
			bHeadingRow = true;
			return *this;
		}

		Table& leadingSpaces(std::size_t spaces)
		{
			m_leadingSpaces = spaces;
			return *this;
		}

		std::size_t cols() const
		{
			auto compareFn = [](TableRow a, TableRow b) { return a.size() < b.size(); };
			auto maxRow = std::max_element(begin(), end(), compareFn);
			return (*maxRow).size();
		}
		std::size_t rows() const { return size(); }
		std::size_t colWidth(std::size_t colIdx) const
		{
			auto compareFn = [colIdx](TableRow a, TableRow b) {
				auto s1 = colIdx >= a.size() ? 0 : a[colIdx].size();
				auto s2 = colIdx >= b.size() ? 0 : b[colIdx].size();
				return s1 < s2;
			};
			auto maxRow = *std::max_element(begin(), end(), compareFn);
			return colIdx >= maxRow.size() ? 0 : maxRow[colIdx].size();
		}
		std::string toString() const
		{
			std::stringstream ss;
			const std::string lspace(m_leadingSpaces, ' ');

			const auto nCols = cols();
			std::vector<std::size_t> columnWidths(nCols);
			std::generate(columnWidths.begin(), columnWidths.end(),
						  [this, idx = 0]() mutable { return colWidth(idx++); });

			auto outputRowSep = [&](char sepChar = '-') {
				ss << lspace;
				ss << "+";
				for (std::size_t ci = 0; ci < nCols; ++ci)
				{
					ss << std::setfill(sepChar);
					ss << std::setw(columnWidths[ci] + 2) << sepChar;
					ss << std::setfill(' ');
					ss << "+";
				}
				ss << std::endl;
			};

			size_t ri = 0;
			for (auto& row : *this)
			{
				outputRowSep((ri == 1 || ri == 0) && bHeadingRow ? '=' : '-');
				ss << lspace << "| ";
				for (std::size_t ci = 0; ci < nCols; ++ci)
				{
					auto colTxt = (ci < row.size()) ? row[ci] : "";
					ss << std::setw(columnWidths[ci]) << colTxt << " | ";
				}
				ss << std::endl;
				ri++;
			}
			outputRowSep();

			return ss.str();
		}

	private:
		bool bHeadingRow = false;
		std::size_t m_leadingSpaces = 0;
	};

	std::ostream& operator<<(std::ostream& lhs, const Table& rhs)
	{
		lhs << rhs.toString();
		return lhs;
	}
} // namespace ivulk::utils

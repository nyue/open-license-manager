/*
 * pc_identifier.cpp
 *
 *  Created on: Dec 22, 2019
 *      Author: GC
 */

#include <algorithm>
#include "pc_identifier.hpp"
#include "../base/base64.h"

namespace license {
namespace pc_identifier {

using namespace std;

PcIdentifier::PcIdentifier() {}

PcIdentifier::PcIdentifier(const std::string& param) {
	string tmp_str(param);  // throw away const
	std::replace(tmp_str.begin(), tmp_str.end(), '-', '\n');
	vector<uint8_t> decoded = unbase64(tmp_str);
	if (decoded.size() != PC_IDENTIFIER_PROPRIETARY_DATA + 1) {
		cerr << decoded.size();
		throw logic_error("wrong identifier size " + param);
	}
	std::copy_n(decoded.begin(), PC_IDENTIFIER_PROPRIETARY_DATA + 1, m_data.begin());
}

PcIdentifier::~PcIdentifier() {}

PcIdentifier::PcIdentifier(const PcIdentifier& other) : m_data(other.m_data) {}

void PcIdentifier::set_identification_strategy(LCC_API_IDENTIFICATION_STRATEGY strategy) {
	if (strategy == STRATEGY_NONE || strategy == STRATEGY_DEFAULT) {
		throw logic_error("Only known strategies are permitted");
	}
	uint8_t stratMov = (strategy << 5);
	m_data[1] = (m_data[1] & 0x1F) | stratMov;
}

void PcIdentifier::set_use_environment_var(bool use_env_var) {
	if (use_env_var) {
		m_data[0] = m_data[0] | 0x40;
	} else {
		m_data[0] = m_data[0] & ~0x40;
	}
}

void PcIdentifier::set_virtual_environment(VIRTUALIZATION virt) {
	// 110000 0x30
	m_data[0] = (m_data[0] && ~0x30) | virt << 4;
}

void PcIdentifier::set_virtualization(VIRTUALIZATION_DETAIL virtualization_detail) {  // TODO
}

void PcIdentifier::set_cloud_provider(CLOUD_PROVIDER cloud_provider) {
	// TODO
}

void PcIdentifier::set_data(const std::array<uint8_t, PC_IDENTIFIER_PROPRIETARY_DATA>& data) {
	m_data[1] = (m_data[1] & (~0x1f)) | (data[0] & 0x1f);
	for (int i = 1; i < PC_IDENTIFIER_PROPRIETARY_DATA; i++) {
		m_data[i + 1] = data[i];
	}
}

std::string PcIdentifier::print() const {
	string result = base64(m_data.data(), m_data.size(), 5);
	std::replace(result.begin(), result.end(), '\n', '-');
	return result.substr(0, result.size() - 1);
}

LCC_API_IDENTIFICATION_STRATEGY PcIdentifier::get_identification_strategy() const {
	uint8_t stratMov = m_data[1] >> 5;
	return static_cast<LCC_API_IDENTIFICATION_STRATEGY>(stratMov);
}

bool PcIdentifier::data_match(const std::array<uint8_t, PC_IDENTIFIER_PROPRIETARY_DATA>& data) const {
	bool equals = true;
	for (int i = 0; i < PC_IDENTIFIER_PROPRIETARY_DATA && equals; i++) {
		equals = (i == 0) ? ((data[i] & 0x1f) == (m_data[i + 1] & 0x1f)) : (data[i] == m_data[i + 1]);
	}
	return equals;
}

}  // namespace pc_identifier
} /* namespace license */

#ifndef BINARY_WRITER_HPP
#define BINARY_WRITER_HPP

#include <fstream>
#include <string>
#include <vector>
#include <map>

class binary_writer_t
{
public:
	binary_writer_t(uintmax_t len) {
		this->m_mempos = 0;
		this->m_len = len;

		m_data = new uint8_t[len];
		memset(m_data, 0, len);
	}
	~binary_writer_t() {
		delete[] m_data;
	}

	void write(bool val) {
		memcpy(m_data + m_mempos, &val, sizeof(bool));
		m_mempos += sizeof(bool);
	}
	void write(uint8_t val) {
		memcpy(m_data + m_mempos, &val, sizeof(uint8_t));
		m_mempos += sizeof(uint8_t);
	}
	void write(uint16_t val) {
		memcpy(m_data + m_mempos, &val, sizeof(uint16_t));
		m_mempos += sizeof(uint16_t);
	}
	void write(uint32_t val) {
		memcpy(m_data + m_mempos, &val, sizeof(uint32_t));
		m_mempos += sizeof(uint32_t);
	}
	void write(int val) {
		memcpy(m_data + m_mempos, &val, sizeof(int));
		m_mempos += sizeof(int);
	}
	void write(float val) {
		memcpy(m_data + m_mempos, &val, sizeof(float));
		m_mempos += sizeof(float);
	}
	void write(long long val) {
		memcpy(m_data + m_mempos, &val, sizeof(long long));
		m_mempos += sizeof(long long);
	}
	void write(uint64_t val) {
		memcpy(m_data + m_mempos, &val, sizeof(uint64_t));
		m_mempos += sizeof(uint64_t);
	}
	void write(uint8_t* val, const uintmax_t& len) {
		memcpy(m_data + m_mempos, val, len);
		m_mempos += (uint32_t)len;
	}
	void write(const std::string& val) {
		uint16_t len = (uint16_t)val.length();

		memcpy(m_data + m_mempos, &len, sizeof(uint16_t));
		memcpy(m_data + m_mempos + sizeof(uint16_t), val.c_str(), val.length());
		m_mempos += len + sizeof(uint16_t);
	}

	const uint8_t* get() {
		return m_data;
	}
	const uint32_t get_pos() {
		return m_mempos;
	}
	const uintmax_t get_size() {
		return m_len;
	}
private:
	uint8_t* m_data;
	uint32_t m_mempos;
	uintmax_t m_len;
};
#endif // !BINARY_WRITER_HPP

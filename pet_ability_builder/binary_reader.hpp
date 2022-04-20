#ifndef BINARY_READER_HPP
#define BINARY_READER_HPP

#include <string>

class binary_reader_t
{
public:
	binary_reader_t(uint8_t* data)
		: m_data(data), m_pos(0) {}
	~binary_reader_t() {
		free(this->m_data);
	}

	uint8_t read_ubyte() {
		uint8_t val;
		memcpy(&val, this->m_data + this->m_pos, sizeof(uint8_t));
		this->m_pos += sizeof(uint8_t);
		return val;
	}
	bool read_bool() {
		bool val;
		memcpy(&val, this->m_data + this->m_pos, sizeof(bool));
		this->m_pos += sizeof(bool);
		return val;
	}
	short read_short() {
		short val;
		memcpy(&val, this->m_data + this->m_pos, sizeof(short));
		this->m_pos += sizeof(short);
		return val;
	}
	uint16_t read_ushort() {
		uint16_t val;
		memcpy(&val, this->m_data + this->m_pos, sizeof(uint16_t));
		this->m_pos += sizeof(uint16_t);
		return val;
	}
	int read_int() {
		int val;
		memcpy(&val, this->m_data + this->m_pos, sizeof(int));
		this->m_pos += sizeof(int);
		return val;
	}
	uint32_t read_uint() {
		uint32_t val;
		memcpy(&val, this->m_data + this->m_pos, sizeof(uint32_t));
		this->m_pos += sizeof(uint32_t);
		return val;
	}
	float read_float() {
		float val;
		memcpy(&val, this->m_data + this->m_pos, sizeof(float));
		this->m_pos += sizeof(float);
		return val;
	}
	uint64_t read_ulong() {
		uint64_t val;
		memcpy(&val, this->m_data + this->m_pos, sizeof(uint64_t));
		this->m_pos += sizeof(uint64_t);
		return val;
	}
	std::string read_string() {
		std::string val;
		uint16_t str_len = *(int16_t*)&this->m_data[this->m_pos];

		val = std::string(reinterpret_cast<char*>(this->m_data + this->m_pos + 2), str_len);
		this->m_pos += sizeof(uint16_t) + str_len;
		return val;
	}
	std::string read_item_name(uint32_t id) {
		std::string secret = "PBG892FXX982ABC*";
		uint16_t str_len = *(int16_t*)&this->m_data[this->m_pos];

		std::string input = std::string(reinterpret_cast<char*>(this->m_data + this->m_pos + 2), str_len);
		std::string val(input.size(), 0);

		for (uint32_t i = 0; i < input.size(); ++i)
			val[i] = input[i] ^ secret[(i + id) % secret.size()];
		this->m_pos += sizeof(uint16_t) + str_len;
		return val;
	}

	void skip(uint32_t len) {
		this->m_pos += len;
	}
private:
	uint8_t* m_data;
	uint32_t m_pos;
};

#endif // !BINARY_READER_HPP

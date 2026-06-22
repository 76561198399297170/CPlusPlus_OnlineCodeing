#include "utils/security.h"

bool skipBOM(std::ifstream& file)
{
    char bom[3];
    std::streamsize bytes_read = file.readsome(bom, 3);

    if (bytes_read == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)
    {
        return true;
    }
    else if (bytes_read > 0)
    {
        file.seekg(0);
    }
    return false;
}

namespace sha256
{
    const uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    std::string generateSalt(int salt_length, int subhash)
    {
        const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

        std::string salt;
        salt.reserve(salt_length);
        for (int i = 0; i < salt_length; i++)
        {
            int ran = util::generateRandomInt(0, sizeof(charset) + subhash);
            salt += charset[ran];
        }

        return salt;
    }

    uint32_t rotateRight(uint32_t x, uint32_t n)
    {
        return (x >> n) | (x << (32 - n));
    }

    void sha256_transform(uint32_t state[8], const uint8_t block[64])
    {
        uint32_t w[64];
        uint32_t a, b, c, d, e, f, g, h;

        for (int i = 0; i < 16; i++)//初始化消息调度数组w
        {
            w[i] = (uint32_t)block[i * 4] << 24 | (uint32_t)block[i * 4 + 1] << 16 |
                (uint32_t)block[i * 4 + 2] << 8 | (uint32_t)block[i * 4 + 3];
        }
        for (int i = 16; i < 64; i++)
        {
            uint32_t s0 = rotateRight(w[i - 15], 7) ^ rotateRight(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = rotateRight(w[i - 2], 17) ^ rotateRight(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        //初始化工作变量
        a = state[0];
        b = state[1];
        c = state[2];
        d = state[3];
        e = state[4];
        f = state[5];
        g = state[6];
        h = state[7];

        //状态加密
        for (int i = 0; i < 64; i++)
        {
            uint32_t S1 = rotateRight(e, 6) ^ rotateRight(e, 11) ^ rotateRight(e, 25);
            uint32_t ch = (e & f) ^ (~e & g);
            uint32_t temp1 = h + S1 + ch + k[i] + w[i];
            uint32_t S0 = rotateRight(a, 2) ^ rotateRight(a, 13) ^ rotateRight(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        //状态更新
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        state[5] += f;
        state[6] += g;
        state[7] += h;
    }

    std::vector<uint8_t> sha256(const std::string& input)
    {
        uint32_t state[8] = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        };

        //补位添加1再补0，最后添加原始长度
        std::vector<uint8_t> data(input.begin(), input.end());
        uint64_t bit_len = data.size() * 8;

        data.push_back(0x80);//补1
        while (data.size() % 64 != 56)
            data.push_back(0x00);//补0

        for (int i = 7; i >= 0; i--)
            data.push_back((bit_len >> (i * 8)) & 0xff);

        //分块处理
        for (size_t i = 0; i < data.size(); i += 64)
            sha256_transform(state, &data[i]);

        //8个32位状态转换为256位字节数组
        std::vector<uint8_t> hash(32);
        for (int i = 0; i < 8; i++)
        {
            hash[i * 4] = (state[i] >> 24) & 0xff;
            hash[i * 4 + 1] = (state[i] >> 16) & 0xff;
            hash[i * 4 + 2] = (state[i] >> 8) & 0xff;
            hash[i * 4 + 3] = state[i] & 0xff;
        }

        return hash;
    }

    std::string hashPasswordBysha256(const std::string& password, const std::string& salt)
    {
        std::string input = password + salt;
        std::vector<uint8_t> hash_bytes = sha256(input);

        //十六进制转换转换哈希字节数组
        std::ostringstream oss;
        for (uint8_t b : hash_bytes)
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;

        return oss.str();
    }
}

namespace hs256
{
    const char kBase64URLCharset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ""abcdefghijklmnopqrstuvwxyz""0123456789-_";
    const char kHS256Alg[] = "HS256";
    const char kJwtType[] = "JWT";

    std::string encodeBase64URL(const std::string& input)
    {
        std::string ret;
        int i = 0, j = 0;
        unsigned char char3[3], char4[4];
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(input.c_str());
        size_t len = input.length();

        while (len--)
        {
            char3[i++] = *(bytes++);
            if (i == 3)
            {
                char4[0] = (char3[0] & 0xfc) >> 2;
                char4[1] = ((char3[0] & 0x03) << 4) + ((char3[1] & 0xf0) >> 4);
                char4[2] = ((char3[1] & 0x0f) << 2) + ((char3[2] & 0xc0) >> 6);
                char4[3] = char3[2] & 0x3f;

                for (i = 0; i < 4; i++) ret += kBase64URLCharset[char4[i]];
                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 3; j++) char3[j] = '\0';
            char4[0] = (char3[0] & 0xfc) >> 2;
            char4[1] = ((char3[0] & 0x03) << 4) + ((char3[1] & 0xf0) >> 4);
            char4[2] = ((char3[1] & 0x0f) << 2) + ((char3[2] & 0xc0) >> 6);

            for (j = 0; j < i + 1; j++) ret += kBase64URLCharset[char4[j]];
        }

        ret.erase(ret.find_last_not_of('=') + 1);
        return ret;
    }

    std::string decodeBase64URL(const std::string& input)
    {
        std::string ret;
        int i = 0, j = 0, in_len = input.length();
        unsigned char char4[4], char3[3];
        std::string temp = input;

        int pad = temp.length() % 4;
        if (pad) temp.append(4 - pad, '=');

        while (in_len--)
        {
            char4[i++] = temp[j++];
            if (i == 4)
            {
                for (i = 0; i < 4; i++)
                {
                    char4[i] = strchr(kBase64URLCharset, char4[i]) - kBase64URLCharset;
                }

                char3[0] = (char4[0] << 2) + ((char4[1] & 0x30) >> 4);
                char3[1] = ((char4[1] & 0xf) << 4) + ((char4[2] & 0x3c) >> 2);
                char3[2] = ((char4[2] & 0x3) << 6) + char4[3];

                for (i = 0; i < 3; i++) ret += char3[i];
                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 4; j++) char4[j] = 0;
            for (j = 0; j < 4; j++) char4[j] = strchr(kBase64URLCharset, char4[j]) - kBase64URLCharset;

            char3[0] = (char4[0] << 2) + ((char4[1] & 0x30) >> 4);
            char3[1] = ((char4[1] & 0xf) << 4) + ((char4[2] & 0x3c) >> 2);

            for (j = 0; j < i - 1; j++) ret += char3[j];
        }

        return ret;
    }

    std::vector<uint8_t> hmac_sha256(const std::string& data, const std::string& secret)
    {
        const size_t block_size = 64;//SHA256块大小
        std::vector<uint8_t> key(block_size, 0);
        std::vector<uint8_t> key_bytes(secret.begin(), secret.end());

        //密钥处理
        if (key_bytes.size() > block_size)
        {
            //密钥过长
            key_bytes = sha256::sha256(secret);
        }
        //复制密钥到64字节缓冲区
        memcpy(key.data(), key_bytes.data(), key_bytes.size());

        //生成ipad和opad
        std::vector<uint8_t> ipad(block_size, 0x36);
        std::vector<uint8_t> opad(block_size, 0x5c);
        for (size_t i = 0; i < block_size; i++)
        {
            ipad[i] ^= key[i];
            opad[i] ^= key[i];
        }

        //计算 ipad + 数据 的SHA256
        std::vector<uint8_t> input1(ipad.begin(), ipad.end());
        input1.insert(input1.end(), data.begin(), data.end());
        std::string input1_str(input1.begin(), input1.end());
        std::vector<uint8_t> hash1 = sha256::sha256(input1_str);

        //计算 opad + hash1 的 SHA256 = 最终HMAC结果
        std::vector<uint8_t> input2(opad.begin(), opad.end());
        input2.insert(input2.end(), hash1.begin(), hash1.end());
        std::string input2_str(input2.begin(), input2.end());

        return sha256::sha256(input2_str);
    }

    std::string generateToken(const std::string& secret, int64_t user_id, const std::string& role, int expire_seconds)
    {
        //构建JWT Header
        nlohmann::json header = { {"alg", kHS256Alg}, {"typ", kJwtType} };
        std::string header_b64 = encodeBase64URL(header.dump(-1, ' ', false));

        //构建JWT Payload
        time_t now = time(nullptr);
        nlohmann::json payload = { {"user_id", user_id}, {"role", role}, {"exp", now + expire_seconds} };
        std::string payload_b64 = encodeBase64URL(payload.dump(-1, ' ', false));

        //拼接header.payload并生成HMAC-SHA256签名
        std::string data = header_b64 + "." + payload_b64;
        std::vector<uint8_t> hmac_bytes = hmac_sha256(data, secret);

        //直接对二进制数据进行Base64URL编码
        std::string signature_b64 = encodeBase64URL(std::string(reinterpret_cast<const char*>(hmac_bytes.data()), hmac_bytes.size()));

        //组合完整Token
        return data + "." + signature_b64;
    }

    hs256::ReturnType verifyToken(const std::string& token, const std::string& secret, int64_t& out_user_id, std::string& out_role, int* time_left)
    {
        //拆分Token
        size_t dot1 = token.find('.');
        size_t dot2 = token.find('.', dot1 + 1);
        if (dot1 == std::string::npos || dot2 == std::string::npos)
        {
            return hs256::ReturnType::TYPEERROR;
        }
        std::string header_b64 = token.substr(0, dot1);
        std::string payload_b64 = token.substr(dot1 + 1, dot2 - dot1 - 1);
        std::string signature_b64 = token.substr(dot2 + 1);

        //重新生成签名
        std::string data = header_b64 + "." + payload_b64;
        std::vector<uint8_t> new_hmac_bytes = hmac_sha256(data, secret);
        std::string new_signature_b64 = encodeBase64URL(std::string(reinterpret_cast<const char*>(new_hmac_bytes.data()), new_hmac_bytes.size()));

        if (new_signature_b64 != signature_b64)
        {
            return hs256::ReturnType::VERIFYERROR;
        }

        //解析Payload并检查过期
        std::string payload_str = decodeBase64URL(payload_b64);
        nlohmann::json payload = nlohmann::json::parse(payload_str);
        time_t now = time(nullptr);
        double exp_double = payload["exp"].get<double>();
        if (now > static_cast<time_t>(exp_double))
        {
            return hs256::ReturnType::OVERDUEERROR;
        }
        if (time_left != nullptr)
        {
            *(time_left) = exp_double - now;
        }

        //提取用户信息
        out_user_id = payload["user_id"].get<int64_t>();
        out_role = payload["role"].get<std::string>();
        return hs256::ReturnType::SUCCESS;
    }
}

namespace util
{
    enum class InputType
    {
        ACCOUNT,        //登录输入账号
        PHONE,          //电话
        EMAIL,          //邮箱
        PASSWORD,       //密码
        NIKONAME,       //昵称
        USERDESC,       //个性签名
        SCHOOL,         //学校
        GRADE,          //年级
        STUDENTNO,      //学号

    };

    template<typename T>
    bool validateInput(T value, InputType type)
    {
        return true;
    }

    int generateRandomInt(int min, int max)
    {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::uniform_int_distribution<> dist(min, max);

        return dist(gen);
    }

    void splitQuestionContent(const std::string content, std::string& question_desc, std::string& input_format, std::string& output_format, std::string& sample_input)
    {
        const unsigned char SECTION_QUESTION_BYTES[] = {0x23, 0x23, 0x23, 0x20, 0xe9, 0x97, 0xae, 0xe9, 0xa2, 0x98, 0xe6, 0x8f, 0x8f, 0xe8, 0xbf, 0xb0};
        const unsigned char SECTION_INPUT_BYTES[] = {0x23, 0x23, 0x23, 0x20, 0xe8, 0xbe, 0x93, 0xe5, 0x85, 0xa5, 0xe6, 0xa0, 0xbc, 0xe5, 0xbc, 0x8f};
        const unsigned char SECTION_OUTPUT_BYTES[] = {0x23, 0x23, 0x23, 0x20, 0xe8, 0xbe, 0x93, 0xe5, 0x87, 0xba, 0xe6, 0xa0, 0xbc, 0xe5, 0xbc, 0x8f};
        const unsigned char SECTION_SAMPLE_BYTES[] = {0x23, 0x23, 0x23, 0x20, 0xe6, 0xa0, 0xb7, 0xe4, 0xbe, 0x8b};
        
        const std::string SECTION_QUESTION(reinterpret_cast<const char*>(SECTION_QUESTION_BYTES), sizeof(SECTION_QUESTION_BYTES));
        const std::string SECTION_INPUT(reinterpret_cast<const char*>(SECTION_INPUT_BYTES), sizeof(SECTION_INPUT_BYTES));
        const std::string SECTION_OUTPUT(reinterpret_cast<const char*>(SECTION_OUTPUT_BYTES), sizeof(SECTION_OUTPUT_BYTES));
        const std::string SECTION_SAMPLE(reinterpret_cast<const char*>(SECTION_SAMPLE_BYTES), sizeof(SECTION_SAMPLE_BYTES));

        size_t pos_question = content.find(SECTION_QUESTION);
        size_t pos_input = content.find(SECTION_INPUT);
        size_t pos_output = content.find(SECTION_OUTPUT);
        size_t pos_sample = content.find(SECTION_SAMPLE);

        if (pos_question != std::string::npos)
        {
            size_t start = pos_question + SECTION_QUESTION.length();
            size_t end = (pos_input != std::string::npos) ? pos_input : content.length();
            question_desc = content.substr(start, end - start);

            size_t first = question_desc.find_first_not_of(" \t\n\r");
            size_t last = question_desc.find_last_not_of(" \t\n\r");
            if (first != std::string::npos && last != std::string::npos)
            {
                question_desc = question_desc.substr(first, last - first + 1);
            }

            size_t pos = 0;
            while ((pos = question_desc.find("\r\n", pos)) != std::string::npos)
            {
                question_desc.replace(pos, 2, "\n");
                pos += 1;
            }
            pos = 0;
            while ((pos = question_desc.find("\r", pos)) != std::string::npos)
            {
                question_desc.replace(pos, 1, "\n");
                pos += 1;
            }
        }

        if (pos_input != std::string::npos)
        {
            size_t start = pos_input + SECTION_INPUT.length();
            size_t end = (pos_output != std::string::npos) ? pos_output : content.length();
            input_format = content.substr(start, end - start);

            size_t first = input_format.find_first_not_of(" \t\n\r");
            size_t last = input_format.find_last_not_of(" \t\n\r");
            if (first != std::string::npos && last != std::string::npos)
            {
                input_format = input_format.substr(first, last - first + 1);
            }

            size_t pos = 0;
            while ((pos = input_format.find("\r\n", pos)) != std::string::npos)
            {
                input_format.replace(pos, 2, "\n");
                pos += 1;
            }
            pos = 0;
            while ((pos = input_format.find("\r", pos)) != std::string::npos)
            {
                input_format.replace(pos, 1, "\n");
                pos += 1;
            }
        }

        if (pos_output != std::string::npos)
        {
            size_t start = pos_output + SECTION_OUTPUT.length();
            size_t end = (pos_sample != std::string::npos) ? pos_sample : content.length();
            output_format = content.substr(start, end - start);

            size_t first = output_format.find_first_not_of(" \t\n\r");
            size_t last = output_format.find_last_not_of(" \t\n\r");
            if (first != std::string::npos && last != std::string::npos)
            {
                output_format = output_format.substr(first, last - first + 1);
            }

            size_t pos = 0;
            while ((pos = output_format.find("\r\n", pos)) != std::string::npos)
            {
                output_format.replace(pos, 2, "\n");
                pos += 1;
            }
            pos = 0;
            while ((pos = output_format.find("\r", pos)) != std::string::npos)
            {
                output_format.replace(pos, 1, "\n");
                pos += 1;
            }
        }

        if (pos_sample != std::string::npos)
        {
            size_t start = pos_sample + SECTION_SAMPLE.length();
            size_t end = content.length();
            sample_input = content.substr(start, end - start);

            size_t first = sample_input.find_first_not_of(" \t\n\r");
            size_t last = sample_input.find_last_not_of(" \t\n\r");
            if (first != std::string::npos && last != std::string::npos)
            {
                sample_input = sample_input.substr(first, last - first + 1);
            }

            size_t pos = 0;
            while ((pos = sample_input.find("\r\n", pos)) != std::string::npos)
            {
                sample_input.replace(pos, 2, "\n");
                pos += 1;
            }
            pos = 0;
            while ((pos = sample_input.find("\r", pos)) != std::string::npos)
            {
                sample_input.replace(pos, 1, "\n");
                pos += 1;
            }
        }
    }

    std::string gbk_to_utf8(const std::string& gbk_str)
    {
        int wide_len = MultiByteToWideChar(936, 0, gbk_str.c_str(), -1, nullptr, 0);
        wchar_t* wide_buf = new wchar_t[wide_len];
        MultiByteToWideChar(936, 0, gbk_str.c_str(), -1, wide_buf, wide_len);

        int utf8_len = WideCharToMultiByte(65001, 0, wide_buf, -1, nullptr, 0, nullptr, nullptr);
        char* utf8_buf = new char[utf8_len];
        WideCharToMultiByte(65001, 0, wide_buf, -1, utf8_buf, utf8_len, nullptr, nullptr);

        std::string utf8_str(utf8_buf);
        delete[] wide_buf;
        delete[] utf8_buf;
        return utf8_str;
    }

    bool isNowAfter(const std::string& s)
    {
        tm t{};
        std::istringstream(s) >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");
        return std::chrono::system_clock::now() > std::chrono::system_clock::from_time_t(std::mktime(&t));
    }

    //std::string utf8_to_gbk(const std::string& utf8_str)
    //{
    //    int wide_len = MultiByteToWideChar(65001, 0, utf8_str.c_str(), -1, nullptr, 0);
    //    wchar_t* wide_buf = new wchar_t[wide_len];
    //    MultiByteToWideChar(65001, 0, utf8_str.c_str(), -1, wide_buf, wide_len);
    //
    //    int gbk_len = WideCharToMultiByte(936, 0, wide_buf, -1, nullptr, 0, nullptr, nullptr);
    //    char* gbk_buf = new char[gbk_len];
    //    WideCharToMultiByte(936, 0, wide_buf, -1, gbk_buf, gbk_len, nullptr, nullptr);
    //
    //    std::string gbk_str(gbk_buf);
    //    delete[] wide_buf;
    //    delete[] gbk_buf;
    //    return gbk_str;
    //}
}
